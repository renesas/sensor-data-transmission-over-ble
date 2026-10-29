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
 * @ingroup RENESAS_INTERFACES
 * @defgroup RM_ICM42670_API ICM42670 Middleware Interface
 * @brief Interface for ICM42670 Middleware functions.
 *
 * @section RM_ICM42670_API_Summary Summary
 * The ICM42670 interface provides ICM42670 functionality.
 *
 * The ICM42670 interface can be implemented by:
 * - @ref RM_ICM42670
 *
 * @{
 **********************************************************************************************************************/

#ifndef RM_ICM42670_API_H_
#define RM_ICM42670_API_H_

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
 #include "bsp_api.h"
 #include "r_external_irq_api.h"

  #include    "FreeRTOS.h"
  #include    "task.h"
  #include    "queue.h"
  #include    "semphr.h"

#include "rm_comms_api.h"

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
#define ACCEL_DATA_BYTES   6
#define GYRO_DATA_BYTES   6
#define TEMP_DATA_BYTES   2

/** Event in the callback function */
typedef enum e_rm_icm42670_event
{
    RM_ICM42670_EVENT_SUCCESS = 0,
    RM_ICM42670_EVENT_ERROR,
    RM_ICM42670_EVENT_MEASUREMENT_COMPLETE,
    RM_ICM42670_EVENT_ACCEL_LPMODE_COMPLETE,
} rm_icm42670_event_t;

/** Operation mode of ICM42670 */
typedef enum e_rm_icm42670_operation_mode
{
    RM_ICM42670_OPERATION_MODE_SLEEP = 0, ///< Sleep
    RM_ICM42670_OPERATION_MODE_STANDBY, ///< Standby, Gyro Drive On
    RM_ICM42670_OPERATION_MODE_ACCEL_LP,       ///< Accel LP mode
    RM_ICM42670_OPERATION_MODE_ACCEL_LN,       ///< Accel LN mode
    RM_ICM42670_OPERATION_MODE_GYRO_LN,   ///< Gyrocope LN mode
    RM_ICM42670_OPERATION_MODE_6AXIS,         ///< 6-Axis Low-Noise Mode
} rm_icm42670_operation_mode_t;

/** Operation interrupt type of ICM42670 */
typedef enum e_rm_icm42670_operation_interrupt_type
{
    RM_ICM42670_OPERATION_INTERRUPT_TYPE_INT1 = 0, ///< int1
    RM_ICM42670_OPERATION_INTERRUPT_TYPE_INT2, ///< int2
    RM_ICM42670_OPERATION_INTERRUPT_TYPE_INT1_INT2,       ///< int1 and int2
} rm_icm42670_operation_interrupt_type_t;

/** Icm42670 interrupt mode */
typedef enum e_rm_icm42670_interrupt_polarity
{
    RM_ICM42670_INTERRUPT_POLARITY_LOW = 0x00, ///< ICM42670  interrupt polarity:low.
    RM_ICM42670_INTERRUPT_POLARITY_HIGH = 0x01, ///< ICM42670  interrupt polarity:high.
} rm_icm42670_interrupt_polarity_t;

/** Icm42670 interrupt mode */
typedef enum e_rm_icm42670_interrupt_drive_circuit
{
    RM_ICM42670_INTERRUPT_DRIVE_CIRCUIT_OPENDRAIN = 0x00, ///< ICM42670  interrupt drive circuit:open drain.
    RM_ICM42670_INTERRUPT_DRIVE_CIRCUIT_PUSHPULL = 0x02, ///< ICM42670  interruptdrive circuit:push pull.
} rm_icm42670_interrupt_drive_circuit_t;

/** Icm42670 interrupt mode */
typedef enum e_rm_icm42670_interrupt_mode
{
    RM_ICM42670_INTERRUPT_MODE_PULSE = 0x00, ///< ICM42670 pulse mode interrupt.
    RM_ICM42670_INTERRUPT_MODE_LATCHED = 0x04, ///< ICM42670 latched mode interrupt.
} rm_icm42670_interrupt_mode_t;

/** Icm42670 interrupt mode */
typedef enum e_rm_icm42670_interrupt_config_mask
{
    RM_ICM42670_INTERRUPT_CONFIG_MASK_INT1 = 0x02, ///< INT_CONFIG OF INT1 and INT2, mask of int1.
    RM_ICM42670_INTERRUPT_CONFIG_MASK_INT2 = 0xC8, ///< INT_CONFIG OF INT1 and INT2, mask of int2.
    RM_ICM42670_INTERRUPT_CONFIG_MASK_INT1_INT2 = 0xCF, ///< INT_CONFIG OF INT1 and INT2, mask of int1&int2.
} rm_icm42670_interrupt_config_mask_t;


/** Icm42670 INTF_CONFIG1:clksel */
typedef enum e_rm_icm42670_clksel
{
    RM_ICM42670_SELECT_RC_OSC = 0x00, ///< Always select internal RC oscillator.
    RM_ICM42670_SELECT_PLL_ELSE_RC = 0x01, ///<  Select PLL when available, else select RC oscillator (default).
    RM_ICM42670_DISABLE_ALL = 0x11, ///<  Disable all clocks.
}rm_icm42670_clksel_t;

/** Icm42670 soft reset */
typedef enum e_rm_icm42670_soft_reset
{
    RM_ICM42670_SOFT_RESET_DISABLE = 0x00, ///< Software reset not enabled.
    RM_ICM42670_SOFT_RESET_ENABLE = 0x01, ///<  Software reset enabled.
}rm_icm42670_soft_reset_t;

/** Icm42670 interrupt mode */
typedef enum e_rm_icm42670_pwr_rc_idle
{
    RM_ICM42670_PWR_RC_ON = 0x00, ///< when Accel and Gyro are powered off, the chip will go to OFF state, since the RC oscillator will also be powered off.
    RM_ICM42670_PWR_RC_OFF = 0x01, ///<  the RC oscillator is powered on even if Accel and Gyro are powered off.
} rm_icm42670_pwr_rc_idle_t;

/**  interrupt source0-source4 */
typedef enum e_rm_icm42670_interrupt_source
{
    RM_ICM42670_INTERRUPT_SOURCE_AGC_RDY_INT1_EN    = 0x00000001, ///< UI AGC ready interrupt routed to INT1.
    RM_ICM42670_INTERRUPT_SOURCE_FIFO_FULL_INT1_EN  = 0x00000002, ///< FIFO full interrupt routed to INT1.
    RM_ICM42670_INTERRUPT_SOURCE_FIFO_THS_INT1_EN   = 0x00000004, ///<  FIFO threshold interrupt routed to INT1.
    RM_ICM42670_INTERRUPT_SOURCE_DRDY_INT1_EN       = 0x00000008, ///< Data Ready interrupt routed to INT1.
    RM_ICM42670_INTERRUPT_SOURCE_RESET_DONE_INT1_EN = 0x00000010, ///< Reset done interrupt routed to INT1.
    RM_ICM42670_INTERRUPT_SOURCE_PLL_RDY_INT1_EN    = 0x00000020, ///< PLL ready interrupt routed to INT1.
    RM_ICM42670_INTERRUPT_SOURCE_FSYNC_INT1_EN      = 0x00000040, ///< FSYNC interrupt routed to INT1.
    RM_ICM42670_INTERRUPT_SOURCE_ST_INT1_EN         = 0x00000080, ///< Self-Test Done interrupt routed to INT1.

    RM_ICM42670_INTERRUPT_SOURCE_WOM_X_INT1_EN = 0x00000100, ///< X-axis WOM interrupt routed to INT1.
    RM_ICM42670_INTERRUPT_SOURCE_WOM_Y_INT1_EN = 0x00000200, ///< Y-axis WOM interrupt routed to INT1.
    RM_ICM42670_INTERRUPT_SOURCE_WOM_Z_INT1_EN = 0x00000400, ///<  Z-axis WOM interrupt routed to INT1.
    RM_ICM42670_INTERRUPT_SOURCE_SMD_INT1_EN   = 0x00000800, ///< SMD interrupt routed to INT1.

    RM_ICM42670_INTERRUPT_SOURCE_AGC_RDY_INT2_EN    = 0x00010000, ///< UI AGC ready interrupt routed to INT2.
    RM_ICM42670_INTERRUPT_SOURCE_FIFO_FULL_INT2_EN  = 0x00020000, ///< FIFO full interrupt routed to INT2.
    RM_ICM42670_INTERRUPT_SOURCE_FIFO_THS_INT2_EN   = 0x00040000, ///<  FIFO threshold interrupt routed to INT2.
    RM_ICM42670_INTERRUPT_SOURCE_DRDY_INT2_EN       = 0x00080000, ///< Data Ready interrupt routed to INT2.
    RM_ICM42670_INTERRUPT_SOURCE_RESET_DONE_INT2_EN = 0x00100000, ///< Reset done interrupt routed to INT2.
    RM_ICM42670_INTERRUPT_SOURCE_PLL_RDY_INT2_EN    = 0x00200000, ///< PLL ready interrupt routed to INT2.
    RM_ICM42670_INTERRUPT_SOURCE_FSYNC_INT2_EN      = 0x00400000, ///< FSYNC interrupt routed to INT2.
    RM_ICM42670_INTERRUPT_SOURCE_ST_INT2_EN         = 0x00800000, ///< Self-Test Done interrupt routed to INT2.

    RM_ICM42670_INTERRUPT_SOURCE_WOM_X_INT2_EN = 0x01000000, ///< X-axis WOM interrupt routed to INT2.
    RM_ICM42670_INTERRUPT_SOURCE_WOM_Y_INT2_EN = 0x02000000, ///< Y-axis WOM interrupt routed to INT2.
    RM_ICM42670_INTERRUPT_SOURCE_WOM_Z_INT2_EN = 0x04000000, ///<  Z-axis WOM interrupt routed to INT2.
    RM_ICM42670_INTERRUPT_SOURCE_SMD_INT2_EN   = 0x08000000, ///< SMD interrupt routed to INT2.
} rm_icm42670_interrupt_source_t;

/**  Data Ready Interrupt Clear Option (latched mode) */
typedef enum e_rm_icm42670_interrupt_int_clear
{
    RM_ICM42670_INTERRUPT_CLEARON_STATUSBIT_DEFAULT = 0x00, ///< Default clear on Status Bit Read.

    RM_ICM42670_INTERRUPT_FIFOFULL_CLEARON_STATUSBIT = 0x01, ///< FIFO_FULL Clear on Status Bit Read.
    RM_ICM42670_INTERRUPT_FIFOFULL_CLEARON_SENSORREGISTER = 0x02, ///< FIFO_FULL Clear on Sensor Register Read.
    RM_ICM42670_INTERRUPT_FIFOFULL_CLEARON_STATUSBIT_OR_SENSORREGISTER = 0x03, ///<  FIFO_FULL Clear on Status Bit Read OR on Sensor Register read.

    RM_ICM42670_INTERRUPT_FIFOTHS_CLEARON_STATUSBIT = 0x04, ///< FIFO_THS Clear on Status Bit Read.
    RM_ICM42670_INTERRUPT_FIFOTHS_CLEARON_SENSORREGISTER = 0x08, ///< FIFO_THS Clear on Sensor Register Read.
    RM_ICM42670_INTERRUPT_FIFOTHS_CLEARON_STATUSBIT_OR_SENSORREGISTER = 0x0C, ///<  FIFO_THS Clear on Status Bit Read OR on Sensor Register read.

    RM_ICM42670_INTERRUPT_UIDRDY_CLEARON_STATUSBIT = 0x10, ///< Clear on Status Bit Read.
    RM_ICM42670_INTERRUPT_UIDRDY_CLEARON_SENSORREGISTER = 0x20, ///< Clear on Sensor Register Read.
    RM_ICM42670_INTERRUPT_UIDRDY_CLEARON_STATUSBIT_OR_SENSORREGISTER = 0x30, ///<  Clear on Status Bit Read OR on Sensor Register read.
} rm_icm42670_interrupt_int_clear_t;

/** Accel sensor mode of ICM42670 */
typedef enum e_rm_icm42670_accel_sensor_mode
{
    RM_ICM42670_ACCEL_SENSOR_MODE_OFF = 0x00, ///< Turns accelerometer off
    RM_ICM42670_ACCEL_SENSOR_MODE_LP = 0x02, ///< Places accelerometer in Low Power (LP) Mode
    RM_ICM42670_ACCEL_SENSOR_MODE_LN = 0x03, ///< Places accelerometer in Low Noise (LN) Mode
} rm_icm42670_accel_sensor_mode_t;

/** Accelerometer LP mode clk select */
typedef enum e_rm_icm42670_accel_lp_clk_sel
{
    RM_ICM42670_ACCEL_CLK_SEL_WAKEUP_OSC = 0x00, ///< Accelerometer LP mode uses Wake Up oscillator clock(recommended setting).
    RM_ICM42670_ACCEL_CLK_SEL_RC_OSC = 0x01, ///< Accelerometer LP mode uses RC oscillator clock
} rm_icm42670_accel_lp_clk_sel_t;

/** Gyro sensor mode of ICM42670 */
typedef enum e_rm_icm42670_gyro_sensor_mode
{
    RM_ICM42670_GYRO_SENSOR_MODE_OFF = 0x00, ///< Turns gyrocope off
    RM_ICM42670_GYRO_SENSOR_MODE_STB = 0x01, ///< Places gyrocope in Standby Mode
    RM_ICM42670_GYRO_SENSOR_MODE_LN = 0x03, ///< Places gyrocope in Low Noise (LN) Mode
} rm_icm42670_gyro_sensor_mode_t;

/** Selects GYRO UI low pass filter bandwidth of ICM42670 */
typedef enum e_rm_icm42670_gyro_ui_filt_bw
{
    RM_ICM42670_GYRO_UI_FILT_BYPASSED = 0x00, ///< 000: Low pass filter bypassed
    RM_ICM42670_GYRO_UI_FILT_BW_180Hz = 0x01, ///< 001: 180Hz
    RM_ICM42670_GYRO_UI_FILT_BW_121Hz = 0x02, ///< 010: 121Hz
    RM_ICM42670_GYRO_UI_FILT_BW_73Hz = 0x03, ///< 011: 73Hz
    RM_ICM42670_GYRO_UI_FILT_BW_53Hz = 0x04, ///< 100: 53Hz
    RM_ICM42670_GYRO_UI_FILT_BW_34Hz = 0x05, ///< 101: 34Hz
    RM_ICM42670_GYRO_UI_FILT_BW_25Hz = 0x06, ///< 110: 25Hz
    RM_ICM42670_GYRO_UI_FILT_BW_16Hz = 0x07, ///< 111: 16Hz
} rm_icm42670_gyro_ui_filt_bw_t;

/** Selects ACCEL UI low pass filter bandwidth of ICM42670 */
typedef enum e_rm_icm42670_accel_ui_filt_bw
{
    RM_ICM42670_ACCEL_UI_FILT_BYPASSED = 0x00, ///< 000: Low pass filter bypassed
    RM_ICM42670_ACCEL_UI_FILT_BW_180Hz = 0x01, ///< 001: 180Hz
    RM_ICM42670_ACCEL_UI_FILT_BW_121Hz = 0x02, ///< 010: 121Hz
    RM_ICM42670_ACCEL_UI_FILT_BW_73Hz = 0x03, ///< 011: 73Hz
    RM_ICM42670_ACCEL_UI_FILT_BW_53Hz = 0x04, ///< 100: 53Hz
    RM_ICM42670_ACCEL_UI_FILT_BW_34Hz = 0x05, ///< 101: 34Hz
    RM_ICM42670_ACCEL_UI_FILT_BW_25Hz = 0x06, ///< 110: 25Hz
    RM_ICM42670_ACCEL_UI_FILT_BW_16Hz = 0x07, ///< 111: 16Hz
} rm_icm42670_accel_ui_filt_bw_t;

/** Sets the bandwidth of the temperature signal DLPF of ICM42670 */
typedef enum e_rm_icm42670_temp_filt_bw
{
    RM_ICM42670_TEMP_DLPF_BYPASSED = 0x00, ///< 000: DLPF bypassed  (BIT 6:4)
    RM_ICM42670_TEMP_DLPF_BW_180Hz = 0x10, ///< 001: DLPF BW = 180Hz
    RM_ICM42670_TEMP_DLPF_BW_72Hz = 0x20, ///< 001: DLPF BW = 72Hz
    RM_ICM42670_TEMP_DLPF_BW_34Hz = 0x30, ///< 001: DLPF BW = 34Hz
    RM_ICM42670_TEMP_DLPF_BW_16Hz = 0x40, ///< 001: DLPF BW = 16Hz
    RM_ICM42670_TEMP_DLPF_BW_8Hz = 0x50, ///< 001: DLPF BW = 8Hz
    RM_ICM42670_TEMP_DLPF_BW_4Hz = 0x60, ///< 001: DLPF BW = 4Hz
} rm_icm42670_temp_filt_bw_t;

/***Selects averaging filter setting to create accelerometer output in accelerometer low power mode (LPM)***/
typedef enum e_rm_icm42670_accel_ui_avg
{
    RM_ICM42670_ACCEL_UI_AVG_2x  = 0x00, ///< 000: 2x average(BIT 6:4)
    RM_ICM42670_ACCEL_UI_AVG_4x  = 0x10, ///< 001: 4x average
    RM_ICM42670_ACCEL_UI_AVG_8x  = 0x20, ///< 010:  8x average
    RM_ICM42670_ACCEL_UI_AVG_16x = 0x30, ///< 011:  16x average
    RM_ICM42670_ACCEL_UI_AVG_32x = 0x40, ///< 100:  32x average
    RM_ICM42670_ACCEL_UI_AVG_64x = 0x50, ///< 101:  64x average
    //RM_ICM42670_ACCEL_UI_AVG_64x = 0x60, ///< 110:  64x average
    //RM_ICM42670_ACCEL_UI_AVG_64x = 0x70, ///< 111:  64x average
} rm_icm42670_accel_ui_avg_t;



/** Data type of Accel */
typedef enum e_rm_icm42670_accel_data_type
{
    RM_ICM42670_ACCEL_DATA_TYPE_ALL = 0, ///< Common
    RM_ICM42670_ACCEL_DATA_TYPE_X1,   ///< X1
    RM_ICM42670_ACCEL_DATA_TYPE_X0,   ///< X0
    RM_ICM42670_ACCEL_DATA_TYPE_Y1,    ///< Y1
    RM_ICM42670_ACCEL_DATA_TYPE_Y0,    ///< Y0
    RM_ICM42670_ACCEL_DATA_TYPE_Z1,   ///< Z1
    RM_ICM42670_ACCEL_DATA_TYPE_Z0,   ///< Z0
} rm_icm42670_accel_data_type_t;


/** PPG interrupt type */
typedef enum e_rm_icm42670_ppg_interrupt_type
{
    RM_ICM42670_PPG_INTERRUPT_TYPE_DATA       = 0x10, ///< PPG data interrupt.
    RM_ICM42670_PPG_INTERRUPT_TYPE_FIFO_AFULL = 0x20, ///< PPG FIFO almost full interrupt.
} rm_icm42670_ppg_interrupt_type_t;

/** Variance threshold */
typedef enum e_rm_icm42670_variance_threshold
{
    RM_ICM42670_VARIANCE_THRESHOLD_8_COUNTS    = 0x00, ///< New LS_DATA varies by ± 8 counts compared to previous result.
    RM_ICM42670_VARIANCE_THRESHOLD_16_COUNTS   = 0x01, ///< New LS_DATA varies by ± 16 counts compared to previous result.
    RM_ICM42670_VARIANCE_THRESHOLD_32_COUNTS   = 0x02, ///< New LS_DATA varies by ± 32 counts compared to previous result.
    RM_ICM42670_VARIANCE_THRESHOLD_64_COUNTS   = 0x03, ///< New LS_DATA varies by ± 64 counts compared to previous result.
    RM_ICM42670_VARIANCE_THRESHOLD_128_COUNTS  = 0x04, ///< New LS_DATA varies by ± 128 counts compared to previous result.
    RM_ICM42670_VARIANCE_THRESHOLD_256_COUNTS  = 0x05, ///< New LS_DATA varies by ± 256 counts compared to previous result.
    RM_ICM42670_VARIANCE_THRESHOLD_512_COUNTS  = 0x06, ///< New LS_DATA varies by ± 512 counts compared to previous result.
    RM_ICM42670_VARIANCE_THRESHOLD_1024_COUNTS = 0x07, ///< New LS_DATA varies by ± 1024 counts compared to previous result.
} rm_icm42670_variance_threshold_t;

/** Sleep after interrupt */
typedef enum e_rm_icm42670_sleep_after_interrupt
{
    RM_ICM42670_SLEEP_AFTER_INTERRUPT_DISABLE = 0x00, ///< Disable sleep after interrupt.
    RM_ICM42670_SLEEP_AFTER_INTERRUPT_ENABLE  = 0x08, ///< Stop measurement after an interrupt occurs. After STATUS_0/STATUS_1 register is read, start measurement.
} rm_icm42670_sleep_after_interrupt_t;

/** Moving average */
typedef enum e_rm_icm42670_moving_average
{
    RM_ICM42670_MOVING_AVERAGE_DISABLE = 0x00, ///< Moving average is disabled for Gyrocope mode
    RM_ICM42670_MOVING_AVERAGE_ENABLE  = 0x80, ///< Moving average is enabled for Gyrocope mode. Gyrocope data is the average of the current and previous measurement. The moving average is applied after digital offset cancellation.
} rm_icm42670_moving_average_t;

/** Power save mode */
typedef enum e_rm_icm42670_power_save_mode
{
    RM_ICM42670_POWER_SAVE_MODE_DISABLE = 0x00, ///< Power save mode is disabled for PPG mode.
    RM_ICM42670_POWER_SAVE_MODE_ENABLE  = 0x40, ///< Power save mode is enabled for PPG mode. On power save mode, some analog circuitry powers down between individual PPG measurements if the idle time.
} rm_icm42670_power_save_mode_t;

/** Analog cancellation */
typedef enum e_rm_icm42670_analog_cancellation
{
    RM_ICM42670_ANALOG_CANCELLATION_DISABLE = 0x00, ///< No offset cancellation
    RM_ICM42670_ANALOG_CANCELLATION_ENABLE  = 0x01, ///< 50% offset of the full-scale value
} rm_icm42670_analog_cancellation_t;

/** Number of LED pulses */
typedef enum e_rm_rm_icm42670_number_led_pulses
{
    RM_ICM42670_NUM_LED_PULSES_1  = 0x00, ///< 1 pulse.
    RM_ICM42670_NUM_LED_PULSES_2  = 0x08, ///< 2 pulses.
    RM_ICM42670_NUM_LED_PULSES_4  = 0x10, ///< 4 pulses.
    RM_ICM42670_NUM_LED_PULSES_8  = 0x18, ///< 8 pulses.
    RM_ICM42670_NUM_LED_PULSES_16 = 0x20, ///< 16 pulses.
    RM_ICM42670_NUM_LED_PULSES_32 = 0x28, ///< 32 pulses.
} rm_icm42670_number_led_pulses_t;

/** Number of averaged samples */
typedef enum e_rm_icm42670_number_averaged_samples
{
    RM_ICM42670_NUM_AVERAGED_SAMPLES_1  = 0x00, ///< 1 (No averaging).
    RM_ICM42670_NUM_AVERAGED_SAMPLES_2  = 0x10, ///< 2 consecutives samples are averaged.
    RM_ICM42670_NUM_AVERAGED_SAMPLES_4  = 0x20, ///< 4 consecutives samples are averaged.
    RM_ICM42670_NUM_AVERAGED_SAMPLES_8  = 0x30, ///< 8 consecutives samples are averaged.
    RM_ICM42670_NUM_AVERAGED_SAMPLES_16 = 0x40, ///< 16 consecutives samples are averaged.
    RM_ICM42670_NUM_AVERAGED_SAMPLES_32 = 0x50, ///< 32 consecutives samples are averaged.
} rm_icm42670_number_averaged_samples_t;

/** Accel Full-Scale Range: See datasheet section 3.2 for details */
typedef enum e_rm_icm42670_accel_fs
{
    RM_ICM42670_ACCEL_FS_16G  = 0x00, ///< ACCEL_UI_FS_SEL=0:2048.
    RM_ICM42670_ACCEL_FS_8G   = 0x01, ///< ACCEL_UI_FS_SEL=1:4096.
    RM_ICM42670_ACCEL_FS_4G   = 0x02, ///< ACCEL_UI_FS_SEL=2:8192.
    RM_ICM42670_ACCEL_FS_2G   = 0x03, ///< ACCEL_UI_FS_SEL=3:16384.
} rm_icm42670_accel_fs_t;

/** Accel Sensitivity Scale Factor: See datasheet section 3.2 for details */
typedef enum e_rm_icm42670_accel_sensitivity_scale_factor
{
    RM_ICM42670_ACCEL_SENSITIVITY_SCALE_FACTOR_0   = 0x800, ///< ACCEL_UI_FS_SEL=0:2048.
    RM_ICM42670_ACCEL_SENSITIVITY_SCALE_FACTOR_1   = 0x1000, ///< ACCEL_UI_FS_SEL=1:4096.
    RM_ICM42670_ACCEL_SENSITIVITY_SCALE_FACTOR_2   = 0x2000, ///< ACCEL_UI_FS_SEL=2:8192.
    RM_ICM42670_ACCEL_SENSITIVITY_SCALE_FACTOR_3   = 0x4000, ///< ACCEL_UI_FS_SEL=3:16384.
} rm_icm42670_accel_sensitivity_scale_factor_t;

/** Accel Full-Scale Range: See datasheet section 3.2 for details */
typedef enum e_rm_icm42670_accel_odr
{
    RM_ICM42670_ACCEL_ODR_1600Hz = 0x05, ///< ACCEL_ODR: 1.6kHz,LN Mode.
    RM_ICM42670_ACCEL_ODR_800Hz  = 0x06, ///< ACCEL_ODR: 800Hz,LN Mode.
    RM_ICM42670_ACCEL_ODR_400Hz  = 0x07, ///< ACCEL_ODR: 400Hz,LP or LN mode.
    RM_ICM42670_ACCEL_ODR_200Hz  = 0x08, ///< ACCEL_ODR: 200Hz,LP or LN mode.
    RM_ICM42670_ACCEL_ODR_100Hz  = 0x09, ///< ACCEL_ODR: 100Hz,LP or LN mode.
    RM_ICM42670_ACCEL_ODR_50Hz   = 0x0A, ///< ACCEL_ODR: 50Hz,LP or LN mode.
    RM_ICM42670_ACCEL_ODR_25Hz   = 0x0B, ///< ACCEL_ODR: 25Hz,LP or LN mode.
    RM_ICM42670_ACCEL_ODR_12Hz   = 0x0C, ///< ACCEL_ODR: 12.5Hz,LP or LN mode.
    RM_ICM42670_ACCEL_ODR_6Hz    = 0x0D, ///< ACCEL_ODR: 6.25Hz,LP mode
    RM_ICM42670_ACCEL_ODR_3Hz    = 0x0E, ///< ACCEL_ODR: 3.125Hz,LP mode.
    RM_ICM42670_ACCEL_ODR_1Hz    = 0x0F, ///< ACCEL_ODR: 1.5625Hz,LP mode.
} rm_icm42670_accel_odr_t;

/** Accel Full-Scale Range: See datasheet section 3.2 for details */
typedef enum e_rm_icm42670_gyro_fs
{
    RM_ICM42670_GYRO_FS_16G  = 0x00, ///< GYRO_UI_FS_SEL=0:2000dps.
    RM_ICM42670_GYRO_FS_8G   = 0x01, ///< GYRO_UI_FS_SEL=1:1000dps.
    RM_ICM42670_GYRO_FS_4G   = 0x02, ///< GYRO_UI_FS_SEL=2:500dps.
    RM_ICM42670_GYRO_FS_2G   = 0x03, ///< GYRO_UI_FS_SEL=3:250dps.
} rm_icm42670_gyro_fs_t;

/** Gyro Sensitivity Scale Factor: See datasheet section 3.1 for details */
typedef enum e_rm_icm42670_gyro_sensitivity_scale_factor
{
    RM_ICM42670_GYRO_SENSITIVITY_SCALE_FACTOR_0   = 164, ///< GYRO_UI_FS_SEL=0: 16.4.
    RM_ICM42670_GYRO_SENSITIVITY_SCALE_FACTOR_1   = 328, ///< GYRO_UI_FS_SEL=1: 32.8.
    RM_ICM42670_GYRO_SENSITIVITY_SCALE_FACTOR_2   = 655, ///< GYRO_UI_FS_SEL=2: 65.5.
    RM_ICM42670_GYRO_SENSITIVITY_SCALE_FACTOR_3   = 131, ///< GYRO_UI_FS_SEL=3: 13.1.
} rm_icm42670_gyro_sensitivity_scale_factor_t;

/** Accel Full-Scale Range: See datasheet section 3.2 for details */
typedef enum e_rm_icm42670_gyro_odr
{
    RM_ICM42670_GYRO_ODR_1600Hz = 0x05, ///< GYRO_ODR: 1.6kHz.
    RM_ICM42670_GYRO_ODR_800Hz  = 0x06, ///< GYRO_ODR: 800Hz.
    RM_ICM42670_GYRO_ODR_400Hz  = 0x07, ///< GYRO_ODR: 400Hz.
    RM_ICM42670_GYRO_ODR_200Hz  = 0x08, ///< GYRO_ODR: 200Hz.
    RM_ICM42670_GYRO_ODR_100Hz  = 0x09, ///< GYRO_ODR: 100Hz.
    RM_ICM42670_GYRO_ODR_50Hz   = 0x0A, ///< GYRO_ODR: 50Hz.
    RM_ICM42670_GYRO_ODR_25Hz   = 0x0B, ///< GYRO_ODR: 25Hz.
    RM_ICM42670_GYRO_ODR_12Hz   = 0x0C, ///< GYRO_ODR: 12.5Hz.
} rm_icm42670_gyro_odr_t;

/** FIFO Rollover */
typedef enum e_rm_icm42670_fifo_rollover
{
    RM_ICM42670_FIFO_ROLLOVER_DISABLE = 0x00, ///< In the event of a full FIFO, no more samples of PPG data are written into the FIFO; the samples from new measurements are lost.
    RM_ICM42670_FIFO_ROLLOVER_ENABLE  = 0x10, ///< New PPG data will always be written to the FIFO, and the FIFO Write Pointer is incremented (rollover). If the FIFO is full, old data will be overwritten. The FIFO Overflow Counter counts the number of lost (overwritten) and respectively the number of new samples. The FIFO Read Pointer remains unchanged.
} rm_icm42670_fifo_rollover_t;

/* Semaphore Structure */
typedef struct st_rm_icm42670_semaphore
{
    SemaphoreHandle_t * p_semaphore_handle;
    StaticSemaphore_t * p_semaphore_memory;
} rm_icm42670_semaphore_t;

/** ICM42670 callback parameter definition */
typedef struct st_rm_icm42670_callback_args
{
    void const      * p_context;
    rm_icm42670_event_t event;
} rm_icm42670_callback_args_t;

/** ICM42670 raw data structure */
typedef struct st_rm_icm42670_raw_data
{
    uint8_t reg_data[20];              ///< data read from sensor register
} rm_icm42670_raw_data_t;

/** ICM42670 accel data structure */
typedef struct st_rm_icm42670_accel_data
{
    float accel_x;               ///< X_Axis Accelerometer data (16bits).
    float accel_y;               ///< Y_Axis Accelerometer data (16bits).
    float accel_z;                ///< Z_Axis Accelerometer data (16bits).
} rm_icm42670_accel_data_t;

/** ICM42670 accel data structure */
typedef struct st_rm_icm42670_gyro_data
{
    float gyro_x;               ///< X_Axis Gyrocope data (16bits).
    float gyro_y;               ///< Y_Axis Gyrocope data (16bits).
    float gyro_z;                ///< Z_Axis Gyrocope data (16bits).
} rm_icm42670_gyro_data_t;

/** ICM42670 temp data structure */
typedef struct st_rm_icm42670_temp_data
{
    int16_t temp_data;                    ///< Temperature compensation (Comp) channel data (16bits).
    float temp_data_float;                ///< Temperature compensation (Comp) channel data (float).
} rm_icm42670_temp_data_t;

/** ICM42670 accel data structure */
typedef struct st_rm_icm42670_accel_gyro_fifo_data
{
    uint8_t header;                 ///< header (8bits).
    uint16_t accel_x;               ///< X_Axis Accelerometer data (16bits).
    uint16_t accel_y;               ///< Y_Axis Accelerometer data (16bits).
    uint16_t accel_z;                ///< Z_Axis Accelerometer data (16bits).
    uint16_t gyro_x;               ///< X_Axis Gyrocope data (16bits).
    uint16_t gyro_y;               ///< Y_Axis Gyrocope data (16bits).
    uint16_t gyro_z;                ///< Z_Axis Gyrocope data (16bits).
    uint16_t comp_data;              ///< Temperature compensation (Comp) channel data (16bits).
} rm_icm42670_accel_gyro_fifo_data_t;

/** ICM42670 device interrupt configuration structure */
typedef struct st_rm_icm42670_device_interrupt_cfg
{
    rm_icm42670_operation_interrupt_type_t     int_type;     ///< interrupt type: INT1, INT2, INT1&INT2
    rm_icm42670_interrupt_source_t       int_source;   ///<  interrupt source.
    uint8_t       int_config;   ///< INT_CONFIG (register 0x06 of Bank0, include mode,drive chain,polarity of INT1 and INT2.
    //rm_icm42670_interrupt_polarity_t     int_polarity; ///< interrupt polarity(in register 0x06 of Bank0)
    //rm_icm42670_interrupt_drive_circuit_t     int_drive_circuit; ///< interrupt drive circuit(in register 0x06 of Bank0)
    //rm_icm42670_interrupt_mode_t     int_mode; ///< interrupt mode(in register 0x06 of Bank0)
} rm_icm42670_device_interrupt_cfg_t;

/** ICM42670 accel configuration structure */
typedef struct st_rm_icm42670_accel_cfg
{
    uint16_t accel_odr;
    uint16_t accel_fs;
} rm_icm42670_accel_cfg_t;

/** ICM42670 gyro configuration structure */
typedef struct st_rm_icm42670_gyro_cfg
{
    uint16_t gyro_odr;
    uint16_t gyro_fs;
} rm_icm42670_gyro_cfg_t;

/** ICM42670 device status */
typedef struct st_rm_icm42670_device_status
{
    bool data_ready;

    bool agc_ready;
    bool fifo_afull_int_occur;   ///< FIFO almost full interrupt
    bool fifo_ths_int_occur;   ///< FIFO threshold interrupt
    bool power_on_reset_occur;
    bool pll_ready;
    bool FSYNC_int_occur;
    bool self_test_done;   //: Self-Test Done

    bool wom_z_occur;
    bool wom_y_occur;
    bool wom_x_occur;
    bool SMD_occur;

    bool lowG_occur;
    bool freefall_occur;
    bool tilt_detection;
    bool step_count_overflow_occur;
    bool step_detection;
} rm_icm42670_device_status_t;

/** ICM42670 FIFO information structure */
typedef struct st_rm_icm42670_fifo_info
{
    uint8_t write_index;               ///< The FIFO index where the next sample of PPG data will be written in the FIFO.
    uint8_t read_index;                ///< The index of the next sample to be read from the FIFO_DATA register.
    uint8_t overflow_counter;          ///< If the FIFO Rollover Enable bit is set, the FIFO overflow counter counts the number of old samples (up to 15) which are overwritten by new data.
    uint8_t unread_samples;            ///< The number of unread samples calculated from the write index and the read index.
} rm_icm42670_fifo_info_t;

/** ICM42670 Configuration */
typedef struct st_rm_icm42670_cfg
{
    rm_icm42670_semaphore_t const * p_semaphore;                     ///< The semaphore to wait for callback. This is used for another data read/write after a communication

    uint32_t semaphore_timeout;                                    ///< timeout for callback.
    rm_comms_instance_t const * p_comms_instance;                  ///< Pointer to Communications Middleware instance.
    void const                * p_irq_instance;                    ///< Pointer to IRQ instance.
    void const                * p_context;                         ///< Pointer to the user-provided context.
    void const                * p_extend;                          ///< Pointer to extended configuration by instance of interface.
    void (* p_comms_callback)(rm_icm42670_callback_args_t * p_args); ///< I2C Communications callback
    void (* p_irq_callback)(rm_icm42670_callback_args_t * p_args);   ///< IRQ callback
} rm_icm42670_cfg_t;

/** ICM42670 control block.  Allocate an instance specific control block to pass into the ICM42670 API calls.
 * @par Implemented as
 * - rm_icm42670_instance_ctrl_t
 */
typedef void rm_icm42670_ctrl_t;

/** ICM42670 APIs */
typedef struct st_rm_icm42670_api
{
    /** Open sensor.
     * @par Implemented as
     * - @ref RM_ICM42670_Open()
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     * @param[in]  p_cfg        Pointer to configuration structure.
     */
    fsp_err_t (* open)(rm_icm42670_ctrl_t * const p_ctrl, rm_icm42670_cfg_t const * const p_cfg);

    /** Start measurement.
     * @par Implemented as
     * - @ref RM_ICM42670_MeasurementStart()
     *
     * @param[in]  p_ctrl    Pointer to control structure.
     * @param[in]  mode      Sensor mode.
     */
    fsp_err_t (* measurementStart)(rm_icm42670_ctrl_t * const p_ctrl);

    /** Stop measurement.
     * @par Implemented as
     * - @ref RM_ICM42670_MeasurementStop()
     *
     * @param[in]  p_ctrl    Pointer to control structure.
     * @param[in]  mode      Sensor mode.
     */
    fsp_err_t (* measurementStop)(rm_icm42670_ctrl_t * const p_ctrl);

    /** Read Accel ADC data from ICM42670.
     * @par Implemented as
     * - @ref RM_ICM42670_AccelRead()
     *
     * @param[in]  p_ctrl           Pointer to control structure.
     * @param[in]  p_raw_data       Pointer to raw data structure.
     * @param[in]  type             Accel data type.
     */
    fsp_err_t (* accelRead)(rm_icm42670_ctrl_t * const p_ctrl, rm_icm42670_raw_data_t * const p_raw_data);

    /** Calculate Accel data from raw data.
     * @par Implemented as
     * - @ref RM_ICM42670_AccelDataCalculate()
     *
     * @param[in]  p_ctrl           Pointer to control structure.
     * @param[in]  p_raw_data       Pointer to raw data structure.
     * @param[in]  p_icm42670_data    Pointer to ICM42670 Accel data structure.
     */
    fsp_err_t (* accelDataCalculate)(rm_icm42670_ctrl_t * const p_ctrl, rm_icm42670_raw_data_t * const p_raw_data,
                                     rm_icm42670_accel_data_t * const p_icm42670_data);

    /** Read Gyrocope ADC data from ICM42670.
     * @par Implemented as
     * - @ref RM_ICM42670_GyroRead()
     *
     * @param[in]  p_ctrl           Pointer to control structure.
     * @param[in]  p_raw_data       Pointer to raw data structure.
     */
    fsp_err_t (* gyroRead)(rm_icm42670_ctrl_t * const p_ctrl, rm_icm42670_raw_data_t * const p_raw_data);

    /** Calculate Gyrocope data from raw data.
     * @par Implemented as
     * - @ref RM_ICM42670_GyroDataCalculate()
     *
     * @param[in]  p_ctrl           Pointer to control structure.
     * @param[in]  p_raw_data       Pointer to raw data structure.
     * @param[in]  p_icm42670_data  Pointer to ICM42670 Gyrocope data structure.
     */
    fsp_err_t (* gyroDataCalculate)(rm_icm42670_ctrl_t * const p_ctrl, rm_icm42670_raw_data_t * const p_raw_data,
                                    rm_icm42670_gyro_data_t * const p_icm42670_data);

    /** Read PPG ADC data from ICM42670.
     * @par Implemented as
     * - @ref RM_ICM42670_TempRead()
     *
     * @param[in]  p_ctrl           Pointer to control structure.
     * @param[in]  p_raw_data       Pointer to raw data structure.
     * @param[in]  number_of_samples  Number of PPG samples. One sample is 3 bytes.
     */
    fsp_err_t (* tempRead)(rm_icm42670_ctrl_t * const p_ctrl, rm_icm42670_raw_data_t * const p_raw_data);

    /** Calculate Temperature data from raw data.
     * @par Implemented as
     * - @ref RM_ICM42670_TempDataCalculate()
     *
     * @param[in]  p_ctrl           Pointer to control structure.
     * @param[in]  p_raw_data       Pointer to raw data structure.
     * @param[in]  p_icm42670_data    Pointer to ICM42670 PPG data structure.
     */
    fsp_err_t (* tempDataCalculate)(rm_icm42670_ctrl_t * const p_ctrl, rm_icm42670_raw_data_t * const p_raw_data,
                                   rm_icm42670_temp_data_t * const p_icm42670_data);

    /** Get device status. Read STATUS_0 and STATUS_1 registers.
     * @par Implemented as
     * - @ref RM_ICM42670_DeviceStatusGet()
     *
     * @param[in]  p_ctrl               Pointer to control structure.
     * @param[in]  p_status             Pointer to device status.
     */
    fsp_err_t (* deviceStatusGet)(rm_icm42670_ctrl_t * const p_ctrl, rm_icm42670_device_status_t * const p_status);

    /** Set device interrupt configuration.
     * @par Implemented as
     * - @ref RM_ICM42670_DeviceInterruptCfgSet()
     *
     * @param[in]  p_ctrl               Pointer to control structure.
     * @param[in]  interrupt_cfg        Device interrupt configuration.
     */
    fsp_err_t (* deviceInterruptCfgSet)(rm_icm42670_ctrl_t * const               p_ctrl,
                                        rm_icm42670_device_interrupt_cfg_t const interrupt_cfg);

    /** Get FIFO information.
     * @par Implemented as
     * - @ref RM_ICM42670_FifoInfoGet()
     *
     * @param[in]  p_ctrl               Pointer to control structure.
     * @param[in]  p_fifo_info          Pointer to FIFO information (write index, read index and overflow counter).
     */
    fsp_err_t (* fifoInfoGet)(rm_icm42670_ctrl_t * const p_ctrl, rm_icm42670_fifo_info_t * const p_fifo_info);

    /** Close ICM42670.
     * @par Implemented as
     * - @ref RM_ICM42670_Close()
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     */
    fsp_err_t (* close)(rm_icm42670_ctrl_t * const p_ctrl);
} rm_icm42670_api_t;

/** ICM42670 instance */
typedef struct st_rm_icm42670_instance
{
    rm_icm42670_ctrl_t * p_ctrl;    /**< Pointer to the control structure for this instance */
    rm_icm42670_cfg_t const * p_cfg;     /**< Pointer to the configuration structure for this instance */
    rm_icm42670_api_t const * p_api;     /**< Pointer to the API structure for this instance */
} rm_icm42670_instance_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * Public Function Prototypes
 **********************************************************************************************************************/

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_FOOTER

#endif                                 /* RM_ICM42670_API_H_*/

/*******************************************************************************************************************//**
 * @} (end addtogroup RM_ICM42670_API)
 **********************************************************************************************************************/
