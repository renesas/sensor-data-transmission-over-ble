#include "rs_ble_sensor_data_tx.h"

/* g_conn_hdl is defined in the QE-generated app/ble/app_main.c but has external linkage - gap_cb()
 * sets/clears it there, this file only reads it to decide whether to notify. */
extern uint16_t g_conn_hdl;


/* g_ble_abs0_ctrl/g_ble_abs0_cfg are FSP-generated (ra_gen/ble_thread.c) */
extern ble_abs_instance_ctrl_t g_ble_abs0_ctrl;
extern const ble_abs_cfg_t     g_ble_abs0_cfg;

/* g_ble_event_group_handle is created by app_main() (app/ble/app_main.c) before its main loop -
 * rs_ble_sensor_tx() sets BLE_SENSOR_TX_EVT_BIT on it to wake the BLE thread for a fresh sample. */
extern EventGroupHandle_t g_ble_event_group_handle;

rs_sensor_sample_t g_sensor_accel_sample;

void rs_ble_sensor_tx(rs_sensor_sample_t accel_data)
{
    g_sensor_accel_sample = accel_data;
    xEventGroupSetBits(g_ble_event_group_handle, RS_BLE_SENSOR_TX_EVT_BIT);
}

ble_status_t rs_ble_sensor_sample(rs_sensor_sample_t sensor_sample_in)
{
    rs_sensor_sample_t sensor_sample = sensor_sample_in;
    ble_status_t status = BLE_ERR_UNSPECIFIED;

    if (BLE_GAP_INVALID_CONN_HDL != g_conn_hdl)
    {
        /* Field-by-field copy generated from the same BLE_SENSORSERVICES_SENSORDATA_FIELDS list both structs
         * are built from (sensor_data_fields.h) - member names always match, so adding a field to that list
         * is enough for it to be copied here too, no manual mapping required. */
        st_ble_sensorservices_sensordata_t accel_data;
#define RS_SENSOR_SAMPLE_COPY_FIELD(name) accel_data.name = (uint16_t) sensor_sample.name;
        BLE_SENSORSERVICES_SENSORDATA_FIELDS(RS_SENSOR_SAMPLE_COPY_FIELD)
#undef RS_SENSOR_SAMPLE_COPY_FIELD

        /* Returns BLE_ERR_INVALID_OPERATION when the peer has not enabled notifications on the CCCD
         * (r_ble_servs_if.c:679) - nothing is sent and that is expected, not a fault. SendHdlVal reads the
         * CCCD itself on every call (r_ble_servs_if.c:650); for a CCCD handle that is a local table lookup,
         * not a GTL round trip. Callers that ignore this status therefore cannot distinguish
         * "not subscribed" from a real send failure. */
        status = R_BLE_SENSORSERVICES_NotifySensordata(g_conn_hdl, &accel_data);
    }
    return status;
}
