// /**
//  *******************************************************************************
//  * @file    drv_gpio_core.h
//  * @brief   Header file for GPIO driver module.
//  *
//  * @defgroup drv_gpio_group GPIO Driver
//  * @ingroup  driver_layer_group
//  *
//  * @details Provides direct GPIO control without method-pointer indirection.
//  *          Context structure strictly follows the AMS architecture:
//  *          - gpio_sw_data_t: runtime pin state/value
//  *          - gpio_hw_cfg_t: port register base and pin mask
//  *          - gpio_ctx_t: encapsulates ID, sw_data, and hw_cfg
//  *
//  *          SysConfig mapping:
//  *            OUTPUT: GPIO_LLC_LATCH/PA3, GPIO_LED_RED/PA9,
//  *                    GPIO_LED_GRN/PA10,  GPIO_RL_CTL/PA11,
//  *                    GPIO_WAKEUP_EN
//  *            INPUT:  GPIO_FAN_FAULT/PA7, GPIO_BIPOLAR_DETECT/PA24
//  *
//  * @version 1.1.0
//  * @date    07/09/2026
//  * @author  EVSELab JSC.
//  * @{
//  *******************************************************************************
//  */

// #ifndef DRV_GPIO_CORE_H_
// #define DRV_GPIO_CORE_H_

// /*==================================================================================================
//  *                                        INCLUDES
//  ==================================================================================================*/
// /**
//  * @brief Include required header files.
//  */
// #include "_config_lib.h"
// #include "drv_config.h"

// /*==================================================================================================
//  *                                        DEFINES
//  ==================================================================================================*/
// /**
//  * @brief Public constant definitions.
//  */

// /*==================================================================================================
//  *                                        MACROS
//  ==================================================================================================*/
// /**
//  * @brief Public macro definitions.
//  */

// /*==================================================================================================
//  *                                   TYPE DEFINITIONS
//  ==================================================================================================*/
// /**
//  * @brief GPIO runtime software data structure.
//  */
// typedef struct {
//   uint32_t pin_state; /**< Last written/read pin state (0 or 1) */
// } gpio_sw_data_t;

// /**
//  * @brief GPIO hardware configuration structure.
//  */
// typedef struct {
//   GPIO_Regs *port; /**< GPIO port register base (e.g. GPIOA) */
//   uint32_t pin;    /**< Pin mask (DL_GPIO_PIN_x) */
// } gpio_hw_cfg_t;

// /**
//  * @brief GPIO context structure grouping ID, software data, and hardware
//  * config.
//  */
// typedef struct {
// #ifdef ENABLE_GPIO
//   gpio_id_e id; /**< Logical GPIO ID from drv_config.h */
// #endif
//   gpio_sw_data_t sw_data; /**< Runtime software data */
//   gpio_hw_cfg_t hw_cfg;   /**< Hardware port + pin mapping */
// } gpio_ctx_t;

// /*==================================================================================================
//  *                                   FUNCTION PROTOTYPES
//  ==================================================================================================*/
// /**
//  * @brief Public API for GPIO driver.
//  */

// /**
//  * @brief  Initialize GPIO context table with SysConfig-generated pin mappings.
//  * @details Populates gpio_inst[id] for all gpio_id_e values. Must be called
//  *          after SYSCFG_DL_init().
//  * @param  void
//  * @retval void
//  */
// void drv_gpio_init(void);

// /**
//  * @brief  Configure individual GPIO object context.
//  * @param[out] p_ctx Pointer to GPIO context structure.
//  * @param[in]  id    Logical GPIO ID.
//  * @param[in]  port  GPIO port register base.
//  * @param[in]  pin   GPIO pin mask.
//  * @retval void
//  */
// void drv_gpio_config(gpio_ctx_t *p_ctx, gpio_id_e id, GPIO_Regs *port, uint32_t pin);

// /**
//  * @brief  Drive a GPIO output HIGH.
//  * @details Calls DL_GPIO_setPins(port, pin) and updates sw_data.pin_state = 1.
//  * @param[in] id  Logical GPIO ID (must be an OUTPUT pin).
//  * @retval void
//  */
// void drv_gpio_set_pin(gpio_id_e id);

// /**
//  * @brief  Drive a GPIO output LOW.
//  * @details Calls DL_GPIO_clearPins(port, pin) and updates sw_data.pin_state = 0.
//  * @param[in] id  Logical GPIO ID (must be an OUTPUT pin).
//  * @retval void
//  */
// void drv_gpio_clear_pin(gpio_id_e id);

// /**
//  * @brief  Toggle a GPIO output.
//  * @details Calls DL_GPIO_togglePins(port, pin) and toggles sw_data.pin_state.
//  * @param[in] id  Logical GPIO ID (must be an OUTPUT pin).
//  * @retval void
//  */
// void drv_gpio_toggle_pin(gpio_id_e id);

// /**
//  * @brief  Read a GPIO input level.
//  * @details Calls DL_GPIO_readPins(port, pin) and updates sw_data.pin_state.
//  * @param[in] id  Logical GPIO ID (INPUT or OUTPUT pin).
//  * @retval uint32_t  Non-zero if HIGH, zero if LOW.
//  */
// uint32_t drv_gpio_read_pin(gpio_id_e id);

// /*==================================================================================================
//  *                                   GLOBAL VARIABLES
//  ==================================================================================================*/
// /**
//  * @brief Extern global variables for GPIO driver.
//  */

// /** @} */ /* End of drv_gpio_group */

// #endif /* DRV_GPIO_CORE_H_ */

// /***************************************************************************************************
//  * End of file
//  **************************************************************************************************/
