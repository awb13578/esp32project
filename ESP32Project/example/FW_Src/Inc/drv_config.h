// /**
//  *******************************************************************************
//  * @file    drv_config.h
//  * @brief   Header file for driver configuration module.
//  *
//  * @defgroup driver_configuration_group Driver Configuration
//  * @ingroup  driver_layer_group
//  *
//  * @details Provides shared logical ID enumerations (gpio_id_e, pwm_id_e,
//  *          gpt_id_e, adc_id_e, uart_id_e, cmpss_id_e, can_id_e) used by all
//  *          driver modules to index their instance/context arrays, plus
//  *          module-wide configuration constants (e.g. debug UART period).
//  *
//  * @version 1.1.0
//  * @date    07/09/2026
//  * @author  EVSELab JSC.
//  * @{
//  *******************************************************************************
//  */

// #ifndef DRV_CONFIG_H_
// #define DRV_CONFIG_H_

// /*==================================================================================================
//  *                                        INCLUDES
//  ==================================================================================================*/
// /**
//  * @brief Include required header files.
//  */
// #include "ti_msp_dl_config.h"

// /*==================================================================================================
//  *                                        DEFINES
//  ==================================================================================================*/
// /**
//  * @brief Public constant definitions.
//  */
// // #define DEBUG_MODE
// #define DEBUG_UART_PERIOD_MS 200U /**< Debug message send period over UART (ms) */

// /*==================================================================================================
//  *                                        MACROS
//  ==================================================================================================*/
// /**
//  * @brief Public macro definitions.
//  */

// /*==================================================================================================
//  *                                   TYPE DEFINITIONS
//  ==================================================================================================*/
// /** @brief Logical GPIO channel IDs, indexing gpio_inst[]. */
// typedef enum {
//   GPIO_LLC_LATCH = 0,      /**< LLC latch output */
//   GPIO_FAN_FAULT = 1,      /**< Fan fault input */
//   GPIO_MCU_FAN = 2,        /**< MCU fan control output */
//   GPIO_LED_RED = 3,        /**< Red status LED output */
//   GPIO_LED_GRN = 4,        /**< Green status LED output */
//   GPIO_RL_CTL = 5,         /**< Relay control output */
//   GPIO_BIPOLAR_DETECT = 6, /**< Bipolar detect input */
//   GPIO_WAKEUP_EN = 7,      /**< Wake-up enable output */
//   NUMBER_OF_GPIO,          /**< Number of GPIO channels */
// } gpio_id_e;

// /** @brief Logical PWM channel IDs, indexing pwm_inst[]. */
// typedef enum {
//   PWM_CC_REF = 0, /**< Constant-current reference PWM (TIMA0) */
//   PWM_CV_REF = 1, /**< Constant-voltage reference PWM (TIMG7) */
//   NUMBER_OF_PWM,  /**< Number of PWM channels */
// } pwm_id_e;

// /** @brief Logical GPT channel IDs, indexing gpt_inst[]. */
// typedef enum {
//   GPT_CPU_TIMER0 = 0, /**< 1 ms scheduler tick timer */
//   GPT_CPU_TIMER1 = 1, /**< 100 ms reserved timer */
//   NUMBER_OF_GPT,      /**< Number of GPT channels */
// } gpt_id_e;

// /** @brief Logical MCU interrupt IDs. */
// typedef enum {
//   MCU_INTERRUPT0 = 0,   /**< Generic MCU interrupt line 0 */
//   NUMBER_OF_INTERRUPT,  /**< Number of MCU interrupt lines */
// } mcu_id_e;

// /** @brief Logical ADC channel IDs, indexing adc_inst[]. */
// typedef enum {
//   ADC_LLC_VOUT,   /**< LLC output voltage sense (ADC1 MEM0) */
//   ADC_BAT,        /**< Battery voltage sense (ADC1 MEM1) */
//   ADC_I_CHG,      /**< Charge current sense (ADC1 MEM2) */
//   ADC_RD_TEMP,    /**< Radiator temperature sense (ADC0 MEM2) */
//   ADC_BD_TEMP,    /**< Board temperature sense (ADC0 MEM1) */
//   ADC_MCU_VAC,    /**< AC input voltage sense (ADC0 MEM0) */
//   NUMBER_OF_ADC,  /**< Number of ADC channels */
// } adc_id_e;

// /** @brief Logical UART channel IDs, indexing uart_inst[]. */
// typedef enum {
//   UART_DEBUG = 0, /**< Debug UART (UART0 / PA0 TX / PA1 RX) */
//   NUMBER_OF_UART, /**< Number of UART channels */
// } uart_id_e;

// /** @brief Logical CMPSS channel IDs, indexing cmpss_inst[]. */
// typedef enum {
//   CMPSS_NUM_0 = 0, /**< Comparator 0 (COMP0 / PA14) */
//   NUMBER_OF_CMPSS, /**< Number of CMPSS channels */
// } cmpss_id_e;

// /** @brief Logical CAN transceiver pin IDs. */
// typedef enum {
//   CAN_0 = 0,
//   NUMBER_OF_CAN,
// } can_id_e;

// /*==================================================================================================
//  *                                   FUNCTION PROTOTYPES
//  ==================================================================================================*/
// /**
//  * @brief Function prototypes for driver configuration.
//  */

// /*==================================================================================================
//  *                                   GLOBAL VARIABLES
//  ==================================================================================================*/
// /**
//  * @brief Extern global variables for driver configuration.
//  */

// /** @} */ /* End of driver_configuration_group */

// #endif /* DRV_CONFIG_H_ */

// /***************************************************************************************************
//  * End of file
//  **************************************************************************************************/
