/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
* other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
* EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
* SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
* SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
* this software. By using this software, you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2019-2020 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
 * File Name: rs_ble_SensorServices.c
 * Version : 1.0
 * Description : The source file for Sensor Service service.
 **********************************************************************************************************************/

#include "rs_ble_SensorServices.h"
#include "profile_cmn/r_ble_servs_if.h"
#include "gatt_db.h"
#include <string.h>

static st_ble_servs_info_t gs_servs_info;

/*----------------------------------------------------------------------------------------------------------------------
    Sensor Data Client Characteristic Configuration descriptor : Client Characteristic Configuration Descriptor
----------------------------------------------------------------------------------------------------------------------*/

static const st_ble_servs_desc_info_t gs_sensordata_cli_cnfg = {
    .attr_hdl = BLE_SENSORSERVICES_SENSORDATA_CLI_CNFG_DESC_HDL,
    .app_size = sizeof(uint16_t),
    .desc_idx = BLE_SENSORSERVICES_SENSORDATA_CLI_CNFG_IDX,
    .db_size  = BLE_SENSORSERVICES_SENSORDATA_CLI_CNFG_LEN,
    .decode   = (ble_servs_attr_decode_t)decode_uint16_t,
    .encode   = (ble_servs_attr_encode_t)encode_uint16_t,
};

ble_status_t R_BLE_SENSORSERVICES_SetSensordataCliCnfg(uint16_t conn_hdl, const uint16_t *p_value)
{
    return R_BLE_SERVS_SetDesc(&gs_sensordata_cli_cnfg, conn_hdl, (const void *)p_value);
}

ble_status_t R_BLE_SENSORSERVICES_GetSensordataCliCnfg(uint16_t conn_hdl, uint16_t *p_value)
{
    return R_BLE_SERVS_GetDesc(&gs_sensordata_cli_cnfg, conn_hdl, (void *)p_value);
}

/*----------------------------------------------------------------------------------------------------------------------
    Sensor Data characteristic
----------------------------------------------------------------------------------------------------------------------*/

static ble_status_t decode_st_ble_sensorservices_sensordata_t(st_ble_sensorservices_sensordata_t *p_app_value, const st_ble_gatt_value_t *p_gatt_value)
{
    /* Plain memcpy of BLE_SENSORSERVICES_SENSORDATA_LEN bytes - correct as long as every field listed in
     * BLE_SENSORSERVICES_SENSORDATA_FIELDS (sensor_data_fields.h) is a uint16_t: the struct is then a flat,
     * unpadded array of uint16_t, and this MCU is little-endian, matching the original field-by-field
     * encoding byte-for-byte. Adding/removing a field there needs no change here. */
    if ((NULL == p_app_value) || (NULL == p_gatt_value) || (NULL == p_gatt_value->p_value) ||
        (p_gatt_value->value_len < BLE_SENSORSERVICES_SENSORDATA_LEN))
    {
        return BLE_ERR_INVALID_DATA;
    }
    memcpy(p_app_value, p_gatt_value->p_value, BLE_SENSORSERVICES_SENSORDATA_LEN);
    return BLE_SUCCESS;
}

static ble_status_t encode_st_ble_sensorservices_sensordata_t(const st_ble_sensorservices_sensordata_t *p_app_value, st_ble_gatt_value_t *p_gatt_value)
{
    /* See decode_st_ble_sensorservices_sensordata_t() above. */
    if ((NULL == p_app_value) || (NULL == p_gatt_value) || (NULL == p_gatt_value->p_value) ||
        (p_gatt_value->value_len < BLE_SENSORSERVICES_SENSORDATA_LEN))
    {
        return BLE_ERR_INVALID_DATA;
    }
    memcpy(p_gatt_value->p_value, p_app_value, BLE_SENSORSERVICES_SENSORDATA_LEN);
    return BLE_SUCCESS;
}

/* Sensor Data characteristic descriptor definition */
static const st_ble_servs_desc_info_t *gspp_sensordata_descs[] = {
    &gs_sensordata_cli_cnfg,
};

/* Sensor Data characteristic definition */
static const st_ble_servs_char_info_t gs_sensordata_char = {
    .start_hdl    = BLE_SENSORSERVICES_SENSORDATA_DECL_HDL,
    .end_hdl      = BLE_SENSORSERVICES_SENSORDATA_CLI_CNFG_DESC_HDL,
    .char_idx     = BLE_SENSORSERVICES_SENSORDATA_IDX,
    .app_size     = sizeof(st_ble_sensorservices_sensordata_t),
    .db_size      = BLE_SENSORSERVICES_SENSORDATA_LEN,
    .decode       = (ble_servs_attr_decode_t)decode_st_ble_sensorservices_sensordata_t,
    .encode       = (ble_servs_attr_encode_t)encode_st_ble_sensorservices_sensordata_t,
    .pp_descs     = gspp_sensordata_descs,
    .num_of_descs = ARRAY_SIZE(gspp_sensordata_descs),
};

ble_status_t R_BLE_SENSORSERVICES_NotifySensordata(uint16_t conn_hdl, const st_ble_sensorservices_sensordata_t *p_value)
{
    return R_BLE_SERVS_SendHdlVal(&gs_sensordata_char, conn_hdl, (const void *)p_value, true);
}

/*----------------------------------------------------------------------------------------------------------------------
    Sensor Service server
----------------------------------------------------------------------------------------------------------------------*/

/* Sensor Service characteristics definition */
static const st_ble_servs_char_info_t *gspp_chars[] = {
    &gs_sensordata_char,
};

/* Sensor Service service definition */
static st_ble_servs_info_t gs_servs_info = {
    .pp_chars     = gspp_chars,
    .num_of_chars = ARRAY_SIZE(gspp_chars),
};

ble_status_t R_BLE_SENSORSERVICES_Init(ble_servs_app_cb_t cb)
{
    if (NULL == cb)
    {
        return BLE_ERR_INVALID_PTR;
    }

    gs_servs_info.cb = cb;

    return R_BLE_SERVS_RegisterServer(&gs_servs_info);
}
