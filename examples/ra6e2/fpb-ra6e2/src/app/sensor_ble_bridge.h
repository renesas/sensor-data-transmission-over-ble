#ifndef sensor_ble_bridge_H_
#define sensor_ble_bridge_H_

#include "FreeRTOS.h"
#include "semphr.h"
#include <stdint.h>
#include <string.h>
#ifdef RTT_DEBUG_ON
#include "SEGGER_RTT.h"
#endif


/*----------------------------------------------------------------------------------------------------------------------
    Event-group handoff: the ICM thread signals the BLE thread that a fresh sample is waiting, instead of the
    BLE thread discovering it on its 100 ms periodic wake.

    The bit is set into the BLE task's EXISTING event group (g_ble_event_group_handle), which that task is
    already blocked on for GTL events - so this costs no extra RTOS object and no extra wait. The wait mask is
    widened in the xEventGroupWaitBits() call in app_main() (src/app/ble/app_main.c).

    BLE_SENSOR_TX_EVT_BIT  must not collide with BLE_EVENT_PATTERN (0x0A0A -> bits 1,3,9,11) which the GTL driver
    sets from both task and ISR context. configUSE_16_BIT_TICKS is 0, so bits 0..23 are usable; bit 16 is well
    clear of anything BLE uses.

    NOTE: event group bits COALESCE - if the ICM thread sets the bit several times while the BLE task is busy,
    the BLE task wakes once and sees only the newest sample. That is accepted by design here; only the latest
    sample is of interest.
----------------------------------------------------------------------------------------------------------------------*/
#define BLE_SENSOR_TX_EVT_BIT     (0x010000)

/*----------------------------------------------------------------------------------------------------------------------
    Data-ready interrupt: RM_ICM42670_Open() opens this IRQ, registers rm_icm42670_irq_callback -> icm42670_irq_callback
    and enables it.
----------------------------------------------------------------------------------------------------------------------*/
#define ICM_DRDY_TIMEOUT_PERIODS (4)     /* missed sample periods before declaring a lost edge */
/* Limits follow from the ODR limits (ICM_ODR_MIN_HZ / ICM_ODR_MAX_HZ, defined below): ICM_DRDY_TIMEOUT_PERIODS
 * sample periods at the fastest ODR (floor, rounded up) and at the slowest ODR (cap). The cap also bounds how long
 * an ODR write waits to be applied. Safety net only - covers a missed/lost edge, not the normal path. */
#define ICM_DRDY_TIMEOUT_MIN_MS  ((ICM_DRDY_TIMEOUT_PERIODS * 1000U + ICM_ODR_MAX_HZ - 1U) / ICM_ODR_MAX_HZ)  /* 3 ms  */
#define ICM_DRDY_TIMEOUT_MAX_MS  ((ICM_DRDY_TIMEOUT_PERIODS * 1000U) / ICM_ODR_MIN_HZ)                          /* 333 ms */

/* Called from icm42670_irq_callback() (ISR context) on every data-ready interrupt. */
void app_icm_drdy_isr_signal(void);

/*----------------------------------------------------------------------------------------------------------------------
    Runtime accelerometer ODR control, driven by the phone writing the Sensor ODR characteristic of the
    Sensor Write Service. The compile-time ODR is set in
    src/app/rm_icm42670/rm_icm42670_hal_data.c (.accel_odr).

    Threading: the write arrives on the BLE thread, but the ICM42670 sits on I2C which is owned exclusively by
    the ICM thread. Reconfiguring the sensor from the BLE thread would race the ICM thread's in-flight I2C
    transfers, so the BLE side only *records* the request (rs_sensor_write_request_cb) and the ICM thread applies it
    at a safe point in its own loop.
----------------------------------------------------------------------------------------------------------------------*/

#include <stdint.h>

/* Bounds enforced by rm_icm42670_accel_configuration() for LN mode (see rm_icm42670_data_ready_mode.c:606).
 * A write outside this range is rejected and the current ODR is kept, rather than pushing the driver into its
 * error path mid-run. */
#define ICM_ODR_MIN_HZ (12)
#define ICM_ODR_MAX_HZ (1600)

#ifdef RTT_DEBUG_ON
extern int SEGGER_RTT_printf(unsigned BufferIndex, const char * sFormat, ...);

/** Print a NUL-terminated string to RTT channel 0. No formatting. */
#define MSG_PRINT(...) ((void) SEGGER_RTT_printf(0, __VA_ARGS__))
#else
#define MSG_PRINT(...) ((void) 0)
#endif
#endif
