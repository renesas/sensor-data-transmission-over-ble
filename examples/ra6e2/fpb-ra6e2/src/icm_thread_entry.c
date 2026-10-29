#include "icm_thread.h"
extern void sensor_main(void);

/* ICM Thread entry function */
/* pvParameters contains TaskHandle_t */
void icm_thread_entry(void *pvParameters)
{
    FSP_PARAMETER_NOT_USED (pvParameters);

    sensor_main();
}
