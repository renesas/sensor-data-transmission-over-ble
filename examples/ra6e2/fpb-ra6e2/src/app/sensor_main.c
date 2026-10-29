#include "icm_thread.h"
#include "rm_icm42670_hal_data.h"
#include "sensor_ble_bridge.h"
#include "rs_ble_sensor_data_tx.h"
#include "semphr.h"   /* SemaphoreHandle_t / xSemaphoreCreateBinary / xSemaphoreGiveFromISR for s_icm_drdy_sem -
                       * included directly rather than relying on it arriving via sensor_ble_bridge.h. */

#include "event_groups.h"
/* Created by app_main() (app/ble/app_main.c) before its main loop; the GTL driver externs it the same
 * way (r_ble_gtl.c:1350). Checked for NULL at every use because the ICM thread can start running before
 * the BLE thread has created it. */
extern EventGroupHandle_t g_ble_event_group_handle;

/* Created in sensor_main() BEFORE RM_ICM42670_Open() - Open enables the IRQ as part of opening the
 * device, so the semaphore must already exist or the first interrupt (which can arrive before Open even
 * returns) dereferences NULL in app_icm_drdy_isr_signal(). Declared up here (rather than next to
 * app_icm_drdy_isr_signal() below) so rs_sensor_write_request_cb() can also give it - see there. */
static SemaphoreHandle_t s_icm_drdy_sem = NULL;

/* Counts data-ready signals that could not be given because the previous one was still un-taken (ICM thread
 * busy/late). Expected under overrun and not a fault; watch it in the debugger. Not printed: RTT printf is
 * not ISR-safe. */
static volatile uint32_t s_icm_drdy_give_fail_cnt = 0;

/* Data-ready wait timeout for the current ODR, so xSemaphoreTake() in the loop doesn't recompute it every
 * pass.*/
static TickType_t s_icm_drdy_timeout_ticks = ICM_DRDY_TIMEOUT_MAX_MS * (configTICK_RATE_HZ / 1000);

static TickType_t icm_drdy_timeout_ticks(uint16_t odr_hz);

/* Written by the BLE thread, consumed by the ICM thread. A single uint16 plus a flag, both volatile: the
 * write is atomic on this core and a lost/overwritten request just means the newest one wins, which is the
 * behaviour we want anyway. No mutex needed, and importantly none is taken on the BLE thread's path. */
static volatile uint16_t s_odr_request_hz = 0;
static volatile uint8_t  s_odr_request_pending = 0;

/* RS CALLBACK - declared in rs_ble_sensor_data_tx.h, NOT defined there. Any project using this RS module
 * must implement it on the APP side; leaving it out is a link error, not a silent no-op. Reached from the
 * DA14531 BLE write path - app_main.c's SensorRates_cb() on a GATT write to the Sensor ODR characteristic -
 * so it runs on the BLE THREAD. Record the request only; here the ICM thread owns the I2C bus and applies it
 * later in icm_odr_apply_pending(). odr_hz arrives unvalidated off the wire, hence the range check. */
void rs_sensor_write_request_cb (uint16_t odr_hz)
{
    if ((odr_hz < ICM_ODR_MIN_HZ) || (odr_hz > ICM_ODR_MAX_HZ)) //add printf to indicate invalid ODR request
    {
        /* Out of range for LN mode - ignore rather than let rm_icm42670_accel_configuration() fail and
         * leave the sensor half-configured. */
        MSG_PRINT("Invalid ODR request: %d Hz\n", odr_hz);
        return;
    }

    s_odr_request_hz      = odr_hz;
    s_odr_request_pending = 1;

    /* Without this, the ICM thread only notices the pending request after its current xSemaphoreTake()
     * returns - which can be up to ICM_DRDY_TIMEOUT_MAX_MS away, since icm_odr_apply_pending() is only checked
     * at the top of the loop, after the wait. Giving the semaphore here wakes it immediately instead.
     * This runs on the BLE thread (task context, not an ISR), so a plain give is correct - not the
     * FromISR variant app_icm_drdy_isr_signal() uses below. NULL-checked because the BLE thread can call
     * this before the ICM thread has created s_icm_drdy_sem. Giving a binary semaphore that a real INT1
     * edge already gave (still un-taken) is a harmless no-op - it does not double-wake the thread. */
    if (NULL != s_icm_drdy_sem)
    {
        if (pdFALSE == xSemaphoreGive(s_icm_drdy_sem))
        {
            MSG_PRINT("Failed to give ICM semaphore\n");
        }
    }
}

/* Applies a pending ODR request. MUST be called from the ICM thread only (owns the I2C bus).
 * The ODR is only written to the sensor while it is being configured (inside RM_ICM42670_Open()), so it
 * cannot be changed on a running sensor. The ICM therefore has to be CLOSED AND REOPENED after every ODR change:
 *   1. RM_ICM42670_Close()                - stops the sensor driver, closes its comms and data-ready IRQ
 *   2. RM_ICM42670_Open()                 - resets the sensor and configures it with the new ODR
 *   3. RM_ICM42670_DeviceInterruptCfgSet()- re-enables the INT1 data-ready interrupt
 *   4. RM_ICM42670_MeasurementStart()     - starts measuring again at the new ODR
 * Steps 2-4 are the same sequence icm_hw_init() performs at startup. No samples are produced between steps 1 and 4. */
static void icm_odr_apply_pending (void)
{
    if (0 == s_odr_request_pending)
    {
        return;
    }
    s_odr_request_pending = 0;

    uint16_t odr_hz = s_odr_request_hz;

    /* g_icm42670_sensor0_extended_cfg is deliberately non-const; rm_icm42670_accel_configuration() reads
     * .accel_odr through p_ctrl->p_mode at configuration time, i.e. the new value only takes effect on the
     * next RM_ICM42670_Open() below. */
    g_icm42670_sensor0_extended_cfg.accel_odr = odr_hz;
    s_icm_drdy_timeout_ticks = icm_drdy_timeout_ticks(odr_hz);

    fsp_err_t err;

    /* Close first, then reopen: the ICM must go through a full Close -> Open cycle for the new ODR to apply. */
    (void) RM_ICM42670_Close(&g_icm42670_sensor0_ctrl);
    err = RM_ICM42670_Open(&g_icm42670_sensor0_ctrl, &g_icm42670_sensor0_cfg);
    if (FSP_SUCCESS == err)
    {
        err = RM_ICM42670_DeviceInterruptCfgSet(&g_icm42670_sensor0_ctrl, g_icm42670_interrupt_cfg);
    }
    if (FSP_SUCCESS == err)
    {
        err = RM_ICM42670_MeasurementStart(&g_icm42670_sensor0_ctrl);
    }
}

/*----------------------------------------------------------------------------------------------------------------------
    project_logic() - ALL application policy for the ICM -> GATT path, in one place, running on the ICM
    thread itself. Called from data_cb() below,
    before rs_ble_sensor_tx() - so a sample that fails this gate never crosses over to the BLE thread at all.

    Given the newest sample, decides whether it is worth notifying. Applies, in order:
      1. rate limit               (BLE_ICM_UPDATE_INTERVAL_MS)
      2. minimum change threshold (BLE_ICM_SENSOR_SENSITIVITY_THRESHOLD_MG)

    Returns true if the caller should notify. p_sample is updated in place when smoothing is enabled, so the
    caller sends the filtered value.

    BLE_ICM_UPDATE_INTERVAL_MS / BLE_ICM_SENSOR_SENSITIVITY_THRESHOLD_MG - rate limit and minimum-change threshold applied
    in project_logic() before an ICM sample is handed to the BLE thread for a GATT notify.
    BLE_ICM_SENSOR_SENSITIVITY_THRESHOLD_MG defaults to 15. This value can be adjusted based on the desired sensitivity to accelerometer changes.
    15 is high sensitivity (more notifications). Raise it (e.g. 500) for low sensitivity (fewer notifications).
----------------------------------------------------------------------------------------------------------------------*/
#define BLE_ICM_UPDATE_INTERVAL_MS (100)
#define BLE_ICM_SENSOR_SENSITIVITY_THRESHOLD_MG (15)

static bool project_logic(rs_sensor_sample_t * p_sample)
{
    if (NULL == p_sample)
    {
        return false;
    }

    /* Rate-limit the GATT update so we don't flood the DA14531 with a GTL command per sample, and require
     * at least one axis to change by BLE_ICM_SENSOR_SENSITIVITY_THRESHOLD_MG so sensor noise at rest doesn't count as
     * a "changed" value. With smoothing off, raw jitter feeds this directly - if a stationary board keeps
     * publishing, raise BLE_ICM_SENSOR_SENSITIVITY_THRESHOLD_MG above the observed peak-to-peak of the accelerometer's
     * noise floor at rest. */
    static TickType_t s_last_tick   = 0;
    static int16_t    s_last_sent_x = 0;
    static int16_t    s_last_sent_y = 0;
    static int16_t    s_last_sent_z = 0;
    TickType_t        now = xTaskGetTickCount();

    int16_t diff_x = (int16_t) (p_sample->sensor_x - s_last_sent_x);
    if (diff_x < 0) { diff_x = (int16_t) (-diff_x); }
    int16_t diff_y = (int16_t) (p_sample->sensor_y - s_last_sent_y);
    if (diff_y < 0) { diff_y = (int16_t) (-diff_y); }
    int16_t diff_z = (int16_t) (p_sample->sensor_z - s_last_sent_z);
    if (diff_z < 0) { diff_z = (int16_t) (-diff_z); }
    int16_t max_diff = diff_x;
    if (diff_y > max_diff) { max_diff = diff_y; }
    if (diff_z > max_diff) { max_diff = diff_z; }

    if (((now - s_last_tick) >= pdMS_TO_TICKS(BLE_ICM_UPDATE_INTERVAL_MS)) &&
        (max_diff >= BLE_ICM_SENSOR_SENSITIVITY_THRESHOLD_MG))
    {
        s_last_tick   = now;
        s_last_sent_x = (int16_t)p_sample->sensor_x;
        s_last_sent_y = (int16_t)p_sample->sensor_y;
        s_last_sent_z = (int16_t)p_sample->sensor_z;
        return true;
    }

    return false;
}

/* Must equal RM_ICM42670_SENSOR_G in rm_icm42670_data_ready_mode.c: the driver reports m/s^2 using this value, so
 * dividing by it recovers g exactly. */
#define ICM_DRIVER_G_MPS2 RM_ICM42670_SENSOR_G

/* Driver output (m/s^2) -> milli-g, rounded to nearest and clamped to int16 so an out-of-range reading can never be
 * an undefined float-to-int conversion. */
static int16_t accel_mps2_to_mg (float accel_mps2)
{
    float mg = (accel_mps2 / ICM_DRIVER_G_MPS2) * 1000.0f;
    mg += (mg >= 0.0f) ? 0.5f : -0.5f; //round the value to the nearest whole number before the cast to int16_t.
    if (mg > (float) INT16_MAX) { return INT16_MAX; }
    if (mg < (float) INT16_MIN) { return INT16_MIN; }
    return (int16_t) mg;
}

/* Acquisition step: read one sample off the sensor, convert to milli-g, hand it to the BLE thread and
 * announce it if project_logic() (above) says it's worth sending. Runs in the ICM thread (it does I2C,
 * so it can never be called from an ISR - the data-ready interrupt would only wake the thread). */
static void data_cb (void)
{
    rm_icm42670_raw_data_t raw_data;
    RM_ICM42670_AccelRead(&g_icm42670_sensor0_ctrl, &raw_data);

    rm_icm42670_accel_data_t icm42670_accel_data;
    RM_ICM42670_AccelDataCalculate(&g_icm42670_sensor0_ctrl, &raw_data, &icm42670_accel_data);

    rs_sensor_sample_t accel_sample;
    accel_sample.sensor_x = (uint16_t) accel_mps2_to_mg(icm42670_accel_data.accel_x);
    accel_sample.sensor_y = (uint16_t) accel_mps2_to_mg(icm42670_accel_data.accel_y);
    accel_sample.sensor_z = (uint16_t) accel_mps2_to_mg(icm42670_accel_data.accel_z);

     if (project_logic(&accel_sample))
     {
        /* Every sample is announced; bits coalesce, so the BLE thread simply sees the newest one. */
        if (NULL != g_ble_event_group_handle)
        {
            rs_ble_sensor_tx(accel_sample);
        }
     }
}

/* ISR-context signal, called from icm42670_irq_callback() - see sensor_ble_bridge.h. */
void app_icm_drdy_isr_signal(void)
{
    if (NULL != s_icm_drdy_sem)
    {
        BaseType_t higher_priority_task_woken = pdFALSE;
        if (pdPASS != xSemaphoreGiveFromISR(s_icm_drdy_sem, &higher_priority_task_woken))
        {
            s_icm_drdy_give_fail_cnt++;
        }
        portYIELD_FROM_ISR(higher_priority_task_woken);
    }
}

fsp_err_t init_i2c_comm(void)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_comms_i2c_bus_extended_cfg_t * p_extend =
        (rm_comms_i2c_bus_extended_cfg_t *) g_icm42670_sensor0_cfg.p_comms_instance->p_cfg->p_extend;
    i2c_master_instance_t * p_driver_instance = (i2c_master_instance_t *) p_extend->p_driver_instance;

    err = p_driver_instance->p_api->open(p_driver_instance->p_ctrl, p_driver_instance->p_cfg);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    /* Create a semaphore for blocking if a semaphore is not NULL */
    if (NULL != p_extend->p_blocking_semaphore)
    {
        *(p_extend->p_blocking_semaphore->p_semaphore_handle) =
            xSemaphoreCreateCountingStatic((UBaseType_t) 1, (UBaseType_t) 0,
                                            p_extend->p_blocking_semaphore->p_semaphore_memory);
    }

    /* Create a recursive mutex for bus lock if a recursive mutex is not NULL */
    if (NULL != p_extend->p_bus_recursive_mutex)
    {
        *(p_extend->p_bus_recursive_mutex->p_mutex_handle) =
            xSemaphoreCreateRecursiveMutexStatic(p_extend->p_bus_recursive_mutex->p_mutex_memory);
    }

    return FSP_SUCCESS;
}

void icm_hw_init(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Open ICM42670 */
    g_icm42670_sensor0_ctrl.open = 0;
    err = RM_ICM42670_Open(&g_icm42670_sensor0_ctrl, &g_icm42670_sensor0_cfg);
    if (err != FSP_SUCCESS) { __BKPT(0); }

    g_icm42670_interrupt_cfg.int_config |= 0x01;
    err = RM_ICM42670_DeviceInterruptCfgSet(&g_icm42670_sensor0_ctrl, g_icm42670_interrupt_cfg);
    if (err != FSP_SUCCESS) { __BKPT(0); }

    err = RM_ICM42670_MeasurementStart(&g_icm42670_sensor0_ctrl);
    if (err != FSP_SUCCESS) { __BKPT(0); }
}

void icm42670_init(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Must exist before RM_ICM42670_Open() - see the comment on s_icm_drdy_sem above. */
    s_icm_drdy_sem = xSemaphoreCreateBinary();
    if (NULL == s_icm_drdy_sem)
    {
        MSG_PRINT("[FAIL] ICM42670 init: data-ready semaphore creation failed (out of heap), err=0x%X\n", (unsigned int) FSP_ERR_OUT_OF_MEMORY);
        __BKPT(0);
    }

    /* Init the I2C comms bus */
    err = init_i2c_comm();
    if (err != FSP_SUCCESS) 
    { 
        MSG_PRINT("[FAIL] ICM42670 init: I2C bus open failed, err=0x%X\n", (unsigned int) err); //change printf to MSG_PRINT
        __BKPT(0); 
    }

    /* Open ICM42670 */
    g_icm42670_sensor0_ctrl.open = 0;
    err = RM_ICM42670_Open(&g_icm42670_sensor0_ctrl, &g_icm42670_sensor0_cfg);
    if (err != FSP_SUCCESS) 
    {
        MSG_PRINT("[FAIL] ICM42670 init: RM_ICM42670_Open() failed - check sensor wiring/address, err=0x%X\n", (unsigned int) err);
        __BKPT(0); 
    }

    g_icm42670_interrupt_cfg.int_config |= 0x01;
    err = RM_ICM42670_DeviceInterruptCfgSet(&g_icm42670_sensor0_ctrl, g_icm42670_interrupt_cfg);
    if (err != FSP_SUCCESS)
    {
        MSG_PRINT("[FAIL] ICM42670 init: interrupt config (INT1) failed, err=0x%X\n", (unsigned int) err); 
        __BKPT(0); 
    }

    err = RM_ICM42670_MeasurementStart(&g_icm42670_sensor0_ctrl);
    if (err != FSP_SUCCESS) 
    { 
        MSG_PRINT("[FAIL] ICM42670 init: MeasurementStart failed, err=0x%X\n", (unsigned int) err); 
        __BKPT(0); 
    }
}

/* Data-ready wait timeout for the current ODR: a few sample periods, clamped to [MIN, MAX]. */
static TickType_t icm_drdy_timeout_ticks(uint16_t odr_hz)
{
    uint32_t ms = ICM_DRDY_TIMEOUT_MAX_MS;
    if (0U != odr_hz)
    {
        ms = (ICM_DRDY_TIMEOUT_PERIODS * 1000U) / odr_hz;
        if (ms < ICM_DRDY_TIMEOUT_MIN_MS) { ms = ICM_DRDY_TIMEOUT_MIN_MS; }
        if (ms > ICM_DRDY_TIMEOUT_MAX_MS) { ms = ICM_DRDY_TIMEOUT_MAX_MS; }
    }
    TickType_t ticks = pdMS_TO_TICKS(ms);
    return (0 == ticks) ? 1 : ticks;
}

/* ICM Thread entry function */
/* pvParameters contains TaskHandle_t */
void sensor_main(void)
{
#ifdef RTT_DEBUG_ON
    uint32_t last_reported_give_fail_cnt = 0;
#endif

    icm42670_init();

    s_icm_drdy_timeout_ticks = icm_drdy_timeout_ticks(g_icm42670_sensor0_extended_cfg.accel_odr);
    MSG_PRINT("ICM42670 ready\n");
    while (1)
    {
        /* Apply a pending Accel ODR change (no-op if none). Done here, outside the
         * data-ready wait, so the sensor is never reconfigured mid-read. */
        icm_odr_apply_pending();

        /* Wakes on: INT1 data-ready ISR, BLE ODR write, or timeout (lost-edge safety net).
         * After an ODR write, data_cb() runs once at the old ODR
         * before the change is applied next loop. */
        if ( pdTRUE != xSemaphoreTake(s_icm_drdy_sem, s_icm_drdy_timeout_ticks))
        {
           MSG_PRINT("Warning: ICM42670 data-ready semaphore timeout\n");
        }

#ifdef RTT_DEBUG_ON
        /* Report missed data-ready signals from thread context (RTT printf is not ISR-safe, so the ISR only
         * counts). Printed only when the count has changed, so it cannot flood RTT. */
        uint32_t give_fail_cnt = s_icm_drdy_give_fail_cnt;
        if (give_fail_cnt != last_reported_give_fail_cnt)
        {
            MSG_PRINT("Warning: ICM42670 data-ready overrun, %u missed\n", (unsigned) give_fail_cnt);
            last_reported_give_fail_cnt = give_fail_cnt;
        }
#endif

        data_cb();
    }
}
