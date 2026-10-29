/***********************************************************************************************************************
 * Copyright [2020-2022] Renesas Electronics Corporation and/or its affiliates.  All Rights Reserved.
 *
 * This software and documentation are supplied by Renesas Electronics America Inc. and may only be used with products
 * of Renesas Electronics Corp. and its affiliates ("Renesas").  No other uses are authorized.  Renesas products are
 * sold pursuant to Renesas terms and conditions of sale.  Purchasers are solely responsible for the selection and use
 * of Renesas products and Renesas assumes no liability.  No license, express or implied, to any intellectual property
 * right is granted by Renesas. This software is protected under all applicable laws, including copyright laws. Renesas
 * reserves the right to change or discontinue this software and/or this documentation. THE SOFTWARE AND DOCUMENTATION
 * IS DELIVERED TO YOU "AS IS," AND RENESAS MAKES NO REPRESENTATIONS OR WARRANTIES, AND TO THE FULLEST EXTENT
 * PERMISSIBLE UNDER APPLICABLE LAW, DISCLAIMS ALL WARRANTIES, WHETHER EXPLICITLY OR IMPLICITLY, INCLUDING WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT, WITH RESPECT TO THE SOFTWARE OR
 * DOCUMENTATION.  RENESAS SHALL HAVE NO LIABILITY ARISING OUT OF ANY SECURITY VULNERABILITY OR BREACH.  TO THE MAXIMUM
 * EXTENT PERMITTED BY LAW, IN NO EVENT WILL RENESAS BE LIABLE TO YOU IN CONNECTION WITH THE SOFTWARE OR DOCUMENTATION
 * (OR ANY PERSON OR ENTITY CLAIMING RIGHTS DERIVED FROM YOU) FOR ANY LOSS, DAMAGES, OR CLAIMS WHATSOEVER, INCLUDING,
 * WITHOUT LIMITATION, ANY DIRECT, CONSEQUENTIAL, SPECIAL, INDIRECT, PUNITIVE, OR INCIDENTAL DAMAGES; ANY LOST PROFITS,
 * OTHER ECONOMIC DAMAGE, PROPERTY DAMAGE, OR PERSONAL INJURY; AND EVEN IF RENESAS HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH LOSS, DAMAGES, CLAIMS OR COSTS.
 **********************************************************************************************************************/


/*******************************************************************************************************************//**
 * @addtogroup RM_ICM42670
 * @{
 **********************************************************************************************************************/

#ifndef RM_ICM42670_H
#define RM_ICM42670_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "rm_icm42670_reg.h"
#include <rm_icm42670_api.h>
#include <rm_icm42670_cfg.h>



/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER

#define RM_ICM42670_SENSOR_G                     (9.8f)

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** ICM42670 initialization process block */
typedef struct st_rm_icm42670_init_process_params
{
    volatile bool              communication_finished; ///< Communication flag for blocking.
    volatile rm_icm42670_event_t event;                  ///< Callback event
} rm_icm42670_init_process_params_t;

/** ICM42670 mode extended configuration */
typedef struct st_rm_icm42670_mode_extended_cfg
{
    // Common
    rm_icm42670_api_t const    * p_api;                                  ///< Pointer to APIs.
    rm_icm42670_operation_mode_t mode_irq;                               ///< Operation mode using IRQ
    rm_icm42670_interrupt_source_t        interrupt_source;         ///<  Interrupt source.
    uint8_t      interrupt_config;     ///<  interrupt config of int1 and int2.
    //rm_icm42670_interrupt_int_clear_t     interrupt_clear;             ///<  INT_CONFIG0, Interrupt Clear Option (latched mode).
    rm_icm42670_pwr_rc_idle_t     pwr_rc_idle;             ///<  INT_CONFIG0, Interrupt Clear Option (latched mode).
    rm_icm42670_clksel_t     clksel;             ///<  SIGNAL_PATH_RESET, Software Reset (auto clear bit).

    // Accel mode cfg
    rm_icm42670_accel_sensor_mode_t      accel_sensor_mode;              ///< LP or LN sensor mode
    int16_t               accel_fs;                       ///< Full scale select for accelerometer UI interface output
    //rm_icm42670_accel_odr_t              accel_odr;                      ///< Accelerometer ODR selection for UI interface output
    uint16_t                               accel_odr;                      ///< Accelerometer ODR selection for UI interface output
    rm_icm42670_accel_lp_clk_sel_t       accel_lp_clk_sel;               ///< Accelerometer LP mode clk select
    rm_icm42670_accel_ui_filt_bw_t       accel_ui_filt_bw;               ///< Selects ACCEL UI low pass filter bandwidth
    rm_icm42670_accel_ui_avg_t    accel_ui_avg;    ///< Selects averaging filter setting to create accelerometer output in accelerometer low power mode (LPM)

    // Gyrocope mode cfg
    rm_icm42670_gyro_sensor_mode_t   gyro_sensor_mode;             ///< Gyroscope mode : standby or LN.
    int16_t gyro_fs;                                    ///< Full scale select for gyroscope UI interface output
    //rm_icm42670_accel_odr_t gyro_odr;                                    ///< Gyroscope ODR selection for UI interface output
    uint16_t              gyro_odr;                                    ///< Gyroscope ODR selection for UI interface output
    rm_icm42670_gyro_ui_filt_bw_t gyro_ui_filt_bw;        ///< Selects GYRO UI low pass filter bandwidth of ICM42670.

    //Temp config
    rm_icm42670_temp_filt_bw_t temp_filt_bw;       ///< Sets the bandwidth of the temperature signal DLPF.


    // APEX config


    //FIFO config

} rm_icm42670_mode_extended_cfg_t;

/** ICM42670 Control Block */
typedef struct rm_icm42670_instance_ctrl
{
    rm_icm42670_semaphore_t const * p_semaphore;                   ///< The semaphore to wait for callback. This is used for another data read/write after a communication
    uint32_t                             open;                     ///< Open flag
    rm_icm42670_cfg_t const              * p_cfg;                  ///< Pointer to ICM42670 Configuration
    uint8_t                              buf[8];                   ///< Buffer for I2C communications
    rm_icm42670_init_process_params_t    init_process_params;      ///< For the initialization process.
    uint8_t                              register_address;         ///< Register address to access
    volatile rm_icm42670_device_status_t * p_device_status;        ///< Pointer to device status.
    volatile rm_icm42670_fifo_info_t     * p_fifo_info;            ///< Pointer to FIFO information structure.
    volatile bool                        fifo_reset;               ///< Flag for FIFO reset for FIFO mode
    volatile bool                        interrupt_bits_clear;     ///< Flag for clearing interrupt bits.
    uint16_t                             accel_sensitivity_shift;  ///< Accel Sensitivity Scale Factor Shift.
    uint16_t                             gyro_sensitivity_x10;     ///< // Gyro Sensitivity Scale Factor * 10.
    rm_comms_instance_t const          * p_comms_i2c_instance;     ///< Pointer of I2C Communications Middleware instance structure
    rm_icm42670_mode_extended_cfg_t    * p_mode;                   ///< Pointer of ICM42670 operation mode extended configuration
    void const                         * p_irq_instance;           ///< Pointer to IRQ instance.
    void const                         * p_context;                ///< Pointer to the user-provided context

    /* Pointer to callback and optional working memory */
    void (* p_comms_callback)(rm_icm42670_callback_args_t * p_args); ///< I2C Communications callback
    void (* p_irq_callback)(rm_icm42670_callback_args_t * p_args);   ///< IRQ callback
} rm_icm42670_instance_ctrl_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
extern volatile bool g_i2c_flag;
extern volatile uint32_t g_sensor_open;

/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern rm_icm42670_api_t const g_icm42670_on_icm42670;


/** @endcond */

/**********************************************************************************************************************
 * Public Function Prototypes
 **********************************************************************************************************************/
fsp_err_t RM_ICM42670_Open(rm_icm42670_ctrl_t * const p_api_ctrl, rm_icm42670_cfg_t const * const p_cfg);
fsp_err_t RM_ICM42670_Close(rm_icm42670_ctrl_t * const p_api_ctrl);
fsp_err_t RM_ICM42670_MeasurementStart(rm_icm42670_ctrl_t * const p_api_ctrl);
fsp_err_t RM_ICM42670_MeasurementStop(rm_icm42670_ctrl_t * const p_api_ctrl);
fsp_err_t RM_ICM42670_AccelRead(rm_icm42670_ctrl_t * const     p_api_ctrl,
                              rm_icm42670_raw_data_t * const p_raw_data);
fsp_err_t RM_ICM42670_AccelDataCalculate(rm_icm42670_ctrl_t * const       p_api_ctrl,
                                       rm_icm42670_raw_data_t * const   p_raw_data,
                                       rm_icm42670_accel_data_t * const p_icm42670_data);
fsp_err_t RM_ICM42670_GyroRead(rm_icm42670_ctrl_t * const p_api_ctrl, rm_icm42670_raw_data_t * const p_raw_data);
fsp_err_t RM_ICM42670_GyroDataCalculate(rm_icm42670_ctrl_t * const      p_api_ctrl,
                                      rm_icm42670_raw_data_t * const  p_raw_data,
                                      rm_icm42670_gyro_data_t * const p_icm42670_data);
fsp_err_t RM_ICM42670_TempRead(rm_icm42670_ctrl_t * const     p_api_ctrl,
                            rm_icm42670_raw_data_t * const p_raw_data);
fsp_err_t RM_ICM42670_TempDataCalculate(rm_icm42670_ctrl_t * const     p_api_ctrl,
                                     rm_icm42670_raw_data_t * const p_raw_data,
                                     rm_icm42670_temp_data_t * const p_icm42670_data);
fsp_err_t RM_ICM42670_DeviceInterruptCfgSet(rm_icm42670_ctrl_t * const               p_api_ctrl,
                                          rm_icm42670_device_interrupt_cfg_t const interrupt_cfg);
fsp_err_t RM_ICM42670_FifoInfoGet(rm_icm42670_ctrl_t * const p_api_ctrl, rm_icm42670_fifo_info_t * const p_fifo_info);
fsp_err_t RM_ICM42670_DeviceStatusGet(rm_icm42670_ctrl_t * const p_api_ctrl, rm_icm42670_device_status_t * const p_status);
fsp_err_t rm_icm42670_device_status_check (rm_icm42670_ctrl_t * const p_api_ctrl);

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_FOOTER

#endif                                 /* RM_ICM42670_H_*/

/*******************************************************************************************************************//**
 * @} (end addtogroup RM_ICM42670)
 **********************************************************************************************************************/
