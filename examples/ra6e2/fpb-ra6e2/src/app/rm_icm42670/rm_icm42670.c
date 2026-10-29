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


/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include <rm_icm42670.h>
#include <stdio.h>

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/* Definitions of Open flag */
#define RM_ICM42670_OPEN                         (0x632A5721UL) // Open state

/* Definitions of Timeout */
#define RM_ICM42670_TIMEOUT                      (10)
#define RM_ICM42670_10MS                         (10)

/* Definitions of Register data */
#define RM_ICM42670_REG_DATA_PPG_PS_GAIN         (0x09)

/* Definitions of Commands */
#define RM_ICM42670_COMMAND_SOFTWARE_RESET       (0x80)

/* Definitions of Register address */
#define RM_ICM42670_REG_ADDR_MCLK_RDY            (0x00) // 0: Indicates internal clock is currently not running;1: Indicates internal clock is currently running
#define RM_ICM42670_REG_ADDR_SIGNAL_PATH_RESET   (0x02) // SOFT_RESET_DEVICE_CONFIG & FIFO FLUSH
#define RM_ICM42670_REG_ADDR_INT_CONFIG          (0x06) // INT1,INT2 MODE,DRIVE CIRCUIT, POLARITY configure
#define RM_ICM42670_REG_ADDR_PWR_MGMT0           (0x1F) // Power mode set: accel mode, gyro mode, idle mode

#define RM_ICM42670_REG_ADDR_INT_STATUS          (0x39) // LS operation mode control, software (SW) reset

/* Definitions of INT Status Mask */
/* Definitions of INT_STATUS_DRDY(0x39) Mask */
#define RM_ICM42670_MASK_DATA_RDY_STATUS         (0x01)     //DATA_RDY_INT

/* Definitions of INT_STATUS(0x3A) Mask */
#define RM_ICM42670_MASK_AGC_RDY_STATUS          (0x01)
#define RM_ICM42670_MASK_FIFO_FULL_STATUS        (0x02)
#define RM_ICM42670_MASK_FIFO_THS_STATUS         (0x04)
#define RM_ICM42670_MASK_RESET_DONE              (0x10)
#define RM_ICM42670_MASK_PLL_RDY_STATUS          (0x20)
#define RM_ICM42670_MASK_FSYNC_STATUS            (0x40)
#define RM_ICM42670_MASK_ST_STATUS               (0x80)

/* Definitions of INT_STATUS2(0x3B) Mask */
#define RM_ICM42670_MASK_WOM_Z_STATUS            (0x01)
#define RM_ICM42670_MASK_WOM_Y_STATUS            (0x02)
#define RM_ICM42670_MASK_WOM_X_STATUS            (0x04)
#define RM_ICM42670_MASK_SMD_STATUS              (0x08)

/* Definitions of INT_STATUS3(0x3C) Mask */
#define RM_ICM42670_MASK_LOWG_DET_STATUS          (0x02)
#define RM_ICM42670_MASK_FF_DET_STATUS            (0x04)
#define RM_ICM42670_MASK_TILT_DET_STATUS          (0x08)
#define RM_ICM42670_MASK_STEP_CNT_OVF_STATUS      (0x10)
#define RM_ICM42670_MASK_STEP_DET_STATUS          (0x20)

/* Definitions of FIFO */
#define RM_ICM42670_FIFO_MAX_SAMPLES_1K          (1024) //1K
#define RM_ICM42670_FIFO_MAX_SAMPLES_EXPAND      (2304) //APEX disabled, FIFO expanded to 2.25K
#define RM_ICM42670_FIFO_MASK_PPG2_0XFE          (0xFE)

#define RM_ICM42670_SELECT_RC_OSC    (0x00)   ///< Always select internal RC oscillator.
/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Global function prototypes
 **********************************************************************************************************************/
void      rm_icm42670_comms_i2c_callback(rm_comms_callback_args_t * p_args);
void      icm42670_comms_i2c_callback(rm_icm42670_callback_args_t * p_args);
fsp_err_t rm_icm42670_read(rm_icm42670_ctrl_t * const p_api_ctrl, rm_comms_write_read_params_t write_read_params);
fsp_err_t rm_icm42670_write(rm_icm42670_ctrl_t * const p_api_ctrl, uint8_t * const p_src, uint8_t const bytes);
fsp_err_t rm_icm42670_int_cfg_register_write(rm_icm42670_ctrl_t * const p_api_ctrl);
fsp_err_t        rm_icm42670_fifo_info_reset(rm_icm42670_ctrl_t * const p_api_ctrl);
fsp_err_t        rm_icm42670_all_interrupt_bits_clear(rm_icm42670_ctrl_t * const p_api_ctrl);

extern fsp_err_t rm_icm42670_delay_ms(rm_icm42670_ctrl_t * const p_ctrl, uint32_t const delay_ms);
extern fsp_err_t rm_icm42670_delay_us(rm_icm42670_ctrl_t * const p_ctrl, uint32_t const delay_us);
extern fsp_err_t rm_icm42670_irq_open(rm_icm42670_ctrl_t * const p_api_ctrl);
extern fsp_err_t rm_icm42670_irq_close(rm_icm42670_ctrl_t * const p_api_ctrl);

extern fsp_err_t rm_icm42670_os_semaphore_create(rm_icm42670_semaphore_t const * p_semaphore);
extern fsp_err_t rm_icm42670_os_semaphore_delete(rm_icm42670_semaphore_t const * p_semaphore);
extern fsp_err_t rm_icm42670_os_semaphore_acquire(rm_icm42670_semaphore_t const * p_semaphore, uint32_t const timeout);
extern fsp_err_t rm_icm42670_os_semaphore_release_from_ISR(rm_icm42670_semaphore_t const * p_semaphore);

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static void rm_icm42670_process_in_callback(rm_icm42670_ctrl_t * const          p_api_ctrl,
                                          rm_icm42670_callback_args_t * const p_args);
//static void      rm_icm42670_unread_samples_calculate(rm_icm42670_ctrl_t * const p_api_ctrl);
static fsp_err_t rm_icm42670_software_reset(rm_icm42670_ctrl_t * const p_api_ctrl);
static fsp_err_t rm_icm42670_clk_sel (rm_icm42670_ctrl_t * const p_api_ctrl);

static fsp_err_t rm_icm42670_who_am_I (rm_icm42670_ctrl_t * const p_api_ctrl);
static fsp_err_t rm_icm42670_mclk_rdy (rm_icm42670_ctrl_t * const p_api_ctrl);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
 volatile uint32_t g_sensor_open = 0;
 volatile bool g_i2c_flag = 0;

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
rm_icm42670_api_t const g_icm42670_on_icm42670 =
{
    .open                  = RM_ICM42670_Open,
    .close                 = RM_ICM42670_Close,
    .measurementStart      = RM_ICM42670_MeasurementStart,
    .measurementStop       = RM_ICM42670_MeasurementStop,
    .accelRead             = RM_ICM42670_AccelRead,
    .accelDataCalculate    = RM_ICM42670_AccelDataCalculate,
    .gyroRead              = RM_ICM42670_GyroRead,
    .gyroDataCalculate     = RM_ICM42670_GyroDataCalculate,
    .tempRead              = RM_ICM42670_TempRead,
    .tempDataCalculate     = RM_ICM42670_TempDataCalculate,
    .deviceInterruptCfgSet = RM_ICM42670_DeviceInterruptCfgSet,
    .fifoInfoGet           = RM_ICM42670_FifoInfoGet,
    .deviceStatusGet       = RM_ICM42670_DeviceStatusGet,
};

/*******************************************************************************************************************//**
 * @addtogroup RM_ICM42670
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Opens and configures the ICM42670 Middle module. Implements @ref rm_icm42670_api_t::open.
 *
 * Example:
 * @snippet rm_icm42670_example.c RM_ICM42670_Open
 *
 * @retval FSP_SUCCESS              ICM42670 successfully configured.
 * @retval FSP_ERR_ASSERTION        Null pointer, or one or more configuration options is invalid.
 * @retval FSP_ERR_ALREADY_OPEN     Module is already open.  This module can only be opened once.
 * @retval FSP_ERR_TIMEOUT          Communication is timeout.
 * @retval FSP_ERR_ABORTED          Communication is aborted.
 **********************************************************************************************************************/
fsp_err_t RM_ICM42670_Open (rm_icm42670_ctrl_t * const p_api_ctrl, rm_icm42670_cfg_t const * const p_cfg)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t     * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;
    rm_icm42670_mode_extended_cfg_t * p_mode;
#if RM_ICM42670_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_ctrl);
    FSP_ASSERT(NULL != p_cfg);
    FSP_ASSERT(NULL != p_cfg->p_comms_instance);
    FSP_ASSERT(NULL != p_cfg->p_extend);
    FSP_ERROR_RETURN(RM_ICM42670_OPEN != p_ctrl->open, FSP_ERR_ALREADY_OPEN);
#endif
    p_ctrl->p_cfg = p_cfg;
    p_mode        = (rm_icm42670_mode_extended_cfg_t *) p_cfg->p_extend;

#if RM_ICM42670_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_mode->p_api);
    FSP_ASSERT(NULL != p_ctrl->p_cfg->p_semaphore);
#endif
    /* Set instances */
    p_ctrl->p_comms_i2c_instance = p_cfg->p_comms_instance;
    p_ctrl->p_irq_instance       = p_cfg->p_irq_instance;

    /* Set operation mode */
    p_ctrl->p_mode = p_mode;

    /* Set parameters */
    p_ctrl->p_context            = p_cfg->p_context;
    p_ctrl->p_comms_callback     = p_cfg->p_comms_callback;
    p_ctrl->p_irq_callback       = p_cfg->p_irq_callback;
    p_ctrl->p_device_status      = NULL;
    p_ctrl->p_fifo_info          = NULL;
    p_ctrl->fifo_reset                 = false;
    p_ctrl->accel_sensitivity_shift    = RM_ICM42670_ACCEL_SENSITIVITY_SCALE_FACTOR_3;
    p_ctrl->gyro_sensitivity_x10       = RM_ICM42670_GYRO_SENSITIVITY_SCALE_FACTOR_3;
    p_ctrl->interrupt_bits_clear       = false;

    /* Open Communications middleware with p_context = this control block (used by rm_icm42670_comms_i2c_callback).
     * Static because rm_comms keeps a pointer to its config. */
    static rm_comms_cfg_t s_comms_cfg;
    s_comms_cfg           = *p_ctrl->p_comms_i2c_instance->p_cfg;
    s_comms_cfg.p_context = p_ctrl;

    err = p_ctrl->p_comms_i2c_instance->p_api->open(p_ctrl->p_comms_i2c_instance->p_ctrl, &s_comms_cfg);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    /* Set semaphore */
    p_ctrl->p_semaphore = p_ctrl->p_cfg->p_semaphore;

    /* Create semaphore */
    err = rm_icm42670_os_semaphore_create(p_ctrl->p_semaphore);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    /* Software reset */
    err = rm_icm42670_software_reset(p_ctrl);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    /* Delay 10ms */
    err = rm_icm42670_delay_ms(p_ctrl, RM_ICM42670_10MS);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    /* clock select */
    err = rm_icm42670_clk_sel(p_ctrl);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    /* Delay 10ms */
    err = rm_icm42670_delay_ms(p_ctrl, 1);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    err = rm_icm42670_mclk_rdy(p_ctrl);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    /* Clear all interrupt bits */
    err = rm_icm42670_all_interrupt_bits_clear(p_ctrl);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    /* Open operation mode */
    err = p_mode->p_api->open(p_ctrl, p_cfg);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    err = rm_icm42670_who_am_I(p_ctrl);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);


    /* Open and enable the data-ready IRQ (INT1 -> ICU) */
    if (NULL != p_ctrl->p_irq_instance)
    {
        err = rm_icm42670_irq_open(p_ctrl);
        FSP_ERROR_RETURN(FSP_SUCCESS == err, err);
    }

    /* Set open flag */
    g_sensor_open = RM_ICM42670_OPEN;
    p_ctrl->open = RM_ICM42670_OPEN;

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Disables specified ICM42670 control block. Implements @ref rm_icm42670_api_t::close.
 *
 * @retval FSP_SUCCESS              Successfully closed.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 **********************************************************************************************************************/
fsp_err_t RM_ICM42670_Close (rm_icm42670_ctrl_t * const p_api_ctrl)
{
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;

#if RM_ICM42670_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_ctrl);
    FSP_ERROR_RETURN(RM_ICM42670_OPEN == p_ctrl->open, FSP_ERR_NOT_OPEN);
#endif


    /* Close Communications Middleware */
    p_ctrl->p_comms_i2c_instance->p_api->close(p_ctrl->p_comms_i2c_instance->p_ctrl);

    /* Close IRQ */
    if (NULL != p_ctrl->p_irq_instance)
    {
        rm_icm42670_irq_close(p_ctrl);
    }

    /* delete a semaphore */
    rm_icm42670_os_semaphore_delete(p_ctrl->p_semaphore);

    /* Clear Open flag */
    p_ctrl->open = 0;
    g_sensor_open = 0;

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Start measurement.
 * Implements @ref rm_icm42670_api_t::measurementStart
 *
 * @retval FSP_SUCCESS              Successfully results are read.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not opened configured.
 **********************************************************************************************************************/
fsp_err_t RM_ICM42670_MeasurementStart (rm_icm42670_ctrl_t * const p_api_ctrl)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;

#if RM_ICM42670_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_ctrl);
    FSP_ERROR_RETURN(RM_ICM42670_OPEN == p_ctrl->open, FSP_ERR_NOT_OPEN);
#endif

    /* Start measurement */
    err = p_ctrl->p_mode->p_api->measurementStart(p_ctrl);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Stop measurement.
 * If device interrupt is enabled, interrupt bits are cleared after measurement stop.
 * If PPG mode, FIFO information is also reset after measurement stop.
 * In RTOS and Accel/Gyroimity/Accel Gyroimity mode, if device interrupt is enabled, blocks 2 bytes on the I2C bus.
 * In RTOS and PPG mode, if device interrupt is enabled, blocks 6 bytes on the I2C bus. If device interrupt is disabled,
 * blocks 4 bytes on the I2C bus.
 * Implements @ref rm_icm42670_api_t::measurementStop
 *
 * @retval FSP_SUCCESS              Successfully results are read.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not opened configured.
 **********************************************************************************************************************/
fsp_err_t RM_ICM42670_MeasurementStop (rm_icm42670_ctrl_t * const p_api_ctrl)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;

#if RM_ICM42670_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_ctrl);
    FSP_ERROR_RETURN(RM_ICM42670_OPEN == p_ctrl->open, FSP_ERR_NOT_OPEN);
#endif

    /* Stop measurement */
    err = p_ctrl->p_mode->p_api->measurementStop(p_ctrl);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Reads Accel data from ICM42670 device.
 * If device interrupt is enabled, interrupt bits are cleared after data read.
 * Implements @ref rm_icm42670_api_t::accelRead
 *
 * @retval FSP_SUCCESS              Successfully results are read.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not opened configured.
 **********************************************************************************************************************/
fsp_err_t RM_ICM42670_AccelRead (rm_icm42670_ctrl_t * const     p_api_ctrl,
                               rm_icm42670_raw_data_t * const p_raw_data)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;

#if RM_ICM42670_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_ctrl);
    FSP_ASSERT(NULL != p_raw_data);
    FSP_ERROR_RETURN(RM_ICM42670_OPEN == p_ctrl->open, FSP_ERR_NOT_OPEN);
#endif

    /* Read Accel raw data */
    err = p_ctrl->p_mode->p_api->accelRead(p_ctrl, p_raw_data);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Calculate accel data from raw data.
 * Implements @ref rm_icm42670_api_t::accelDataCalculate
 *
 * @retval FSP_SUCCESS              Successfully results are read.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not opened configured.
 **********************************************************************************************************************/
fsp_err_t RM_ICM42670_AccelDataCalculate (rm_icm42670_ctrl_t * const       p_api_ctrl,
                                        rm_icm42670_raw_data_t * const   p_raw_data,
                                        rm_icm42670_accel_data_t * const p_icm42670_data)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;

#if RM_ICM42670_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_ctrl);
    FSP_ASSERT(NULL != p_raw_data);
    FSP_ASSERT(NULL != p_icm42670_data);
    FSP_ERROR_RETURN(RM_ICM42670_OPEN == p_ctrl->open, FSP_ERR_NOT_OPEN);
#endif

    /* Calculate Accel data from raw data */
    err = p_ctrl->p_mode->p_api->accelDataCalculate(p_ctrl, p_raw_data, p_icm42670_data);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Reads gyroscope data from ICM42670 device.
 * If device interrupt is enabled, interrupt bits are cleared after data read.
 * Implements @ref rm_icm42670_api_t::gyroRead
 *
 * @retval FSP_SUCCESS              Successfully results are read.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not opened configured.
 **********************************************************************************************************************/
fsp_err_t RM_ICM42670_GyroRead (rm_icm42670_ctrl_t * const p_api_ctrl, rm_icm42670_raw_data_t * const p_raw_data)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;

#if RM_ICM42670_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_ctrl);
    FSP_ASSERT(NULL != p_raw_data);
    FSP_ERROR_RETURN(RM_ICM42670_OPEN == p_ctrl->open, FSP_ERR_NOT_OPEN);
#endif

    /* Read Gyro raw data */
    err = p_ctrl->p_mode->p_api->gyroRead(p_ctrl, p_raw_data);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Calculate gyroscope data from raw data.
 * Implements @ref rm_icm42670_api_t::gyroDataCalculate
 *
 * @retval FSP_SUCCESS              Successfully results are read.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not opened configured.
 **********************************************************************************************************************/
fsp_err_t RM_ICM42670_GyroDataCalculate (rm_icm42670_ctrl_t * const      p_api_ctrl,
                                       rm_icm42670_raw_data_t * const  p_raw_data,
                                       rm_icm42670_gyro_data_t * const p_icm42670_data)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;

#if RM_ICM42670_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_ctrl);
    FSP_ASSERT(NULL != p_raw_data);
    FSP_ASSERT(NULL != p_icm42670_data);
    FSP_ERROR_RETURN(RM_ICM42670_OPEN == p_ctrl->open, FSP_ERR_NOT_OPEN);
#endif

    /* Calculate Gyroimity data from raw data */
    err = p_ctrl->p_mode->p_api->gyroDataCalculate(p_ctrl, p_raw_data, p_icm42670_data);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Reads Temperature data from ICM42670 device.
 * Implements @ref rm_icm42670_api_t::TempRead
 *
 * @retval FSP_SUCCESS              Successfully results are read.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not opened configured.
 **********************************************************************************************************************/
fsp_err_t RM_ICM42670_TempRead (rm_icm42670_ctrl_t * const     p_api_ctrl,
                             rm_icm42670_raw_data_t * const p_raw_data)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;

#if RM_ICM42670_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_ctrl);
    FSP_ASSERT(NULL != p_raw_data);
    FSP_ERROR_RETURN(RM_ICM42670_OPEN == p_ctrl->open, FSP_ERR_NOT_OPEN);
#endif

    /* Read PPG raw data */
    err = p_ctrl->p_mode->p_api->tempRead(p_ctrl, p_raw_data);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Calculate Temperature data from raw data.
 * Implements @ref rm_icm42670_api_t::ppgDataCalculate
 *
 * @retval FSP_SUCCESS              Successfully results are read.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not opened configured.
 **********************************************************************************************************************/
fsp_err_t RM_ICM42670_TempDataCalculate (rm_icm42670_ctrl_t * const     p_api_ctrl,
                                      rm_icm42670_raw_data_t * const p_raw_data,
                                      rm_icm42670_temp_data_t * const p_icm42670_data)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;

#if RM_ICM42670_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_ctrl);
    FSP_ASSERT(NULL != p_raw_data);
    FSP_ASSERT(NULL != p_icm42670_data);
    FSP_ERROR_RETURN(RM_ICM42670_OPEN == p_ctrl->open, FSP_ERR_NOT_OPEN);
#endif

    /* Calculate PPG data from raw data */
    err = p_ctrl->p_mode->p_api->tempDataCalculate(p_ctrl, p_raw_data, p_icm42670_data);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}
/*******************************************************************************************************************//**
 * @brief  Set device interrupt configurations. This function should be called after calling RM_ICM42670_MeasurementStop().
 * Implements @ref rm_icm42670_api_t::deviceInterruptCfgSet
 *
 * @retval FSP_SUCCESS              Successfully results are read.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not opened configured.
 **********************************************************************************************************************/
fsp_err_t RM_ICM42670_DeviceInterruptCfgSet (rm_icm42670_ctrl_t * const               p_api_ctrl,
                                           rm_icm42670_device_interrupt_cfg_t const interrupt_cfg)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;

#if RM_ICM42670_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_ctrl);
    FSP_ERROR_RETURN(RM_ICM42670_OPEN == p_ctrl->open, FSP_ERR_NOT_OPEN);
#endif

    /* Set PPG device interrupt configuration */
    err = p_ctrl->p_mode->p_api->deviceInterruptCfgSet(p_ctrl, interrupt_cfg);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Get FIFO information from ICM42670 device.(Reserved)
 * Implements @ref rm_icm42670_api_t::fifoInfoGet
 *
 * @retval FSP_SUCCESS              Successfully results are read.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not opened configured.
 **********************************************************************************************************************/
fsp_err_t RM_ICM42670_FifoInfoGet (rm_icm42670_ctrl_t * const p_api_ctrl, rm_icm42670_fifo_info_t * const p_fifo_info)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;

#if RM_ICM42670_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_ctrl);
    FSP_ASSERT(NULL != p_fifo_info);
    FSP_ERROR_RETURN(RM_ICM42670_OPEN == p_ctrl->open, FSP_ERR_NOT_OPEN);
#endif

    /* Get PPG FIFO information */
    err = p_ctrl->p_mode->p_api->fifoInfoGet(p_ctrl, p_fifo_info);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Get device status from ICM42670 device. Clear all interrupt bits.
 * Implements @ref rm_icm42670_api_t::deviceStatusGet
 *
 * @retval FSP_SUCCESS              Successfully results are read.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not opened configured.
 * @retval FSP_ERR_TIMEOUT          Communication is timeout.
 * @retval FSP_ERR_ABORTED          Communication is aborted.
 **********************************************************************************************************************/
fsp_err_t RM_ICM42670_DeviceStatusGet (rm_icm42670_ctrl_t * const p_api_ctrl, rm_icm42670_device_status_t * const p_status)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t  * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;
    rm_comms_write_read_params_t write_read_params;

#if RM_ICM42670_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_ctrl);
    FSP_ASSERT(NULL != p_status);
    FSP_ERROR_RETURN(RM_ICM42670_OPEN == p_ctrl->open, FSP_ERR_NOT_OPEN);
#endif

    /* Set pointer */
    p_ctrl->p_device_status = p_status;

    /* Get status */
    p_ctrl->register_address     = RM_ICM42670_REG_ADDR_INT_STATUS; //Begin with 0x39(INT_STATUS_DRDY)
    write_read_params.p_src      = &p_ctrl->register_address;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = &p_ctrl->buf[0];
    write_read_params.dest_bytes = 4;                          //0x39(INT_STATUS_DRDY)--0x3C(INT_STATUS3)
    err = rm_icm42670_read(p_ctrl, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @} (end addtogroup RM_ICM42670)
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief ICM42670 callback function called in the I2C Communications Middleware callback function.
 **********************************************************************************************************************/
void rm_icm42670_comms_i2c_callback (rm_comms_callback_args_t * p_args)
{
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_args->p_context;
    rm_icm42670_callback_args_t   icm42670_callback_args;

    /* Set context */
    icm42670_callback_args.p_context = p_ctrl->p_context;

    /* Set event */
    switch (p_args->event)
    {
        case RM_COMMS_EVENT_OPERATION_COMPLETE:
        {
            icm42670_callback_args.event = RM_ICM42670_EVENT_SUCCESS;
            break;
        }

        case RM_COMMS_EVENT_ERROR:
        default:
        {
            icm42670_callback_args.event = RM_ICM42670_EVENT_ERROR;
            break;
        }
    }

//    if (RM_ICM42670_OPEN != p_ctrl->open)
    if (RM_ICM42670_OPEN != g_sensor_open)
    {
        /* Set flag */
        //p_ctrl->init_process_params.communication_finished = true;
        g_i2c_flag = true;

        /* Set event */
        p_ctrl->init_process_params.event = icm42670_callback_args.event;
        rm_icm42670_process_in_callback(p_ctrl, &icm42670_callback_args);
    }
    else
    {
        if (RM_ICM42670_EVENT_SUCCESS == icm42670_callback_args.event)
        {
            g_i2c_flag = true;
            if (NULL != p_ctrl->p_device_status)
            {
                /* Check device status */
                rm_icm42670_device_status_check(p_ctrl);

                /* Clear pointer */
                p_ctrl->p_device_status = NULL;

                /* Call user callback function */
                rm_icm42670_process_in_callback(p_ctrl, &icm42670_callback_args);
            }
            else if (NULL != p_ctrl->p_fifo_info)
            {
                /* Calculate the number of unread FIFO samples */
               // rm_icm42670_unread_samples_calculate(p_ctrl);

                /* Clear pointer */
                p_ctrl->p_fifo_info = NULL;

                /* Call user callback function */
                rm_icm42670_process_in_callback(p_ctrl, &icm42670_callback_args);
            }
            else if (false != p_ctrl->fifo_reset)
            {
                /* Clear flag */
                p_ctrl->fifo_reset = false;

                /* Release a semaphore */
                rm_icm42670_os_semaphore_release_from_ISR(p_ctrl->p_semaphore);
            }

            else if (false != p_ctrl->interrupt_bits_clear)
            {
                /* Clear flag */
                p_ctrl->interrupt_bits_clear = false;

                /* Release a semaphore */
                rm_icm42670_os_semaphore_release_from_ISR(p_ctrl->p_semaphore);
            }
            else
            {

                /* Call user callback function */
                rm_icm42670_process_in_callback(p_ctrl, &icm42670_callback_args);
            }
        }
        else
        {
            g_i2c_flag = false;
            /* Call user callback function */
            rm_icm42670_process_in_callback(p_ctrl, &icm42670_callback_args);
        }
    }
}

/*******************************************************************************************************************//**
 * @brief Read data from ICM42670 device.
 *
 * @retval FSP_SUCCESS              Successfully started.
 * @retval FSP_ERR_TIMEOUT          communication is timeout.
 * @retval FSP_ERR_ABORTED          communication is aborted.
 **********************************************************************************************************************/
fsp_err_t rm_icm42670_read (rm_icm42670_ctrl_t * const p_api_ctrl, rm_comms_write_read_params_t write_read_params)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;
    uint16_t counter = 0;

    /* Clear flag */
    while((true != g_i2c_flag) && ( counter < RM_ICM42670_TIMEOUT ) ) {
        rm_icm42670_delay_ms(p_ctrl, 1);
        counter++;
    }
    g_i2c_flag = false;
    counter = 0;

    /* WriteRead data */
    err = p_ctrl->p_comms_i2c_instance->p_api->writeRead(p_ctrl->p_comms_i2c_instance->p_ctrl, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    /* Wait callback */
    //while (false == p_ctrl->init_process_params.communication_finished)
    while(false == g_i2c_flag)
    {
        rm_icm42670_delay_us(p_ctrl, 1);
        counter++;
        FSP_ERROR_RETURN(RM_ICM42670_TIMEOUT >= (counter/1000), FSP_ERR_TIMEOUT);
    }

    /* Check callback event */
    FSP_ERROR_RETURN(RM_ICM42670_EVENT_SUCCESS == p_ctrl->init_process_params.event, FSP_ERR_ABORTED);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Write data to ICM42670 device.
 *
 * @retval FSP_SUCCESS              Successfully started.
 * @retval FSP_ERR_TIMEOUT          communication is timeout.
 * @retval FSP_ERR_ABORTED          communication is aborted.
 **********************************************************************************************************************/
fsp_err_t rm_icm42670_write (rm_icm42670_ctrl_t * const p_api_ctrl, uint8_t * const p_src, uint8_t const bytes)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;
    uint16_t counter = 0;

    while((true != g_i2c_flag) && ( counter < RM_ICM42670_TIMEOUT ) ) {
        rm_icm42670_delay_ms(p_ctrl, 1);
        counter++;
    }

    g_i2c_flag = false;
    counter = 0;

    /* Write data */
    err = p_ctrl->p_comms_i2c_instance->p_api->write(p_ctrl->p_comms_i2c_instance->p_ctrl, p_src, (uint32_t) bytes);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);


    /* Wait callback */
    while(false == g_i2c_flag)
    {
        rm_icm42670_delay_us(p_ctrl, 1);
        counter++;
        FSP_ERROR_RETURN(RM_ICM42670_TIMEOUT >= (counter/1000), FSP_ERR_TIMEOUT);
    }

    /* Check callback event */
    //FSP_ERROR_RETURN(RM_ICM42670_EVENT_SUCCESS == p_ctrl->init_process_params.event, FSP_ERR_ABORTED);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Write data to the INT_CFG and INT_PST registers.
 *
 * @retval FSP_SUCCESS              Successfully started.
 * @retval FSP_ERR_TIMEOUT          Communication is timeout.
 * @retval FSP_ERR_ABORTED          Communication is aborted.
 **********************************************************************************************************************/
fsp_err_t rm_icm42670_int_cfg_register_write(rm_icm42670_ctrl_t * const p_api_ctrl)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;

    /* Set the interrupt source data */
    p_ctrl->buf[0] = RM_ICM42670_REG_INT_SOURCE0;
    p_ctrl->buf[1] = (uint8_t)(p_ctrl->p_mode->interrupt_source & 0x000000FF);
    p_ctrl->buf[2] = (uint8_t)(p_ctrl->p_mode->interrupt_source & 0x0000FF00);
    p_ctrl->buf[3] = (uint8_t)(p_ctrl->p_mode->interrupt_source & 0x00FF0000);
    p_ctrl->buf[4] = (uint8_t)(p_ctrl->p_mode->interrupt_source & 0xFF000000);

    /* Write the interrupt source data */
    err = rm_icm42670_write(p_ctrl, &p_ctrl->buf[0], 5);

    /* Set the interrupt config data */
    p_ctrl->buf[0] = RM_ICM42670_REG_INT_CONFIG;
    p_ctrl->buf[1] = p_ctrl->p_mode->interrupt_config;

    R_BSP_SoftwareDelay(50, 1);

    /* Write the interrupt config data */
    err |= rm_icm42670_write(p_ctrl, &p_ctrl->buf[0], 2);

    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Clear all interrupt bits.
 *
 * @retval FSP_SUCCESS              Successfully started.
 * @retval FSP_ERR_TIMEOUT          Communication is timeout.
 * @retval FSP_ERR_ABORTED          Communication is aborted.
 **********************************************************************************************************************/
fsp_err_t rm_icm42670_all_interrupt_bits_clear (rm_icm42670_ctrl_t * const p_api_ctrl)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t  * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;
    rm_comms_write_read_params_t write_read_params;

    /* Clear all interrupt bits */
    p_ctrl->register_address     = RM_ICM42670_REG_ADDR_INT_STATUS;
    write_read_params.p_src      = &p_ctrl->register_address;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = &p_ctrl->buf[0];
    write_read_params.dest_bytes = 4;
    err = rm_icm42670_read(p_ctrl, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * Internal icm42670 private function.
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Process in callback function. Call user callback.
 **********************************************************************************************************************/
static void rm_icm42670_process_in_callback (rm_icm42670_ctrl_t * const          p_api_ctrl,
                                           rm_icm42670_callback_args_t * const p_args)
{
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;

    if (NULL != p_ctrl->p_comms_callback)
    {
        /* Call callback function */
        p_ctrl->p_comms_callback(p_args);
    }
}

/*******************************************************************************************************************//**
 * @brief Check device status and set callback event.
 **********************************************************************************************************************/
fsp_err_t rm_icm42670_device_status_check (rm_icm42670_ctrl_t * const p_api_ctrl)
{
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;

    /* Check if INT_STATUS_DRDY occurs */
    if (0x00 != (p_ctrl->buf[0] & RM_ICM42670_MASK_DATA_RDY_STATUS))
    {
        p_ctrl->p_device_status->data_ready = true;
    }
    else
    {
        p_ctrl->p_device_status->data_ready = false;
    }
    /* Check if power-on-reset occurs */
    if (0x00 != (p_ctrl->buf[1] & RM_ICM42670_MASK_RESET_DONE))
    {
        p_ctrl->p_device_status->power_on_reset_occur = true;
    }
    else
    {
        p_ctrl->p_device_status->power_on_reset_occur = false;
    }

    /* Check if Accel mode interrupt condition occurs */
    if (0x00 != (p_ctrl->buf[1] & RM_ICM42670_MASK_AGC_RDY_STATUS))
    {
        p_ctrl->p_device_status->agc_ready = true;
    }
    else
    {
        p_ctrl->p_device_status->agc_ready = false;
    }

    /* Check if FIFO full interrupt occurs */
    if (0x00 != (p_ctrl->buf[1] & RM_ICM42670_MASK_FIFO_FULL_STATUS))
    {
        p_ctrl->p_device_status->fifo_afull_int_occur = true;
    }
    else
    {
        p_ctrl->p_device_status->fifo_afull_int_occur = false;
    }

    /* Check if FIFO threshold interrupt occurs */
    if (0x00 != (p_ctrl->buf[1] & RM_ICM42670_MASK_FIFO_THS_STATUS))
    {
        p_ctrl->p_device_status->fifo_ths_int_occur = true;
    }
    else
    {
        p_ctrl->p_device_status->fifo_ths_int_occur = false;
    }

    /* Check if PLL ready interrupt occurs */
    if (0x00 != (p_ctrl->buf[1] & RM_ICM42670_MASK_PLL_RDY_STATUS))
    {
        p_ctrl->p_device_status->pll_ready = true;
    }
    else
    {
        p_ctrl->p_device_status->pll_ready = false;
    }

    /* Check if FSYNC interrupt occurs */
    if (0x00 != (p_ctrl->buf[1] & RM_ICM42670_MASK_FSYNC_STATUS))
    {
        p_ctrl->p_device_status->FSYNC_int_occur = true;
    }
    else
    {
        p_ctrl->p_device_status->FSYNC_int_occur = false;
    }

    /* Check if selftest done interrupt occurs */
    if (0x00 != (p_ctrl->buf[1] & RM_ICM42670_MASK_ST_STATUS))
    {
        p_ctrl->p_device_status->self_test_done = true;
    }
    else
    {
        p_ctrl->p_device_status->self_test_done = false;
    }

    /* Check if WOM_Z interrupt occurs */
    if (0x00 != (p_ctrl->buf[2] & RM_ICM42670_MASK_WOM_Z_STATUS))
    {
        p_ctrl->p_device_status->wom_z_occur = true;
    }
    else
    {
        p_ctrl->p_device_status->wom_z_occur = false;
    }

    /* Check if WOM_Y interrupt occurs */
    if (0x00 != (p_ctrl->buf[2] & RM_ICM42670_MASK_WOM_Y_STATUS))
    {
        p_ctrl->p_device_status->wom_y_occur = true;
    }
    else
    {
        p_ctrl->p_device_status->wom_y_occur = false;
    }

    /* Check if WOM_X interrupt occurs */
    if (0x00 != (p_ctrl->buf[2] & RM_ICM42670_MASK_WOM_X_STATUS))
    {
        p_ctrl->p_device_status->wom_x_occur = true;
    }
    else
    {
        p_ctrl->p_device_status->wom_x_occur = false;
    }

    /* Check if SMD interrupt occurs */
    if (0x00 != (p_ctrl->buf[2] & RM_ICM42670_MASK_SMD_STATUS))
    {
        p_ctrl->p_device_status->SMD_occur = true;
    }
    else
    {
        p_ctrl->p_device_status->SMD_occur = false;
    }

    /* Check if LowG detection */
    if (0x00 != (p_ctrl->buf[3] & RM_ICM42670_MASK_LOWG_DET_STATUS))
    {
        p_ctrl->p_device_status->lowG_occur = true;
    }
    else
    {
        p_ctrl->p_device_status->lowG_occur = false;
    }

    /* Check if FF detection */
    if (0x00 != (p_ctrl->buf[3] & RM_ICM42670_MASK_FF_DET_STATUS))
    {
        p_ctrl->p_device_status->freefall_occur = true;
    }
    else
    {
        p_ctrl->p_device_status->freefall_occur = false;
    }

    /* Check if tilt detection */
    if (0x00 != (p_ctrl->buf[3] & RM_ICM42670_MASK_TILT_DET_STATUS))
    {
        p_ctrl->p_device_status->tilt_detection = true;
    }
    else
    {
        p_ctrl->p_device_status->tilt_detection = false;
    }

    /* Check if step count overflow occurs */
    if (0x00 != (p_ctrl->buf[3] & RM_ICM42670_MASK_STEP_CNT_OVF_STATUS))
    {
        p_ctrl->p_device_status->step_count_overflow_occur = true;
    }
    else
    {
        p_ctrl->p_device_status->step_count_overflow_occur = false;
    }

    /* Check if step detection */
    if (0x00 != (p_ctrl->buf[3] & RM_ICM42670_MASK_STEP_DET_STATUS))
    {
        p_ctrl->p_device_status->step_detection = true;
    }
    else
    {
        p_ctrl->p_device_status->step_detection = false;
    }
    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Software reset.
 *
 * @retval FSP_SUCCESS              Successfully started.
 * @retval FSP_ERR_TIMEOUT          Communication is timeout.
 * @retval FSP_ERR_ABORTED          Communication is aborted.
 **********************************************************************************************************************/
static fsp_err_t rm_icm42670_software_reset (rm_icm42670_ctrl_t * const p_api_ctrl)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;

    /* Set the data */
    p_ctrl->buf[0] = RM_ICM42670_REG_SIGNAL_PATH_RESET;
    p_ctrl->buf[1] = RM_ICM42670_COMMAND_SOFTWARE_RESET;

    /* Software reset */
    err = rm_icm42670_write(p_ctrl, &p_ctrl->buf[0], 2);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief clock select:always use internal RC oscillator.
 *
 * @retval FSP_SUCCESS              Successfully started.
 * @retval FSP_ERR_TIMEOUT          Communication is timeout.
 * @retval FSP_ERR_ABORTED          Communication is aborted.
 **********************************************************************************************************************/
static fsp_err_t rm_icm42670_clk_sel (rm_icm42670_ctrl_t * const p_api_ctrl)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;

    /* Set the data */
    p_ctrl->buf[0] = RM_ICM42670_REG_INTF_CONFIG1;
    p_ctrl->buf[1] = RM_ICM42670_SELECT_RC_OSC;

    /* write clk_sel to register */
    err = rm_icm42670_write(p_ctrl, &p_ctrl->buf[0], 2);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}
/*******************************************************************************************************************//**
 * @brief clock select:always use internal RC oscillator.
 *
 * @retval FSP_SUCCESS              Successfully started.
 * @retval FSP_ERR_TIMEOUT          Communication is timeout.
 * @retval FSP_ERR_ABORTED          Communication is aborted.
 **********************************************************************************************************************/
static fsp_err_t rm_icm42670_who_am_I (rm_icm42670_ctrl_t * const p_api_ctrl)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t  * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;
    rm_comms_write_read_params_t write_read_params;

    /* Clear all interrupt bits */
    p_ctrl->register_address     = RM_ICM42670_REG_WHO_AM_I;
    write_read_params.p_src      = &p_ctrl->register_address;
    write_read_params.src_bytes  = 1;
    write_read_params.p_dest     = &p_ctrl->buf[0];
    write_read_params.dest_bytes = 1;
    err = rm_icm42670_read(p_ctrl, write_read_params);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    if ((p_ctrl->buf[0] != RM_ICM42670_WHO_AM_I_ICM42671) && (p_ctrl->buf[0] != RM_ICM42670_WHO_AM_I_ICM42670))
    {
        err = FSP_ERR_INVALID_DATA;
    }
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}
/*******************************************************************************************************************//**
 * @brief clock select:always use internal RC oscillator.
 *
 * @retval FSP_SUCCESS              Successfully started.
 * @retval FSP_ERR_TIMEOUT          Communication is timeout.
 * @retval FSP_ERR_ABORTED          Communication is aborted.
 **********************************************************************************************************************/
static fsp_err_t rm_icm42670_mclk_rdy (rm_icm42670_ctrl_t * const p_api_ctrl)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_icm42670_instance_ctrl_t  * p_ctrl = (rm_icm42670_instance_ctrl_t *) p_api_ctrl;
    rm_comms_write_read_params_t write_read_params;
    uint8_t counter = 0;

    while ((p_ctrl->buf[0] &0x01) != 0x01)
    {
        rm_icm42670_delay_ms(p_ctrl, 1);
        counter++;
        //FSP_ERROR_RETURN(RM_ICM42670_TIMEOUT >= counter, FSP_ERR_TIMEOUT);
        FSP_ERROR_RETURN(RM_ICM42670_TIMEOUT >= counter, FSP_SUCCESS);
        /* read mclk value */
        p_ctrl->register_address     = RM_ICM42670_REG_MCLK_RDY;
        write_read_params.p_src      = &p_ctrl->register_address;
        write_read_params.src_bytes  = 1;
        write_read_params.p_dest     = &p_ctrl->buf[0];
        write_read_params.dest_bytes = 1;
        err = rm_icm42670_read(p_ctrl, write_read_params);
        FSP_ERROR_RETURN(FSP_SUCCESS == err, err);
        rm_icm42670_read(p_ctrl, write_read_params);
    }
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    return FSP_SUCCESS;
}
/*******************************************************************************************************************//**
 * @brief ICM42670 callback function called in the I2C Communications user callback function.
 **********************************************************************************************************************/
void icm42670_comms_i2c_callback (rm_icm42670_callback_args_t * p_args)
{
    if (RM_ICM42670_EVENT_ERROR != p_args->event)
    {
        g_i2c_flag = true;
    }
    else
    {
        g_i2c_flag = false;
    }
}
