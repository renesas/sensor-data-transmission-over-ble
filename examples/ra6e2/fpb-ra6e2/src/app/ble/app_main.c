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
* Copyright (C) 2019-2025 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/******************************************************************************
* File Name    : app_main.c
* Tool-Chain   : e2Studio
* Description  : This is a application file for peripheral role.
*******************************************************************************/

/******************************************************************************
 Includes   <System Includes> , "Project Includes"
*******************************************************************************/
#include <string.h>
#include "r_ble_api.h"
#include "rm_ble_abs.h"
#include "rm_ble_abs_api.h"
#include "gatt_db.h"
#include "profile_cmn/r_ble_servs_if.h"
#include "profile_cmn/r_ble_servc_if.h"
#include "hal_data.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#define BLE_EVENT_PATTERN   (0x0A0A)
EventGroupHandle_t  g_ble_event_group_handle;
#include "r_ble_gat2s.h"
#include "rs_ble_SensorServices.h"
#include "rs_ble_SensorRates.h"

/******************************************************************************
 User file includes
*******************************************************************************/
/* Start user code for file includes. Do not edit comment generated here */
#include "qe_ble_profile.h"
#include "ble_thread.h"
#include "sensor_ble_bridge.h"
#include "rs_ble_sensor_data_tx.h"
/* End user code. Do not edit comment generated here */

#define BLE_LOG_TAG "app_main"
#define BLE_GATTS_QUEUE_ELEMENTS_SIZE       (14)
#define BLE_GATTS_QUEUE_BUFFER_LEN          (245)
#define BLE_GATTS_QUEUE_NUM                 (1)
#define ENABLE_SENSOR_EVT_TO_BLE (1)  /* Set to 0 to disable the ICM->GATT publish logic in the main loop and in rs_ble_sensor_sample() */

/******************************************************************************
 User macro definitions
*******************************************************************************/
/* Start user code for macro definitions. Do not edit comment generated here */
/* The GATT profile is hand-maintained. */
/* End user code. Do not edit comment generated here */

/******************************************************************************
 Generated function prototype declarations
*******************************************************************************/
/* Internal functions */
void gap_cb(uint16_t type, ble_status_t result, st_ble_evt_data_t *p_data);
void gatts_cb(uint16_t type, ble_status_t result, st_ble_gatts_evt_data_t *p_data);
void gattc_cb(uint16_t type, ble_status_t result, st_ble_gattc_evt_data_t *p_data);
void vs_cb(uint16_t type, ble_status_t result, st_ble_vs_evt_data_t *p_data);
ble_status_t ble_init(void);
void app_main(void);

/******************************************************************************
 User function prototype declarations
*******************************************************************************/
/* Start user code for function prototype declarations. Do not edit comment generated here */
/* Called from the BLE side when the phone writes Sensor ODR. Only records the request - safe from any thread. */
extern void rs_sensor_write_request_cb(uint16_t odr_hz);
/* End user code. Do not edit comment generated here */

/******************************************************************************
 Generated global variables
*******************************************************************************/
/* Advertising Data */
static uint8_t gs_advertising_data[] =
{
    /* Flags */
    0x02, /**< Data Size */
    0x01, /**< Data Type */
    ( 0x06 ), /**< Data Value */

    /* Complete Local Name */
    0x19, /**< Data Size */
    0x09, /**< Data Type */
    0x72, 0x61, 0x36, 0x65, 0x32, 0x5f, 0x64, 0x61, 0x31, 0x34, 0x35, 0x33, 0x31, 0x5f, 0x42, 0x4c, 0x45, 0x5f, 0x53, 0x65, 0x6e, 0x73, 0x6f, 0x72, /**< Data Value */
};

ble_abs_legacy_advertising_parameter_t g_ble_advertising_parameter =
{
 .p_peer_address             = NULL,       ///< Peer address.
 .slow_advertising_interval  = 0x000000A0, ///< Slow advertising interval. 100.0(ms)
 .slow_advertising_period    = 0x0000,     ///< Slow advertising period.
 .p_advertising_data         = gs_advertising_data,             ///< Advertising data. If p_advertising_data is specified as NULL, advertising data is not set.
 .advertising_data_length    = ARRAY_SIZE(gs_advertising_data), ///< Advertising data length (in bytes).
 .advertising_filter_policy  = BLE_ABS_ADVERTISING_FILTER_ALLOW_ANY, ///< Advertising Filter Policy.
 .advertising_channel_map    = ( BLE_GAP_ADV_CH_37 | BLE_GAP_ADV_CH_38 | BLE_GAP_ADV_CH_39 ), ///< Channel Map.
 .own_bluetooth_address_type = BLE_GAP_ADDR_RAND, ///< Own Bluetooth address type.
 .own_bluetooth_address      = { 0 },
};

/* GATT server callback parameters */
ble_abs_gatt_server_callback_set_t gs_abs_gatts_cb_param[] =
{
    {
        .gatt_server_callback_function = gatts_cb,
        .gatt_server_callback_priority = 1,
    },
    {
        .gatt_server_callback_function = NULL,
    }
};

/* GATT client callback parameters */
ble_abs_gatt_client_callback_set_t gs_abs_gattc_cb_param[] =
{
    {
        .gatt_client_callback_function = gattc_cb,
        .gatt_client_callback_priority = 1,
    },
    {
        .gatt_client_callback_function = NULL,
    }
};

/* GATT server Prepare Write Queue parameters */
static st_ble_gatt_queue_elm_t  gs_queue_elms[BLE_GATTS_QUEUE_ELEMENTS_SIZE];
static uint8_t gs_buffer[BLE_GATTS_QUEUE_BUFFER_LEN];
static st_ble_gatt_pre_queue_t gs_queue[BLE_GATTS_QUEUE_NUM] = {
    {
        .p_buf_start = gs_buffer,
        .buffer_len  = BLE_GATTS_QUEUE_BUFFER_LEN,
        .p_queue     = gs_queue_elms,
        .queue_size  = BLE_GATTS_QUEUE_ELEMENTS_SIZE,
    }
};

/* Connection handle */
uint16_t g_conn_hdl = BLE_GAP_INVALID_CONN_HDL;

/******************************************************************************
 User global variables
*******************************************************************************/
/* Start user code for global variables. Do not edit comment generated here */
/* The ICM->GATT publish logic is rs_ble_sensor_sample() in src/rs_ble_sensor/rs_ble_sensor_data_tx.c. */
/* End user code. Do not edit comment generated here */

/******************************************************************************
 Generated function definitions
*******************************************************************************/
/******************************************************************************
 * Function Name: gap_cb
 * Description  : Callback function for GAP API.
 * Arguments    : uint16_t type -
 *                  Event type of GAP API.
 *              : ble_status_t result -
 *                  Event result of GAP API.
 *              : st_ble_vs_evt_data_t *p_data -
 *                  Event parameters of GAP API.
 * Return Value : none
 ******************************************************************************/
void gap_cb(uint16_t type, ble_status_t result, st_ble_evt_data_t *p_data)
{
/* Hint: Input common process of callback function such as variable definitions */
/* Start user code for GAP callback function common process. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

    switch(type)
    {
        case BLE_GAP_EVENT_STACK_ON:
        {
            /* Get BD address for Advertising */
            R_BLE_VS_GetBdAddr(BLE_VS_ADDR_AREA_REG, BLE_GAP_ADDR_RAND);
        } break;

        case BLE_GAP_EVENT_CONN_IND:
        {
            if (BLE_SUCCESS == result)
            {
                /* Store connection handle */
                st_ble_gap_conn_evt_t *p_gap_conn_evt_param = (st_ble_gap_conn_evt_t *)p_data->p_param;
                g_conn_hdl = p_gap_conn_evt_param->conn_hdl;
            }
            else
            {
                /* Restart advertising when connection failed */
                RM_BLE_ABS_StartLegacyAdvertising(&g_ble_abs0_ctrl, &g_ble_advertising_parameter);
            }
        } break;

        case BLE_GAP_EVENT_DISCONN_IND:
        {
            /* Restart advertising when disconnected */
            g_conn_hdl = BLE_GAP_INVALID_CONN_HDL;
            RM_BLE_ABS_StartLegacyAdvertising(&g_ble_abs0_ctrl, &g_ble_advertising_parameter);
        } break;

        case BLE_GAP_EVENT_CONN_PARAM_UPD_REQ:
        {
            /* Send connection update response with value received on connection update request */
            st_ble_gap_conn_upd_req_evt_t *p_conn_upd_req_evt_param = (st_ble_gap_conn_upd_req_evt_t *)p_data->p_param;

            st_ble_gap_conn_param_t conn_updt_param = {
                .conn_intv_min = p_conn_upd_req_evt_param->conn_intv_min,
                .conn_intv_max = p_conn_upd_req_evt_param->conn_intv_max,
                .conn_latency  = p_conn_upd_req_evt_param->conn_latency,
                .sup_to        = p_conn_upd_req_evt_param->sup_to,
            };

            R_BLE_GAP_UpdConn(p_conn_upd_req_evt_param->conn_hdl,
                              BLE_GAP_CONN_UPD_MODE_RSP,
                              BLE_GAP_CONN_UPD_ACCEPT,
                              &conn_updt_param);
        } break;

/* Hint: Add cases of GAP event macros defined as BLE_GAP_XXX */
/* Start user code for GAP callback function event process. Do not edit comment generated here */
        case BLE_GAP_EVENT_PAIRING_REQ:
        {
            /* Android sent an SMP Pairing Request. We reply to accept it (without this reply the
             * DA14531 never sends a Pairing Response and the central reports "No Response").
             *
             * We must FORCE LE Secure Connections here. The QE-generated config
             * (ra_gen/ble_thread.c) sets secure_connection_only = BLE_GAP_SC_BEST_EFFORT, which
             * this GTL stack packs as SC-bit = 0 -> the peripheral offers LEGACY-only pairing.
             * Legacy Just Works + "bonding" + zero key distribution is an inconsistent combo the
             * DA14531 aborts with SMP "Unspecified Reason". SC Just Works derives the LTK via
             * ECDH (no key transport needed) and matches what the phone requested.
             *
             * Overriding the pairing params here (instead of in ble_thread.c) survives project
             * regeneration, and it takes effect because r_ble_gtl_send_gapc_bond_cfm() reads
             * ble_pairing_param_local at reply time. */
            st_ble_gap_pairing_param_t pair_param = {
                .iocap         = BLE_GAP_IOCAP_NOINPUT_NOOUTPUT,   /* Just Works, no PIN */
                .mitm          = BLE_GAP_SEC_MITM_BEST_EFFORT,     /* no MITM required   */
                .bonding       = BLE_GAP_BONDING,
                .max_key_size  = 16,
                .min_key_size  = 16,
                .loc_key_dist  = 0,
                .rem_key_dist  = 0,
                .key_notf      = BLE_GAP_SC_KEY_PRESS_NTF_NOT_SPRT,
                .sec_conn_only = BLE_GAP_SC_STRICT,                /* <-- sets the SC bit */
            };
            (void)R_BLE_GAP_SetPairingParams(&pair_param);

            st_ble_gap_pairing_req_evt_t * p_pair_req = (st_ble_gap_pairing_req_evt_t *)p_data->p_param;
            R_BLE_GAP_ReplyPairing(p_pair_req->conn_hdl, BLE_GAP_PAIRING_ACCEPT);
        } break;
/* End user code. Do not edit comment generated here */
    }
}

/******************************************************************************
 * Function Name: gatts_cb
 * Description  : Callback function for GATT Server API.
 * Arguments    : uint16_t type -
 *                  Event type of GATT Server API.
 *              : ble_status_t result -
 *                  Event result of GATT Server API.
 *              : st_ble_gatts_evt_data_t *p_data -
 *                  Event parameters of GATT Server API.
 * Return Value : none
 ******************************************************************************/
void gatts_cb(uint16_t type, ble_status_t result, st_ble_gatts_evt_data_t *p_data)
{
/* Hint: Input common process of callback function such as variable definitions */
/* Start user code for GATT Server callback function common process. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

    R_BLE_SERVS_GattsCb(type, result, p_data);
    switch(type)
    {
/* Hint: Add cases of GATT Server event macros defined as BLE_GATTS_XXX */
/* Start user code for GATT Server callback function event process. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
    }
}

/******************************************************************************
 * Function Name: gattc_cb
 * Description  : Callback function for GATT Client API.
 * Arguments    : uint16_t type -
 *                  Event type of GATT Client API.
 *              : ble_status_t result -
 *                  Event result of GATT Client API.
 *              : st_ble_gattc_evt_data_t *p_data -
 *                  Event parameters of GATT Client API.
 * Return Value : none
 ******************************************************************************/
void gattc_cb(uint16_t type, ble_status_t result, st_ble_gattc_evt_data_t *p_data)
{
/* Hint: Input common process of callback function such as variable definitions */
/* Start user code for GATT Client callback function common process. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

    R_BLE_SERVC_GattcCb(type, result, p_data);
    switch(type)
    {

/* Hint: Add cases of GATT Client event macros defined as BLE_GATTC_XXX */
/* Start user code for GATT Client callback function event process. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
    }
}

/******************************************************************************
 * Function Name: vs_cb
 * Description  : Callback function for Vendor Specific API.
 * Arguments    : uint16_t type -
 *                  Event type of Vendor Specific API.
 *              : ble_status_t result -
 *                  Event result of Vendor Specific API.
 *              : st_ble_vs_evt_data_t *p_data -
 *                  Event parameters of Vendor Specific API.
 * Return Value : none
 ******************************************************************************/
void vs_cb(uint16_t type, ble_status_t result, st_ble_vs_evt_data_t *p_data)
{
/* Hint: Input common process of callback function such as variable definitions */
/* Start user code for vender specific callback function common process. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

    R_BLE_SERVS_VsCb(type, result, p_data);
    switch(type)
    {
        case BLE_VS_EVENT_GET_ADDR_COMP:
        {
            /* Start advertising when BD address is ready */
            st_ble_vs_get_bd_addr_comp_evt_t * get_address = (st_ble_vs_get_bd_addr_comp_evt_t *)p_data->p_param;
            memcpy(g_ble_advertising_parameter.own_bluetooth_address, get_address->addr.addr, BLE_BD_ADDR_LEN);
            RM_BLE_ABS_StartLegacyAdvertising(&g_ble_abs0_ctrl, &g_ble_advertising_parameter);
        } break;

/* Hint: Add cases of vender specific event macros defined as BLE_VS_XXX */
/* Start user code for vender specific callback function event process. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
    }
}

/******************************************************************************
 * Function Name: gat2s_cb
 * Description  : Callback function for GATT Service server feature.
 * Arguments    : uint16_t type -
 *                  Event type of GATT Service server feature.
 *              : ble_status_t result -
 *                  Event result of GATT Service server feature.
 *              : st_ble_servs_evt_data_t *p_data -
 *                  Event parameters of GATT Service server feature.
 * Return Value : none
 ******************************************************************************/
static void gat2s_cb(uint16_t type, ble_status_t result, st_ble_servs_evt_data_t *p_data)
{
/* Hint: Input common process of callback function such as variable definitions */
/* Start user code for GATT Service Server callback function common process. Do not edit comment generated here */
    (void) result;
    (void) p_data;
/* End user code. Do not edit comment generated here */

    switch(type)
    {
/* Hint: Add cases of GATT Service server events defined in e_ble_gat2s_event_t */
/* Start user code for GATT Service Server callback function event process. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
    }
}

/******************************************************************************
 * Function Name: SensorServices_cb
 * Description  : Callback function for Sensor Service server feature.
 * Arguments    : uint16_t type -
 *                  Event type of Sensor Service server feature.
 *              : ble_status_t result -
 *                  Event result of Sensor Service server feature.
 *              : st_ble_servs_evt_data_t *p_data -
 *                  Event parameters of Sensor Service server feature.
 * Return Value : none
 ******************************************************************************/
static void SensorServices_cb(uint16_t type, ble_status_t result, st_ble_servs_evt_data_t *p_data)
{
/* Hint: Input common process of callback function such as variable definitions */
/* Start user code for Sensor Service Server callback function common process. Do not edit comment generated here */
    (void) result;
    (void) p_data;
/* End user code. Do not edit comment generated here */

    switch(type)
    {
/* Hint: Add cases of Sensor Service server events defined in e_ble_SensorServices_event_t */
/* Start user code for Sensor Service Server callback function event process. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
    }
}

/******************************************************************************
 * Function Name: SensorRates_cb
 * Description  : Callback function for Sensor Write Service server feature.
 * Arguments    : uint16_t type -
 *                  Event type of Sensor Write Service server feature.
 *              : ble_status_t result -
 *                  Event result of Sensor Write Service server feature.
 *              : st_ble_servs_evt_data_t *p_data -
 *                  Event parameters of Sensor Write Service server feature.
 * Return Value : none
 ******************************************************************************/
static void SensorRates_cb(uint16_t type, ble_status_t result, st_ble_servs_evt_data_t *p_data)
{
/* Hint: Input common process of callback function such as variable definitions */
/* Start user code for Sensor Write Service Server callback function common process. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

    switch(type)
    {
/* Hint: Add cases of Sensor Write Service server events defined in e_ble_SensorRates_event_t */
/* Start user code for Sensor Write Service Server callback function event process. Do not edit comment generated here */
        case BLE_SENSORRATES_EVENT_SENSOR_ODR_WRITE_REQ:
        {
            /* 'result' carries decode_uint16_t's status. It FAILS (BLE_ERR_INVALID_DATA) unless the peer
             * wrote exactly 2 bytes, and on failure leaves the output buffer uninitialised - so acting on
             * p_param without checking here turns a wrong-length write into a bogus ODR of 0. */
            if ((BLE_SUCCESS == result) && (NULL != p_data) && (NULL != p_data->p_param))
            {
                /* Decoded little-endian ODR in Hz. Only recorded here - the ICM thread owns the I2C bus. */
                rs_sensor_write_request_cb(*(const uint16_t *) p_data->p_param);
            }
        } break;
/* End user code. Do not edit comment generated here */
    }
}
/******************************************************************************
 * Function Name: ble_init
 * Description  : Initialize BLE and profiles.
 * Arguments    : none
 * Return Value : BLE_SUCCESS - SUCCESS
 *                BLE_ERR_INVALID_OPERATION -
 *                    Failed to initialize BLE or profiles.
 ******************************************************************************/
ble_status_t ble_init(void)
{
    ble_status_t status;
    fsp_err_t err;

    /* Initialize BLE */
    err = RM_BLE_ABS_Open(&g_ble_abs0_ctrl, &g_ble_abs0_cfg);
    if (FSP_SUCCESS != err)
    {
        return BLE_ERR_INVALID_OPERATION;
    }

    /* Initialize GATT Database */
    status = R_BLE_GATTS_SetDbInst(NULL);
    if (BLE_SUCCESS != status)
    {
        return BLE_ERR_INVALID_OPERATION;
    }

    /* Initialize GATT server */
    status = R_BLE_SERVS_Init();
    if (BLE_SUCCESS != status)
    {
        return BLE_ERR_INVALID_OPERATION;
    }

    /*Initialize GATT client */
    status = R_BLE_SERVC_Init();
    if (BLE_SUCCESS != status)
    {
        return BLE_ERR_INVALID_OPERATION;
    }

    /* Set Prepare Write Queue */
    R_BLE_GATTS_SetPrepareQueue(gs_queue, BLE_GATTS_QUEUE_NUM);

    /* Initialize GATT Service server API */
    status = R_BLE_GAT2S_Init(gat2s_cb);
    if (BLE_SUCCESS != status)
    {
        return BLE_ERR_INVALID_OPERATION;
    }

    /* Initialize Sensor Service server API */
    status = R_BLE_SENSORSERVICES_Init(SensorServices_cb);
    if (BLE_SUCCESS != status)
    {
        return BLE_ERR_INVALID_OPERATION;
    }

    /* Initialize Sensor Write Service server API */
    status = R_BLE_SENSORRATES_Init(SensorRates_cb);
    if (BLE_SUCCESS != status)
    {
        return BLE_ERR_INVALID_OPERATION;
    }

    return status;
}

/******************************************************************************
 * Function Name: app_main
 * Description  : Application main function with main loop
 * Arguments    : none
 * Return Value : none
 ******************************************************************************/
void app_main(void)
{
    /* Create Event Group */
    g_ble_event_group_handle = xEventGroupCreate();
    assert(g_ble_event_group_handle);

    /* Initialize BLE and profiles */
    ble_status_t ble_status = ble_init();
    if (ble_status != BLE_SUCCESS)
    {
        MSG_PRINT("Error: BLE initialization failed with status %d.\n", ble_status);
        __BKPT(0);
    }

/* Hint: Input process that should be done before main loop such as calling initial function or variable definitions */
/* Start user code for process before main loop. Do not edit comment generated here */

/* End user code. Do not edit comment generated here */

    /* main loop */
    while (1)
    {
        /* Process BLE Event */
        R_BLE_Execute();

        EventBits_t bits = 0;
        if (0 != R_BLE_IsTaskFree())
        {
            /* If the BLE Task has no operation to be processed, it transits block state until the event from RF
             * transciever occurs or the ICM thread signals BLE_SENSOR_TX_EVT_BIT - whichever comes first. */
            bits = xEventGroupWaitBits(g_ble_event_group_handle,
                                (EventBits_t) (BLE_EVENT_PATTERN
#if ENABLE_SENSOR_EVT_TO_BLE
                                | BLE_SENSOR_TX_EVT_BIT),
#endif
                                pdTRUE, pdFALSE,
                                portMAX_DELAY);
        }

/* Hint: Input process that should be done during main loop such as calling processing functions */
/* Start user code for process during main loop. Do not edit comment generated here */
        /* Rate-limited/threshold-gated ICM sample -> Sensor Data Notify. Logic lives in
         * src/rs_ble_sensor/rs_ble_sensor_data_tx.c - see rs_ble_sensor_sample(). */
#if ENABLE_SENSOR_EVT_TO_BLE
        if ((bits & BLE_SENSOR_TX_EVT_BIT) != 0)
        {
            ble_status = rs_ble_sensor_sample(g_sensor_accel_sample);
            if (ble_status != BLE_SUCCESS)
            {
                /* This checks the result of the Sensor Data notify (R_BLE_SENSORSERVICES_NotifySensordata,
                 * called from rs_ble_sensor_sample()). Anything other than BLE_SUCCESS means the sample was NOT sent:
                 *   - BLE_ERR_UNSPECIFIED       : no BLE connection (g_conn_hdl is invalid) - expected, not a fault.
                 *   - BLE_ERR_INVALID_OPERATION : connected, but the phone has not enabled notifications on the
                 *                                 CCCD of the Sensor Data characteristic - expected, not a fault.
                 *   - any other status          : a real failure sending the notification.
                 * The print below is disabled because the first two cases are normal and would flood RTT while no
                 * phone is connected or subscribed. To see the status code, uncomment it (the value is a
                 * ble_status_t, see r_ble_api.h). To report only real failures, add a check such as
                 * (BLE_ERR_UNSPECIFIED != ble_status) && (BLE_ERR_INVALID_OPERATION != ble_status) around it. */

//                MSG_PRINT("Error: BLE Sensor Data Notify failed with status %d.\n", ble_status);
            }
        }
#else
        (void)bits;   /* suppress unused variable warning when the BLE_SENSOR_TX_BIT block is disabled */
#endif
/* End user code. Do not edit comment generated here */
    }

/* Hint: Input process that should be done after main loop such as calling closing functions */
/* Start user code for process after main loop. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

    /* Terminate BLE */
    RM_BLE_ABS_Close(&g_ble_abs0_ctrl);
}

/******************************************************************************
 User function definitions
*******************************************************************************/
/* Start user code for function definitions. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
