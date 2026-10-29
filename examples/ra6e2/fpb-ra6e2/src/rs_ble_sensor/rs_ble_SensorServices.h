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
 * File Name: rs_ble_SensorServices.h
 * Version : 1.0
 * Description : The header file for Sensor Service service.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * History : DD.MM.YYYY Version Description
 *         : 31.12.2999 1.00 First Release
 ***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @file
 * @defgroup SensorServices Sensor Service Service
 * @{
 * @ingroup profile
 * @brief
 **********************************************************************************************************************/
#include "profile_cmn/r_ble_servs_if.h"
#include "gatt_db.h"
#include "sensor_data_fields.h"

#ifndef R_BLE_SENSORSERVICES_H
#define R_BLE_SENSORSERVICES_H

/*----------------------------------------------------------------------------------------------------------------------
    Sensor Data Characteristic
        Client Characteristic Configuration descriptor : Client Characteristic Configuration Descriptor
----------------------------------------------------------------------------------------------------------------------*/

/***************************************************************************//**
 * @brief Sensor Data value structure.
 * Fields come from BLE_SENSORSERVICES_SENSORDATA_FIELDS - see sensor_data_fields.h.
*******************************************************************************/
typedef struct {
    BLE_SENSORSERVICES_SENSORDATA_FIELDS(BLE_SENSORSERVICES_SENSORDATA_DECL_U16_FIELD)
} st_ble_sensorservices_sensordata_t;

/* Guards the one invariant sensor_data_fields.h relies on: every field must be uint16_t, so this struct is
 * a flat, unpadded array of uint16_t matching BLE_SENSORSERVICES_SENSORDATA_LEN exactly. If this ever
 * fires, a non-uint16_t field was added - the memcpy-based encode/decode in rs_ble_SensorServices.c would
 * otherwise silently corrupt the notified payload instead of failing to compile. */
_Static_assert(sizeof(st_ble_sensorservices_sensordata_t) == BLE_SENSORSERVICES_SENSORDATA_LEN,
    "sensor data fields must all be uint16_t - see sensor_data_fields.h");

/***************************************************************************//**
 * @brief     Send notification of  Sensor Data characteristic value to the remote device.
 * @param[in] conn_hdl Connection handle.
 * @param[in] p_value  Characteristic value to send.
 * @return    @ref ble_status_t
*******************************************************************************/
ble_status_t R_BLE_SENSORSERVICES_NotifySensordata(uint16_t conn_hdl, const st_ble_sensorservices_sensordata_t *p_value);

/***************************************************************************//**
 * @brief     Set Sensor Data cli cnfg descriptor value to the local GATT database.
 * @param[in] conn_hdl Connection handle.
 * @param[in] p_value  Descriptor value to set.
 * @return    @ref ble_status_t
*******************************************************************************/
ble_status_t R_BLE_SENSORSERVICES_SetSensordataCliCnfg(uint16_t conn_hdl, const uint16_t *p_value);

/***************************************************************************//**
 * @brief     Get Sensor Data cli cnfg descriptor value from the local GATT database.
 * @param[in] conn_hdl Connection handle.
 * @param[out] p_value  Output location for the acquired descriptor value.
 * @return    @ref ble_status_t
*******************************************************************************/
ble_status_t R_BLE_SENSORSERVICES_GetSensordataCliCnfg(uint16_t conn_hdl, uint16_t *p_value);

/*----------------------------------------------------------------------------------------------------------------------
    Sensor Service Service
----------------------------------------------------------------------------------------------------------------------*/

/***************************************************************************//**
 * @brief Sensor Service characteristic Index.
*******************************************************************************/
typedef enum {
    BLE_SENSORSERVICES_SENSORDATA_IDX,
    BLE_SENSORSERVICES_SENSORDATA_CLI_CNFG_IDX,
} e_ble_sensorservices_char_idx_t;

/***************************************************************************//**
 * @brief Sensor Service event type.
*******************************************************************************/
typedef enum {
    /* Sensor Data */
    BLE_SENSORSERVICES_EVENT_SENSORDATA_CLI_CNFG_WRITE_REQ = BLE_SERVS_ATTR_EVENT(BLE_SENSORSERVICES_SENSORDATA_CLI_CNFG_IDX, BLE_SERVS_WRITE_REQ),
    BLE_SENSORSERVICES_EVENT_SENSORDATA_CLI_CNFG_WRITE_COMP = BLE_SERVS_ATTR_EVENT(BLE_SENSORSERVICES_SENSORDATA_CLI_CNFG_IDX, BLE_SERVS_WRITE_COMP),
    BLE_SENSORSERVICES_EVENT_SENSORDATA_CLI_CNFG_READ_REQ = BLE_SERVS_ATTR_EVENT(BLE_SENSORSERVICES_SENSORDATA_CLI_CNFG_IDX, BLE_SERVS_READ_REQ),
} e_ble_sensorservices_event_t;

/***************************************************************************//**
 * @brief     Initialize Sensor Service service.
 * @param[in] cb Service callback.
 * @return    @ref ble_status_t
*******************************************************************************/
ble_status_t R_BLE_SENSORSERVICES_Init(ble_servs_app_cb_t cb);

#endif /* R_BLE_SENSORSERVICES_H */

/** @} */
