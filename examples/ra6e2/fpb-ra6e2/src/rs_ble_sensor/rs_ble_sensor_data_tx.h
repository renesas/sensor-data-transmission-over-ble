#ifndef RS_BLE_SENSOR_DATA_TX_H_
#define RS_BLE_SENSOR_DATA_TX_H_

/***********************************************************************************************************************
 * Cross-thread handoff and GATT notify glue between the sensor thread and the BLE thread.
 **********************************************************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "hal_data.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "r_ble_api.h"
#include "rm_ble_abs.h"
#include "rm_ble_abs_api.h"
#include "sensor_ble_bridge.h"
#include "rs_ble_SensorServices.h"
#include "sensor_data_fields.h"

#define RS_BLE_SENSOR_TX_EVT_BIT BLE_SENSOR_TX_EVT_BIT

/* Same field list as st_ble_sensorservices_sensordata_t (rs_ble_SensorServices.h) - see sensor_data_fields.h.
 * Kept as a separate struct/type from the GATT one by design (see rs_ble_sensor_sample()), but sharing the
 * field list means a field added there automatically appears here too, with matching member names. */
typedef struct
{
    BLE_SENSORSERVICES_SENSORDATA_FIELDS(BLE_SENSORSERVICES_SENSORDATA_DECL_U16_FIELD)
} rs_sensor_sample_t;

extern rs_sensor_sample_t g_sensor_accel_sample;
/*----------------------------------------------------------------------------------------------------------------------
    Sensor-> BLE handoff. Called from the sensor thread (data_cb(), sensor_main.c) with the newest sample already
    converted to milli-g. Stores it into g_sensor_accel_sample and wakes the BLE thread via BLE_SENSOR_TX_EVT_BIT -
    see sensor_ble_bridge.h. */

void rs_ble_sensor_tx(rs_sensor_sample_t accel_data);

/*----------------------------------------------------------------------------------------------------------------------
    Sensor -> GATT publish. Takes the sample handed off by the sensor thread and pushes it via R_BLE_SENSORSERVICES_NotifySensordata() if connected. Call it from the BLE
    main loop:
 *     app_main() main loop:
 *         rs_ble_sensor_sample(sensor_sample_in); */
ble_status_t rs_ble_sensor_sample(rs_sensor_sample_t sensor_sample_in);

/* Called from the BLE side when the phone writes Sensor ODR. Only records the request - safe from any thread. */
void rs_sensor_write_request_cb(uint16_t odr_hz);

void sensor_main(void);
fsp_err_t init_i2c_comm(void);
void icm_hw_init(void);

#endif /* RS_BLE_SENSOR_DATA_TX_H_ */
