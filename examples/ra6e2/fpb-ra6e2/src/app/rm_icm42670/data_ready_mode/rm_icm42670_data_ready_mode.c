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

/**********************************************************************************************************************
 * Includes   <System Includes> , "Project Includes"
 *********************************************************************************************************************/

#include <stdio.h>

#define RM_ICM42670_ACCEL_GYRO_MODE_ENABLE    (1)

#if RM_ICM42670_ACCEL_GYRO_MODE_ENABLE
 #include "rm_icm42670.h"

/**********************************************************************************************************************
 * Macro definitions
 *********************************************************************************************************************/

/* Definitions of Command */
 #define RM_ICM42670_COMMAND_MEASUREMENT_START    (0x01)
 #define RM_ICM42670_COMMAND_INTERRUPT_ENABLE     (0x01)

/* Definitions of Mask */
 #define RM_ICM42670_MASK_PERSIST                 (0x0F)
 #define RM_ICM42670_MASK_DATA                    (0x0F)
 #define RM_ICM42670_MASK_ACCEL_THRESHOLD         (0x000FFFFF)
 #define RM_ICM42670_MASK_LED_CURRENT             (0x03FF)
 #define RM_ICM42670_MASK_8BITS                   (0xFF)
 #define RM_ICM42670_MASK_HYSTERESIS              (0x7F)

/* Definitions of Calculation */
 #define RM_ICM42670_MAX_NUM_ACCEL_SOURCE         (5)
 #define RM_ICM42670_SENSOR_PI                    (3.14f)
/**********************************************************************************************************************
 * Local Typedef definitions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported global functions
 *********************************************************************************************************************/
extern fsp_err_t rm_icm42670_read(rm_icm42670_ctrl_t * const p_api_ctrl, rm_comms_write_read_params_t write_read_params);
extern fsp_err_t rm_icm42670_write(rm_icm42670_ctrl_t * const p_api_ctrl, uint8_t * const p_src, uint8_t const bytes);
extern fsp_err_t rm_icm42670_int_cfg_register_write(rm_icm42670_ctrl_t * const p_api_ctrl);
extern fsp_err_t rm_icm42670_ppg_ps_gain_register_write(rm_icm42670_ctrl_t * const p_api_ctrl, uint8_t const ppg_ps_gain);
extern fsp_err_t rm_icm42670_all_interrupt_bits_clear(rm_icm42670_ctrl_t * const p_api_ctrl);

extern fsp_err_t rm_icm42670_os_semaphore_acquire(rm_icm42670_semaphore_t const * p_semaphore, uint32_t const timeout);


/**********************************************************************************************************************
 * Private (static) variables and functions
 *********************************************************************************************************************/
static fsp_err_t rm_icm42670_data_ready_open(rm_icm42670_ctrl_t * const p_api_ctrl, rm_icm42670_cfg_t const * const p_cfg);
static fsp_err_t rm_icm42670_data_ready_close(rm_icm42670_ctrl_t * const p_api_ctrl);
static fsp_err_t rm_icm42670_data_ready_measurement_start(rm_icm42670_ctrl_t * const p_api_ctrl);
static fsp_err_t rm_icm42670_data_ready_measurement_stop(rm_icm42670_ctrl_t * const p_api_ctrl);
static fsp_err_t rm_icm42670_data_ready_accel_read(rm_icm42670_ctrl_t * const     p_api_ctrl,
                                                 rm_icm42670_raw_data_t * const p_raw_data);
static fsp_err_t rm_icm42670_data_ready_accel_data_calculate(rm_icm42670_ctrl_t * const       p_api_ctrl,
                                                           rm_icm42670_raw_data_t * const   p_raw_data,
                                                           rm_icm42670_accel_data_t * const p_icm42670_data);
static fsp_err_t rm_icm42670_data_ready_gyro_read(rm_icm42670_ctrl_t * const     p_api_ctrl,
                                                rm_icm42670_raw_data_t * const p_raw_data);
static fsp_err_t rm_icm42670_data_ready_gyro_data_calculate(rm_icm42670_ctrl_t * const      p_api_ctrl,
                                                          rm_icm42670_raw_data_t * const  p_raw_data,
                                                          rm_icm42670_gyro_data_t * const p_icm42670_data);
static fsp_err_t rm_icm42670_data_ready_temp_read(rm_icm42670_ctrl_t * const     p_api_ctrl,
                                               rm_icm42670_raw_data_t * const p_raw_data);
static fsp_err_t rm_icm42670_data_ready_temp_data_calculate(rm_icm42670_ctrl_t * const     p_api_ctrl,
                                                         rm_icm42670_raw_data_t * const p_raw_data,
                                                         rm_icm42670_temp_data_t * const p_icm42670_data);
static fsp_err_t rm_icm42670_data_ready_device_interrupt_cfg_set(rm_icm42670_ctrl_t * const               p_api_ctrl,
                                                               rm_icm42670_device_interrupt_cfg_t const interrupt_cfg);
static fsp_err_t rm_icm42670_data_ready_fifo_info_get(rm_icm42670_ctrl_t * const      p_api_ctrl,
                                                    rm_icm42670_fifo_info_t * const p_fifo_info);
static fsp_err_t rm_icm42670_data_ready_device_status_get(rm_icm42670_ctrl_t * const          p_api_ctrl,
                                                        rm_icm42670_device_status_t * const p_status);

static fsp_err_t rm_icm42670_accel_configuration (rm_icm42670_ctrl_t * const p_api_ctrl);
static fsp_err_t rm_icm42670_gyro_configuration (rm_icm42670_ctrl_t * const p_api_ctrl);
//static fsp_err_t rm_icm42670_pwr_mgmt_configuration(rm_icm42670_ctrl_t * const p_api_ctrl);

/**********************************************************************************************************************
 * Exported global variables
 *********************************************************************************************************************/
rm_icm42670_api_t const g_icm42670_on_icm42670_accel_gyro =
{
    .open                  = rm_icm42670_data_ready_open,
    .close                 = rm_icm42670_data_ready_close,
    .measurementStart      = rm_icm42670_data_ready_measurement_start,
    .measurementStop       = rm_icm42670_data_ready_measurement_stop,
    .accelRead             = rm_icm42670_data_ready_accel_read,
    .accelDataCalculate    = rm_icm42670_data_ready_accel_data_calculate,
    .gyroRead              = rm_icm42670_data_ready_gyro_read,
    .gyroDataCalculate     = rm_icm42670_data_ready_gyro_data_calculate,
    .tempRead              = rm_icm42670_data_ready_temp_read,
    .tempDataCalculate     = rm_icm42670_data_ready_temp_data_calculate,
    .deviceInterruptCfgSet = rm_icm42670_data_ready_device_interrupt_cfg_set,
    .fifoInfoGet           = rm_icm42670_data_ready_fifo_info_get,
    .deviceStatusGet       = rm_icm42670_data_ready_device_status_get,
};

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Configures the ICM42670 Gyroscope mode.
 *
 * @retval FSP_SUCCESS              ICM42670 successfully configured.
 * @retval FSP_ERR_TIMEOUT          Communication is timeout.
 * @retval FSP_ERR_ABORTED          Communication is aborted.
 **********************************************************************************************************************/
static fsp_err_t rm_icm42670_data_ready_open (rm_icm42670_ctrl_t * const p_api_ctrl, rm_icm42670_cfg_t const * const p_cfg)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;

    FSP_PARAMETER_NOT_USED(p_cfg);

    /* Accel configuration */
    err = rm_icm42670_accel_configuration(p_ctrl);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    /* Gyrocope configuration */
    err = rm_icm42670_gyro_configuration(p_ctrl);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    //err = rm_icm42670_accel_gyro_device_interrupt_cfg_set(p_ctrl, p_cfg);
    //FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Unsupported API.
 *
 * @retval FSP_ERR_UNSUPPORTED                    Operation mode is not supported.
 **********************************************************************************************************************/
static fsp_err_t rm_icm42670_data_ready_close (rm_icm42670_ctrl_t * const p_api_ctrl)
{
    FSP_PARAMETER_NOT_USED(p_api_ctrl);

    return FSP_ERR_UNSUPPORTED;
}

/*******************************************************************************************************************//**
 * @brief  Start periodic measurements in both Accel mode and Gyroscope mode
 *
 * @retval FSP_SUCCESS              ICM42670 successfully configured.
 * @retval FSP_ERR_TIMEOUT          Communication is timeout.
 * @retval FSP_ERR_ABORTED          Communication is aborted.
 **********************************************************************************************************************/
static fsp_err_t rm_icm42670_data_ready_measurement_start (rm_icm42670_ctrl_t * const p_api_ctrl)
{
    fsp_err_t err = FSP_SUCCESS;

    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;
    uint8_t pwr_cfg_tmp;

    /* PWR_MGMT configuration */
    pwr_cfg_tmp = (uint8_t) (p_ctrl->p_mode->accel_sensor_mode | (p_ctrl->p_mode->gyro_sensor_mode<<2) |
                        (p_ctrl->p_mode->pwr_rc_idle<<4) | (p_ctrl->p_mode->accel_lp_clk_sel<<7));

    /* Set pwr mgmt */
    p_ctrl->buf[0] = RM_ICM42670_REG_PWR_MGMT0;
    p_ctrl->buf[1] = pwr_cfg_tmp;
    err            = rm_icm42670_write(p_ctrl, &p_ctrl->buf[0], 2);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Stop a periodic measurement
 *
 * @retval FSP_SUCCESS              ICM42670 successfully configured.
 * @retval FSP_ERR_TIMEOUT          Communication is timeout.
 * @retval FSP_ERR_ABORTED          Communication is aborted.
 **********************************************************************************************************************/
static fsp_err_t rm_icm42670_data_ready_measurement_stop (rm_icm42670_ctrl_t * const p_api_ctrl)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;

    uint8_t pwr_cfg_tmp;

    /* PWR_MGMT configuration */
    pwr_cfg_tmp = (RM_ICM42670_ACCEL_SENSOR_MODE_OFF | (RM_ICM42670_GYRO_SENSOR_MODE_STB<<2) |
                  (RM_ICM42670_PWR_RC_ON<<4) | (RM_ICM42670_ACCEL_CLK_SEL_WAKEUP_OSC<<7));

    /* Set power management */
    p_ctrl->buf[0] = RM_ICM42670_REG_PWR_MGMT0;
    p_ctrl->buf[1] = pwr_cfg_tmp;
    err            = rm_icm42670_write(p_ctrl, &p_ctrl->buf[0], 2);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Reads accel data from ICM42670 device. If device interrupt is enabled, interrupt bits are cleared after data read.
 *
 * @retval FSP_SUCCESS              Successfully results are read.
 * @retval FSP_ERR_TIMEOUT          Communication is timeout.
 * @retval FSP_ERR_ABORTED          Communication is aborted.
 **********************************************************************************************************************/
static fsp_err_t rm_icm42670_data_ready_accel_read (rm_icm42670_ctrl_t * const     p_api_ctrl,
                                                  rm_icm42670_raw_data_t * const p_raw_data)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;

    uint8_t bytes = RM_ICM42670_ACCEL_DATA_SIZE;
    rm_comms_write_read_params_t write_read_params;


    if (RM_ICM42670_OPERATION_MODE_6AXIS == p_ctrl->p_mode->mode_irq)
    {
        /* Clear interrupt bits after data read */
        p_ctrl->interrupt_bits_clear = true;
    }

    /* Read accel data. */
    p_ctrl->register_address     = RM_ICM42670_REG_ACCEL_DATA_X1;
    write_read_params.p_src      = &p_ctrl->register_address;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = (uint8_t *) p_raw_data;
    write_read_params.dest_bytes = bytes;
    err = rm_icm42670_read(p_ctrl, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    if (RM_ICM42670_OPERATION_MODE_ACCEL_LN == p_ctrl->p_mode->mode_irq)
    {
        /* Acquire a semaphore */
        err = rm_icm42670_os_semaphore_acquire(p_ctrl->p_semaphore, p_ctrl->p_cfg->semaphore_timeout);
        FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

        /* Clear all interrupt bits */
        err = rm_icm42670_all_interrupt_bits_clear(p_ctrl);
        FSP_ERROR_RETURN(FSP_SUCCESS == err, err);
    }

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Calculate accel data from raw data.
 *
 * @retval FSP_SUCCESS              Successfully results are read.
 **********************************************************************************************************************/
static fsp_err_t rm_icm42670_data_ready_accel_data_calculate (rm_icm42670_ctrl_t * const       p_api_ctrl,
                                                            rm_icm42670_raw_data_t * const   p_raw_data,
                                                            rm_icm42670_accel_data_t * const p_icm42670_data)
{
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;
    int16_t temp_x = 0;
    int16_t temp_y = 0;
    int16_t temp_z = 0;
    FSP_PARAMETER_NOT_USED(p_api_ctrl);

    /* Get Accel data */
    temp_x = (int16_t)((uint16_t)(p_raw_data->reg_data[0]<<8) + p_raw_data->reg_data[1]);
    temp_y = (int16_t)((uint16_t)(p_raw_data->reg_data[2]<<8) + p_raw_data->reg_data[3]);
    temp_z = (int16_t)((uint16_t)(p_raw_data->reg_data[4]<<8) + p_raw_data->reg_data[5]);

    /* Calculate Accel data */
    p_icm42670_data->accel_x = (float)((temp_x * RM_ICM42670_SENSOR_G)/p_ctrl->accel_sensitivity_shift);
    p_icm42670_data->accel_y = (float)((temp_y * RM_ICM42670_SENSOR_G)/p_ctrl->accel_sensitivity_shift);
    p_icm42670_data->accel_z = (float)((temp_z * RM_ICM42670_SENSOR_G)/p_ctrl->accel_sensitivity_shift);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Reads Gyroscope data from ICM42670 device. If device interrupt is enabled, interrupt bits are cleared after data read.
 *
 * @retval FSP_SUCCESS              Successfully results are read.
 * @retval FSP_ERR_TIMEOUT          Communication is timeout.
 * @retval FSP_ERR_ABORTED          Communication is aborted.
 **********************************************************************************************************************/
static fsp_err_t rm_icm42670_data_ready_gyro_read (rm_icm42670_ctrl_t * const     p_api_ctrl,
                                                 rm_icm42670_raw_data_t * const p_raw_data)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;
    uint8_t bytes = RM_ICM42670_GYRO_DATA_SIZE;
    rm_comms_write_read_params_t write_read_params;


    if (RM_ICM42670_OPERATION_MODE_6AXIS == p_ctrl->p_mode->mode_irq)
    {
        /* Clear interrupt bits after data read */
        p_ctrl->interrupt_bits_clear = true;
    }

    /* Read gyro data. */
    p_ctrl->register_address     = RM_ICM42670_REG_GYRO_DATA_X1;
    write_read_params.p_src      = &p_ctrl->register_address;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = (uint8_t *) p_raw_data;
    write_read_params.dest_bytes = bytes;
    err = rm_icm42670_read(p_ctrl, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    if (RM_ICM42670_OPERATION_MODE_GYRO_LN == p_ctrl->p_mode->mode_irq)
    {
        /* Acquire a semaphore */
        err = rm_icm42670_os_semaphore_acquire(p_ctrl->p_semaphore, p_ctrl->p_cfg->semaphore_timeout);
        FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

        /* Clear all interrupt bits */
        err = rm_icm42670_all_interrupt_bits_clear(p_ctrl);
        FSP_ERROR_RETURN(FSP_SUCCESS == err, err);
    }

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Calculate Gyroscope data from raw data.
 *
 * @retval FSP_SUCCESS              Successfully results are read.
 **********************************************************************************************************************/
static fsp_err_t rm_icm42670_data_ready_gyro_data_calculate (rm_icm42670_ctrl_t * const      p_api_ctrl,
                                                           rm_icm42670_raw_data_t * const  p_raw_data,
                                                           rm_icm42670_gyro_data_t * const p_icm42670_data)
{
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;
    int16_t temp_x = 0;
    int16_t temp_y = 0;
    int16_t temp_z = 0;
    FSP_PARAMETER_NOT_USED(p_api_ctrl);

     /* Get Gyroscope data */
    temp_x = (int16_t)((uint16_t)(p_raw_data->reg_data[0]<<8) + p_raw_data->reg_data[1]);
    temp_y = (int16_t)((uint16_t)(p_raw_data->reg_data[2]<<8) + p_raw_data->reg_data[3]);
    temp_z = (int16_t)((uint16_t)(p_raw_data->reg_data[4]<<8) + p_raw_data->reg_data[5]);

    /* Calculate Gyroscope data */
    p_icm42670_data->gyro_x = (float)((temp_x * RM_ICM42670_SENSOR_PI * 10) / (p_ctrl->gyro_sensitivity_x10 * 180LL));
    p_icm42670_data->gyro_y = (float)((temp_y * RM_ICM42670_SENSOR_PI * 10) / (p_ctrl->gyro_sensitivity_x10 * 180LL));
    p_icm42670_data->gyro_z = (float)((temp_z * RM_ICM42670_SENSOR_PI * 10) / (p_ctrl->gyro_sensitivity_x10 * 180LL));

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Reads Temperature data from ICM42670 device. If device interrupt is enabled, interrupt bits are cleared after data read.
 *
 * @retval FSP_SUCCESS              Successfully results are read.
 * @retval FSP_ERR_TIMEOUT          Communication is timeout.
 * @retval FSP_ERR_ABORTED          Communication is aborted.
 **********************************************************************************************************************/
static fsp_err_t rm_icm42670_data_ready_temp_read (rm_icm42670_ctrl_t * const     p_api_ctrl,
                                                rm_icm42670_raw_data_t * const p_raw_data)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;

    uint8_t bytes = RM_ICM42670_TEMP_DATA_SIZE;
    rm_comms_write_read_params_t write_read_params;


    if (RM_ICM42670_OPERATION_MODE_6AXIS == p_ctrl->p_mode->mode_irq)
    {
        /* Clear interrupt bits after data read */
        p_ctrl->interrupt_bits_clear = true;
    }

    /* Read accel data. */
    p_ctrl->register_address     = RM_ICM42670_REG_TEMP_DATA1;
    write_read_params.p_src      = &p_ctrl->register_address;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = (uint8_t *) p_raw_data;
    write_read_params.dest_bytes = bytes;
    err = rm_icm42670_read(p_ctrl, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    if (RM_ICM42670_OPERATION_MODE_ACCEL_LN == p_ctrl->p_mode->mode_irq)
    {
        /* Acquire a semaphore */
        err = rm_icm42670_os_semaphore_acquire(p_ctrl->p_semaphore, p_ctrl->p_cfg->semaphore_timeout);
        FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

        /* Clear all interrupt bits */
        err = rm_icm42670_all_interrupt_bits_clear(p_ctrl);
        FSP_ERROR_RETURN(FSP_SUCCESS == err, err);
    }

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Calculate Temperature data from raw data.
 *
 * @retval FSP_SUCCESS              Successfully results are read.
 **********************************************************************************************************************/
static fsp_err_t rm_icm42670_data_ready_temp_data_calculate (rm_icm42670_ctrl_t * const     p_api_ctrl,
                                                          rm_icm42670_raw_data_t * const p_raw_data,
                                                          rm_icm42670_temp_data_t * const p_icm42670_data)
{

    FSP_PARAMETER_NOT_USED(p_api_ctrl);

    /* Calculate Temperature data */
    p_icm42670_data->temp_data       = (int16_t)((uint16_t)(p_raw_data->reg_data[0]<<8) + p_raw_data->reg_data[1]);
    p_icm42670_data->temp_data_float = (float)(p_icm42670_data->temp_data)/128.0f + 25.0f;
    p_icm42670_data->temp_data = p_icm42670_data->temp_data/128 + 25;

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Set device interrupt configurations.
 *
 * @retval FSP_ERR_TIMEOUT          Communication is timeout.
 * @retval FSP_ERR_ABORTED          Communication is aborted.
 * @retval FSP_ERR_INVALID_MODE     Invalid mode.
 **********************************************************************************************************************/
static fsp_err_t rm_icm42670_data_ready_device_interrupt_cfg_set (rm_icm42670_ctrl_t * const               p_api_ctrl,
                                                                rm_icm42670_device_interrupt_cfg_t const interrupt_cfg)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;

 #if RM_ICM42670_CFG_PARAM_CHECKING_ENABLE
    {
        bool err_res = ((RM_ICM42670_OPERATION_INTERRUPT_TYPE_INT1 == interrupt_cfg.int_type) ||
                        (RM_ICM42670_OPERATION_INTERRUPT_TYPE_INT2 == interrupt_cfg.int_type) ||
                        (RM_ICM42670_OPERATION_INTERRUPT_TYPE_INT1_INT2 == interrupt_cfg.int_type));
        FSP_ERROR_RETURN(err_res, FSP_ERR_INVALID_MODE);
    }
 #endif


    /* Set device interrupt configurations of int1 */
    if (RM_ICM42670_OPERATION_INTERRUPT_TYPE_INT1 == interrupt_cfg.int_type)
    {
        p_ctrl->p_mode->mode_irq = RM_ICM42670_OPERATION_MODE_6AXIS;
        p_ctrl->p_mode->interrupt_source = interrupt_cfg.int_source;
        p_ctrl->p_mode->interrupt_config = interrupt_cfg.int_config & ( RM_ICM42670_INTERRUPT_CONFIG_MASK_INT1 | 0x01 );
    }
    /* Set device interrupt configurations of int2 */
    if (RM_ICM42670_OPERATION_INTERRUPT_TYPE_INT2 == interrupt_cfg.int_type)
    {
        p_ctrl->p_mode->mode_irq = RM_ICM42670_OPERATION_MODE_6AXIS;
        p_ctrl->p_mode->interrupt_source = interrupt_cfg.int_source;
        p_ctrl->p_mode->interrupt_config = interrupt_cfg.int_config & RM_ICM42670_INTERRUPT_CONFIG_MASK_INT2;
    }
    else
    {
        p_ctrl->p_mode->mode_irq = RM_ICM42670_OPERATION_MODE_6AXIS;
        p_ctrl->p_mode->interrupt_source = interrupt_cfg.int_source;
        p_ctrl->p_mode->interrupt_config = interrupt_cfg.int_config & RM_ICM42670_INTERRUPT_CONFIG_MASK_INT1_INT2;
    }

    err = rm_icm42670_int_cfg_register_write(p_ctrl);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Unsupported API.
 *
 * @retval FSP_ERR_UNSUPPORTED                    Operation mode is not supported.
 **********************************************************************************************************************/
static fsp_err_t rm_icm42670_data_ready_fifo_info_get (rm_icm42670_ctrl_t * const      p_api_ctrl,
                                                     rm_icm42670_fifo_info_t * const p_fifo_info)
{
    FSP_PARAMETER_NOT_USED(p_api_ctrl);
    FSP_PARAMETER_NOT_USED(p_fifo_info);

    return FSP_ERR_UNSUPPORTED;
}

/*******************************************************************************************************************//**
 * @brief  Unsupported API.
 *
 * @retval FSP_ERR_UNSUPPORTED                    Operation mode is not supported.
 **********************************************************************************************************************/
static fsp_err_t rm_icm42670_data_ready_device_status_get (rm_icm42670_ctrl_t * const          p_api_ctrl,
                                                         rm_icm42670_device_status_t * const p_status)
{
    FSP_PARAMETER_NOT_USED(p_api_ctrl);
    FSP_PARAMETER_NOT_USED(p_status);

    return FSP_ERR_UNSUPPORTED;
}
#if 0
/*******************************************************************************************************************//**
 * @brief Configures the ICM42670 accel mode.
 *
 * @retval FSP_SUCCESS              ICM42670 successfully configured.
 * @retval FSP_ERR_TIMEOUT          Communication is timeout.
 * @retval FSP_ERR_ABORTED          Communication is aborted.
 **********************************************************************************************************************/
static fsp_err_t rm_icm42670_pwr_mgmt_configuration (rm_icm42670_ctrl_t * const p_api_ctrl)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;
    uint8_t pwr_cfg_tmp;

    #if RM_ICM42670_CFG_PARAM_CHECKING_ENABLE
    {
        bool err_res = !((p_ctrl->p_mode->accel_sensor_mode > 3) ||
                        (p_ctrl->p_mode->gyro_sensor_mode > 3) ||
                        (p_ctrl->p_mode->pwr_rc_idle > 1) ||
                        (p_ctrl->p_mode->accel_lp_clk_sel > 1));
        FSP_ERROR_RETURN(err_res, FSP_ERR_INVALID_MODE);
    }
    #endif

    pwr_cfg_tmp = (p_ctrl->p_mode->accel_sensor_mode | (p_ctrl->p_mode->gyro_sensor_mode<<2) |
                  (p_ctrl->p_mode->pwr_rc_idle<<4) | (p_ctrl->p_mode->accel_lp_clk_sel<<7));

    /* Set pwr mgmt */
    p_ctrl->buf[0] = RM_ICM42670_REG_PWR_MGMT0;
    p_ctrl->buf[1] = pwr_cfg_tmp;
    err            = rm_icm42670_write(p_ctrl, &p_ctrl->buf[0], 2);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}
#endif

/*******************************************************************************************************************//**
 * @brief Configure fs and odr of the ICM42670 Acceleremoter mode.
 *
 * @retval FSP_SUCCESS              ICM42670 successfully configured.
 * @retval FSP_ERR_TIMEOUT          Communication is timeout.
 * @retval FSP_ERR_ABORTED          Communication is aborted.
 **********************************************************************************************************************/
static fsp_err_t rm_icm42670_accel_configuration (rm_icm42670_ctrl_t * const p_api_ctrl)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;
    uint8_t accel_cfg_tmp;

    #if RM_ICM42670_CFG_PARAM_CHECKING_ENABLE
    {
        bool err_res = !((p_ctrl->p_mode->accel_fs > 16) || (p_ctrl->p_mode->accel_fs < -16));
        FSP_ERROR_RETURN(err_res, FSP_ERR_INVALID_MODE);
    }
    #endif

    if ((p_ctrl->p_mode->accel_fs > 8) || (p_ctrl->p_mode->accel_fs < -8))
    {
    	accel_cfg_tmp = RM_ICM42670_ACCEL_FS_16G << 5;
        p_ctrl->accel_sensitivity_shift = RM_ICM42670_ACCEL_SENSITIVITY_SCALE_FACTOR_0;
    }
    else if ((p_ctrl->p_mode->accel_fs > 4) || (p_ctrl->p_mode->accel_fs < -4))
    {
    	accel_cfg_tmp = RM_ICM42670_ACCEL_FS_8G << 5;
        p_ctrl->accel_sensitivity_shift = RM_ICM42670_ACCEL_SENSITIVITY_SCALE_FACTOR_1;
    }
    else if ((p_ctrl->p_mode->accel_fs > 2) || (p_ctrl->p_mode->accel_fs < -2))
    {
    	accel_cfg_tmp = RM_ICM42670_ACCEL_FS_4G << 5;
        p_ctrl->accel_sensitivity_shift = RM_ICM42670_ACCEL_SENSITIVITY_SCALE_FACTOR_2;
    }
    else
    {
    	accel_cfg_tmp = RM_ICM42670_ACCEL_FS_2G << 5;
        p_ctrl->accel_sensitivity_shift = RM_ICM42670_ACCEL_SENSITIVITY_SCALE_FACTOR_3;
    }


    if (RM_ICM42670_ACCEL_SENSOR_MODE_LN == p_ctrl->p_mode->accel_sensor_mode)
    {
        #if RM_ICM42670_CFG_PARAM_CHECKING_ENABLE
        {
            bool err_res = !((p_ctrl->p_mode->accel_odr > 1600)  || (p_ctrl->p_mode->accel_odr < 12));
            FSP_ERROR_RETURN(err_res, FSP_ERR_INVALID_MODE);
        }
        #endif

        if (p_ctrl->p_mode->accel_odr > 800)
        {
	    	accel_cfg_tmp |= RM_ICM42670_ACCEL_ODR_1600Hz;
	    }
        else if (p_ctrl->p_mode->accel_odr > 400)
        {
	    	accel_cfg_tmp |= RM_ICM42670_ACCEL_ODR_800Hz;
	    }
        else if (p_ctrl->p_mode->accel_odr > 200)
        {
	    	accel_cfg_tmp |= RM_ICM42670_ACCEL_ODR_400Hz;
	    }
        else if (p_ctrl->p_mode->accel_odr > 100)
        {
	    	accel_cfg_tmp |= RM_ICM42670_ACCEL_ODR_200Hz;
	    }
        else if (p_ctrl->p_mode->accel_odr > 50)
        {
	    	accel_cfg_tmp |= RM_ICM42670_ACCEL_ODR_100Hz;
	    }
        else if (p_ctrl->p_mode->accel_odr > 25)
        {
	    	accel_cfg_tmp |= RM_ICM42670_ACCEL_ODR_50Hz;
	    }
        else if (p_ctrl->p_mode->accel_odr > 12)
        {
	    	accel_cfg_tmp |= RM_ICM42670_ACCEL_ODR_25Hz;
	    }
        else
        {
	    	accel_cfg_tmp |= RM_ICM42670_ACCEL_ODR_12Hz;
	    }
    }
    else if (RM_ICM42670_ACCEL_SENSOR_MODE_LP == p_ctrl->p_mode->accel_sensor_mode)
    {
        #if RM_ICM42670_CFG_PARAM_CHECKING_ENABLE
        {
            bool err_res = !((p_ctrl->p_mode->accel_odr > 400)  || (p_ctrl->p_mode->accel_odr < 1));
            FSP_ERROR_RETURN(err_res, FSP_ERR_INVALID_MODE);
        }
        #endif

        if (p_ctrl->p_mode->accel_odr > 200)
        {
	    	accel_cfg_tmp |= RM_ICM42670_ACCEL_ODR_400Hz;
	    }
        else if (p_ctrl->p_mode->accel_odr > 100)
        {
	    	accel_cfg_tmp |= RM_ICM42670_ACCEL_ODR_200Hz;
	    }
        else if (p_ctrl->p_mode->accel_odr > 50)
        {
	    	accel_cfg_tmp |= RM_ICM42670_ACCEL_ODR_100Hz;
	    }
        else if (p_ctrl->p_mode->accel_odr > 25)
        {
	    	accel_cfg_tmp |= RM_ICM42670_ACCEL_ODR_50Hz;
	    }
        else if (p_ctrl->p_mode->accel_odr > 12)
        {
	    	accel_cfg_tmp |= RM_ICM42670_ACCEL_ODR_25Hz;
	    }
        else if (p_ctrl->p_mode->accel_odr > 6)
        {
	    	accel_cfg_tmp |= RM_ICM42670_ACCEL_ODR_12Hz;
	    }
        else if (p_ctrl->p_mode->accel_odr > 3)
        {
	    	accel_cfg_tmp |= RM_ICM42670_ACCEL_ODR_6Hz;
	    }
        else if (p_ctrl->p_mode->accel_odr > 2)
        {
	    	accel_cfg_tmp |= RM_ICM42670_ACCEL_ODR_3Hz;
	    }
        else
        {
	    	accel_cfg_tmp |= RM_ICM42670_ACCEL_ODR_1Hz;
	    }
    }
    /* Set accel fs&odr */
    p_ctrl->buf[0] = RM_ICM42670_REG_ACCEL_CONFIG0;
    p_ctrl->buf[1] = accel_cfg_tmp;
    err            = rm_icm42670_write(p_ctrl, &p_ctrl->buf[0], 2);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Configure fs and odr of the ICM42670 Gyroscope mode.
 *
 * @retval FSP_SUCCESS              ICM42670 successfully configured.
 * @retval FSP_ERR_TIMEOUT          Communication is timeout.
 * @retval FSP_ERR_ABORTED          Communication is aborted.
 **********************************************************************************************************************/
static fsp_err_t rm_icm42670_gyro_configuration (rm_icm42670_ctrl_t * const p_api_ctrl)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;
    uint8_t gyro_cfg_tmp;

    #if RM_ICM42670_CFG_PARAM_CHECKING_ENABLE
    {
        bool err_res = !((p_ctrl->p_mode->gyro_fs > 2000) || (p_ctrl->p_mode->gyro_fs < -2000));
        FSP_ERROR_RETURN(err_res, FSP_ERR_INVALID_MODE);
    }
    #endif
    if ((p_ctrl->p_mode->gyro_fs > 1000) || (p_ctrl->p_mode->gyro_fs < -1000))
    {
        gyro_cfg_tmp = RM_ICM42670_GYRO_FS_16G << 5;
        p_ctrl->gyro_sensitivity_x10 = RM_ICM42670_GYRO_SENSITIVITY_SCALE_FACTOR_0;
    }
    else if ((p_ctrl->p_mode->gyro_fs > 500) || (p_ctrl->p_mode->gyro_fs < -500))
    {
        gyro_cfg_tmp = RM_ICM42670_GYRO_FS_8G << 5;
        p_ctrl->gyro_sensitivity_x10 = RM_ICM42670_GYRO_SENSITIVITY_SCALE_FACTOR_1;
    }
    else if ((p_ctrl->p_mode->gyro_fs > 250) || (p_ctrl->p_mode->gyro_fs < -250))
    {
        gyro_cfg_tmp = RM_ICM42670_GYRO_FS_4G << 5;
        p_ctrl->gyro_sensitivity_x10 = RM_ICM42670_GYRO_SENSITIVITY_SCALE_FACTOR_2;
    }
    else
    {
        gyro_cfg_tmp = RM_ICM42670_GYRO_FS_2G << 5;
        p_ctrl->gyro_sensitivity_x10 = RM_ICM42670_GYRO_SENSITIVITY_SCALE_FACTOR_3;
    }

    #if RM_ICM42670_CFG_PARAM_CHECKING_ENABLE
    {
        bool err_res = !((p_ctrl->p_mode->gyro_odr > 1600) || (p_ctrl->p_mode->gyro_odr < 12));
        FSP_ERROR_RETURN(err_res, FSP_ERR_INVALID_MODE);
    }
    #endif

    if (p_ctrl->p_mode->gyro_odr > 800)
    {
        gyro_cfg_tmp |= RM_ICM42670_GYRO_ODR_1600Hz;
	}
    else if (p_ctrl->p_mode->gyro_odr > 400)
    {
        gyro_cfg_tmp |= RM_ICM42670_GYRO_ODR_800Hz;
	}
    else if (p_ctrl->p_mode->gyro_odr > 200)
    {
        gyro_cfg_tmp |= RM_ICM42670_GYRO_ODR_400Hz;
	}
    else if (p_ctrl->p_mode->gyro_odr > 100)
    {
        gyro_cfg_tmp |= RM_ICM42670_GYRO_ODR_200Hz;
	}
    else if (p_ctrl->p_mode->gyro_odr > 50)
    {
        gyro_cfg_tmp |= RM_ICM42670_GYRO_ODR_100Hz;
	}
    else if (p_ctrl->p_mode->gyro_odr > 25)
    {
        gyro_cfg_tmp |= RM_ICM42670_GYRO_ODR_50Hz;
	}
    else if (p_ctrl->p_mode->gyro_odr > 13)
    {
        gyro_cfg_tmp |= RM_ICM42670_GYRO_ODR_25Hz;
	}
    else
    {
        gyro_cfg_tmp |= RM_ICM42670_GYRO_ODR_12Hz;
	}

    /* Set gyro fs&odr */
    p_ctrl->buf[0] = RM_ICM42670_REG_GYRO_CONFIG0;
    p_ctrl->buf[1] = gyro_cfg_tmp;
    err            = rm_icm42670_write(p_ctrl, &p_ctrl->buf[0], 2);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}

#endif
