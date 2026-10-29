/* HAL source file of icm42670 */
#include <rm_icm42670_hal_data.h>


extern rm_icm42670_api_t const g_icm42670_on_icm42670_accel_gyro;
rm_icm42670_mode_extended_cfg_t g_icm42670_sensor0_extended_cfg =
{ .p_api = (void*) &g_icm42670_on_icm42670_accel_gyro,
  .mode_irq = RM_ICM42670_OPERATION_MODE_6AXIS,
  .interrupt_source = RM_ICM42670_INTERRUPT_SOURCE_DRDY_INT1_EN,
  .interrupt_config = RM_ICM42670_INTERRUPT_CONFIG_MASK_INT1,
  .pwr_rc_idle = RM_ICM42670_PWR_RC_ON,
  .clksel = RM_ICM42670_SELECT_RC_OSC,

  .accel_sensor_mode = RM_ICM42670_ACCEL_SENSOR_MODE_LN,
  .accel_fs = 2,
  .accel_odr = 100, //means 100 Hz, i.e. 100 samples per second (one new reading every 10 ms)
  .accel_lp_clk_sel = RM_ICM42670_ACCEL_CLK_SEL_WAKEUP_OSC,
  .accel_ui_filt_bw = RM_ICM42670_ACCEL_UI_FILT_BYPASSED,
  .accel_ui_avg = RM_ICM42670_ACCEL_UI_AVG_2x,

  .gyro_sensor_mode = RM_ICM42670_GYRO_SENSOR_MODE_OFF ,
  .gyro_fs = 1000,
  .gyro_odr = 25,
  .gyro_ui_filt_bw = RM_ICM42670_GYRO_UI_FILT_BYPASSED,

  .temp_filt_bw = RM_ICM42670_TEMP_DLPF_BYPASSED, };

SemaphoreHandle_t g_icm42670_sensor0_semaphore_handle;
StaticSemaphore_t g_icm42670_sensor0_semaphore_memory;

/* Semaphore to wait for callback  */
rm_icm42670_semaphore_t g_icm42670_sensor0_semaphore =
{
    .p_semaphore_handle = &g_icm42670_sensor0_semaphore_handle,
    .p_semaphore_memory = &g_icm42670_sensor0_semaphore_memory,
};

rm_icm42670_device_interrupt_cfg_t g_icm42670_interrupt_cfg =
{
  .int_type = RM_ICM42670_OPERATION_INTERRUPT_TYPE_INT1,
  .int_source = RM_ICM42670_INTERRUPT_SOURCE_DRDY_INT1_EN,
  .int_config = RM_ICM42670_INTERRUPT_CONFIG_MASK_INT1,
};

rm_icm42670_instance_ctrl_t g_icm42670_sensor0_ctrl;
const rm_icm42670_cfg_t g_icm42670_sensor0_cfg =
{
  .p_semaphore = &g_icm42670_sensor0_semaphore,
  .semaphore_timeout = 0xFFFFFFFF,
  .p_comms_instance = &g_comms_i2c_device1,
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == g_external_irq14)
    .p_irq_instance = NULL,
    .p_irq_callback = NULL,
#else
  .p_irq_instance = &g_external_irq6_pmod1,
  .p_irq_callback = icm42670_irq_callback,
#endif
#undef RA_NOT_DEFINED
  .p_comms_callback = icm42670_comms_i2c_callback,
#if defined(NULL)
    .p_context           = NULL,
#else
  .p_context = &NULL,
#endif
  .p_extend = (void*) &g_icm42670_sensor0_extended_cfg, };

 const rm_icm42670_instance_t g_icm42670_sensor0 =
{ .p_ctrl = &g_icm42670_sensor0_ctrl, .p_cfg = &g_icm42670_sensor0_cfg, .p_api = &g_icm42670_on_icm42670, };
