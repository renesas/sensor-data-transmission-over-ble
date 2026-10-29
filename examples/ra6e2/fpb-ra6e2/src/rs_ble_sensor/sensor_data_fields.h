#ifndef SENSOR_DATA_FIELDS_H_
#define SENSOR_DATA_FIELDS_H_

/***********************************************************************************************************************
 * Sensor Data characteristic layout - EDIT THIS FILE to change the fields.
 *
 * This is the single place that should define the Sensor Data GATT characteristic's on-wire layout. Everything else derives from the list below:
 *   - st_ble_sensorservices_sensordata_t (rs_ble_SensorServices.h) is built from this list.
 *   - BLE_SENSORSERVICES_SENSORDATA_LEN (gatt_db.h) is computed from this list (2 bytes per field).
 *   - The GATT DB characteristic entry [14] in qe_ble_profile.c sizes .value_length / .value from
 *     BLE_SENSORSERVICES_SENSORDATA_LEN, so it never needs to be hand-edited when a field is added.
 *   - encode/decode in rs_ble_SensorServices.c is a plain memcpy of BLE_SENSORSERVICES_SENSORDATA_LEN bytes,
 *     so it does not need editing either.
 *
 * To add/remove/rename a field, only change the X(...) list below. Every field must be uint16_t
 * (little-endian on the wire, matching this MCU's native byte order) - do not add other types here,
 * since the memcpy-based encode/decode in rs_ble_SensorServices.c assumes a flat, unpadded uint16_t array.
 **********************************************************************************************************************/
#define BLE_SENSORSERVICES_SENSORDATA_FIELDS(X) \
    X(sensor_x) \
    X(sensor_y) \
    X(sensor_z)

/* Number of fields listed above - used only to compute BLE_SENSORSERVICES_SENSORDATA_LEN below. */
#define QE_BLE_SENSORDATA_COUNT_FIELD(name) +1
#define BLE_SENSORSERVICES_SENSORDATA_LEN \
    (2 * (0 BLE_SENSORSERVICES_SENSORDATA_FIELDS(QE_BLE_SENSORDATA_COUNT_FIELD)))

/* Shared "declare a struct member" callbacks for the X-macro list above, used by every struct built from
 * it - st_ble_sensorservices_sensordata_t (rs_ble_SensorServices.h) and rs_sensor_sample_t
 * (rs_ble_sensor_data_tx.h) - so the declare-a-field boilerplate is defined once instead of once per struct.
 * Left defined (not #undef'd) since both consumers use it directly in their own typedef. */
#define BLE_SENSORSERVICES_SENSORDATA_DECL_U16_FIELD(name) uint16_t name;

#endif /* SENSOR_DATA_FIELDS_H_ */
