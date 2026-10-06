// /**
//  *******************************************************************************
//  * @file    drv_gpio_core.c
//  * @brief   Source file for GPIO driver module.
//  *
//  * @addtogroup drv_gpio_group
//  * @{
//  *
//  * @details Implements digital output/input control (set/clear/toggle/read)
//  *          for all logical GPIO channels defined in drv_config.h.
//  *
//  * @version 1.1.0
//  * @date    07/09/2026
//  * @author  EVSELab JSC.
//  *******************************************************************************
//  */

// #include "drv_gpio_core.h"

// /** @brief GPIO instance array — holds runtime context for all GPIO channels. */
//  gpio_ctx_t gpio_inst[NUMBER_OF_GPIO];

// /**
//  * @brief  Configure individual GPIO channel context.
//  * @param[out] p_ctx Pointer to GPIO context structure to populate.
//  * @param[in]  id    Logical GPIO ID.
//  * @param[in]  port  GPIO port base register address.
//  * @param[in]  pin   Pin bitmask within the port.
//  * @retval void
//  * @note   Resets sw_data.pin_state to 0.
//  */
// void drv_gpio_config(gpio_ctx_t *p_ctx, gpio_id_e id, GPIO_Regs *port,
//                      uint32_t pin)
// {
//     if (p_ctx == (gpio_ctx_t *)0 || id >= NUMBER_OF_GPIO) {
//         return;
//     }

// #ifdef ENABLE_GPIO
//     p_ctx->id = id;
// #endif
//     p_ctx->hw_cfg.port = port;
//     p_ctx->hw_cfg.pin = pin;
//     p_ctx->sw_data.pin_state = 0U;
// }

// /**
//  * @brief  Initialize all GPIO channel contexts and drive default output states.
//  * @details Populates gpio_inst[id] for every gpio_id_e, then clears all output
//  *         pins (LLC_LATCH, RL_CTL, MCU_FAN, LED_RED, LED_GRN, WAKEUP_EN).
//  * @param  void
//  * @retval void
//  * @note   Must be called after SYSCFG_DL_init().
//  */
// void drv_gpio_init(void)
// {
//     drv_gpio_config(&gpio_inst[GPIO_LLC_LATCH], GPIO_LLC_LATCH,
//                     GPIO_OUTPUT_PORT, GPIO_OUTPUT_LLC_LATCH_PIN);
//     drv_gpio_config(&gpio_inst[GPIO_FAN_FAULT], GPIO_FAN_FAULT,
//                     GPIO_INPUT_PORT, GPIO_INPUT_FAN_FAULT_PIN);
//     drv_gpio_config(&gpio_inst[GPIO_RL_CTL], GPIO_RL_CTL,
//                     GPIO_OUTPUT_PORT, GPIO_OUTPUT_RL_CTL_PIN);
//     drv_gpio_config(&gpio_inst[GPIO_MCU_FAN], GPIO_MCU_FAN,
//                     GPIO_OUTPUT_PORT, GPIO_OUTPUT_MCU_FAN_PIN);
//     drv_gpio_config(&gpio_inst[GPIO_LED_RED], GPIO_LED_RED,
//                     GPIO_OUTPUT_PORT, GPIO_OUTPUT_LED_RED_PIN);
//     drv_gpio_config(&gpio_inst[GPIO_LED_GRN], GPIO_LED_GRN,
//                     GPIO_OUTPUT_PORT, GPIO_OUTPUT_LED_GRN_PIN);
//     drv_gpio_config(&gpio_inst[GPIO_BIPOLAR_DETECT], GPIO_BIPOLAR_DETECT,
//                     GPIO_INPUT_PORT, GPIO_INPUT_BIPOLAR_DETECT_PIN);
//     // drv_gpio_config(&gpio_inst[GPIO_WAKEUP_EN], GPIO_WAKEUP_EN,
//     //                 GPIO_OUTPUT_PORT, GPIO_OUTPUT_WAKEUP_EN_PIN);


//     drv_gpio_clear_pin(GPIO_LLC_LATCH);
//     drv_gpio_clear_pin(GPIO_RL_CTL);
//     drv_gpio_clear_pin(GPIO_MCU_FAN);
//     drv_gpio_clear_pin(GPIO_LED_RED);
//     drv_gpio_clear_pin(GPIO_LED_GRN);
//     drv_gpio_clear_pin(GPIO_WAKEUP_EN);
// }

// /**
//  * @brief  Drive a GPIO output pin high.
//  * @details Calls DL_GPIO_setPins() and updates sw_data.pin_state to 1.
//  * @param[in] id  Logical GPIO ID.
//  * @retval void
//  */
// void drv_gpio_set_pin(gpio_id_e id)
// {
//     if (id >= NUMBER_OF_GPIO || gpio_inst[id].hw_cfg.port == (GPIO_Regs *)0) {
//         return;
//     }

//     DL_GPIO_setPins(gpio_inst[id].hw_cfg.port, gpio_inst[id].hw_cfg.pin);
//     gpio_inst[id].sw_data.pin_state = 1U;
// }

// /**
//  * @brief  Drive a GPIO output pin low.
//  * @details Calls DL_GPIO_clearPins() and updates sw_data.pin_state to 0.
//  * @param[in] id  Logical GPIO ID.
//  * @retval void
//  */
// void drv_gpio_clear_pin(gpio_id_e id)
// {
//     if (id >= NUMBER_OF_GPIO || gpio_inst[id].hw_cfg.port == (GPIO_Regs *)0) {
//         return;
//     }

//     DL_GPIO_clearPins(gpio_inst[id].hw_cfg.port, gpio_inst[id].hw_cfg.pin);
//     gpio_inst[id].sw_data.pin_state = 0U;
// }

// /**
//  * @brief  Toggle a GPIO output pin.
//  * @details Calls DL_GPIO_togglePins() and flips sw_data.pin_state.
//  * @param[in] id  Logical GPIO ID.
//  * @retval void
//  */
// void drv_gpio_toggle_pin(gpio_id_e id)
// {
//     if (id >= NUMBER_OF_GPIO || gpio_inst[id].hw_cfg.port == (GPIO_Regs *)0) {
//         return;
//     }

//     DL_GPIO_togglePins(gpio_inst[id].hw_cfg.port, gpio_inst[id].hw_cfg.pin);
//     gpio_inst[id].sw_data.pin_state ^= 1U;
// }

// /**
//  * @brief  Read the current logic level of a GPIO pin.
//  * @details Calls DL_GPIO_readPins() and caches the result in sw_data.pin_state.
//  * @param[in] id  Logical GPIO ID.
//  * @retval uint32_t 1 if pin is high, 0 if low or id/port invalid.
//  */
// uint32_t drv_gpio_read_pin(gpio_id_e id)
// {
//     uint32_t state;

//     if (id >= NUMBER_OF_GPIO || gpio_inst[id].hw_cfg.port == (GPIO_Regs *)0) {
//         return 0U;
//     }

//     state = DL_GPIO_readPins(gpio_inst[id].hw_cfg.port, gpio_inst[id].hw_cfg.pin);
//     gpio_inst[id].sw_data.pin_state = (state != 0U) ? 1U : 0U;
//     return gpio_inst[id].sw_data.pin_state;
// }

// /** @} */ /* End of drv_gpio_group */

// /***************************************************************************************************
//  * End of file
//  **************************************************************************************************/
