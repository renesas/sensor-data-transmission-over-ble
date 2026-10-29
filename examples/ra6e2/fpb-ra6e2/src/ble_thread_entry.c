#include "ble_thread.h"
/* BLE Thread entry function */
/* pvParameters contains TaskHandle_t */
extern void app_main(void);

void ble_thread_entry(void *pvParameters)
{
    FSP_PARAMETER_NOT_USED(pvParameters);
    app_main();
}

/* Stub: rm_ble_abs_gtl_storage.c's real implementation is compiled out
 * because BLE_ABS_CFG_ENABLE_SECURE_DATA is 0, but g_vee_cfg in
 * ra_gen/ble_thread.c references this callback unconditionally. */
void vee_callback(rm_vee_callback_args_t *p_args)
{
    FSP_PARAMETER_NOT_USED(p_args);
}
