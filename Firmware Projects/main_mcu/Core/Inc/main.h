/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define F_REQ_DONE_Pin GPIO_PIN_3
#define F_REQ_DONE_GPIO_Port GPIOC
#define F_PROCESS_REQ_Pin GPIO_PIN_1
#define F_PROCESS_REQ_GPIO_Port GPIOA
#define F_RST_Pin GPIO_PIN_2
#define F_RST_GPIO_Port GPIOA
#define F_CS0_Pin GPIO_PIN_3
#define F_CS0_GPIO_Port GPIOA
#define F_CS1_Pin GPIO_PIN_4
#define F_CS1_GPIO_Port GPIOA
#define F_SCK_Pin GPIO_PIN_5
#define F_SCK_GPIO_Port GPIOA
#define F_MISO_Pin GPIO_PIN_6
#define F_MISO_GPIO_Port GPIOA
#define F_MOSI_Pin GPIO_PIN_7
#define F_MOSI_GPIO_Port GPIOA
#define FLASH_CS_Pin GPIO_PIN_4
#define FLASH_CS_GPIO_Port GPIOC
#define F_REQ2_Pin GPIO_PIN_5
#define F_REQ2_GPIO_Port GPIOC
#define F_REQ1_Pin GPIO_PIN_0
#define F_REQ1_GPIO_Port GPIOB
#define F_REQ0_Pin GPIO_PIN_1
#define F_REQ0_GPIO_Port GPIOB
#define F_ADDR3_Pin GPIO_PIN_2
#define F_ADDR3_GPIO_Port GPIOB
#define F_ADDR2_Pin GPIO_PIN_11
#define F_ADDR2_GPIO_Port GPIOF
#define F_ADDR1_Pin GPIO_PIN_12
#define F_ADDR1_GPIO_Port GPIOF
#define F_ADDR0_Pin GPIO_PIN_13
#define F_ADDR0_GPIO_Port GPIOF
#define SH_RST_Pin GPIO_PIN_14
#define SH_RST_GPIO_Port GPIOF
#define LDAC_Pin GPIO_PIN_14
#define LDAC_GPIO_Port GPIOE
#define SHCP_Pin GPIO_PIN_13
#define SHCP_GPIO_Port GPIOB
#define DOUT_Pin GPIO_PIN_14
#define DOUT_GPIO_Port GPIOB
#define DIN_Pin GPIO_PIN_15
#define DIN_GPIO_Port GPIOB
#define STCP_Pin GPIO_PIN_8
#define STCP_GPIO_Port GPIOD
#define OE_Pin GPIO_PIN_9
#define OE_GPIO_Port GPIOD
#define CLKBUF_EN_Pin GPIO_PIN_2
#define CLKBUF_EN_GPIO_Port GPIOG
#define BUZZER_Pin GPIO_PIN_8
#define BUZZER_GPIO_Port GPIOG
#define LED1_Pin GPIO_PIN_6
#define LED1_GPIO_Port GPIOC
#define LED2_Pin GPIO_PIN_7
#define LED2_GPIO_Port GPIOC
#define LED3_Pin GPIO_PIN_8
#define LED3_GPIO_Port GPIOC
#define LED4_Pin GPIO_PIN_9
#define LED4_GPIO_Port GPIOC
#define LED5_Pin GPIO_PIN_8
#define LED5_GPIO_Port GPIOA
#define LED6_Pin GPIO_PIN_15
#define LED6_GPIO_Port GPIOA
#define LED7_Pin GPIO_PIN_10
#define LED7_GPIO_Port GPIOC
#define LED8_Pin GPIO_PIN_11
#define LED8_GPIO_Port GPIOC
#define SW1_Pin GPIO_PIN_12
#define SW1_GPIO_Port GPIOC
#define SW3_Pin GPIO_PIN_0
#define SW3_GPIO_Port GPIOD
#define SW2_Pin GPIO_PIN_1
#define SW2_GPIO_Port GPIOD
#define SW4_Pin GPIO_PIN_2
#define SW4_GPIO_Port GPIOD
#define new_ic_switch_Pin GPIO_PIN_9
#define new_ic_switch_GPIO_Port GPIOG
#define LED_STM_Pin GPIO_PIN_15
#define LED_STM_GPIO_Port GPIOG
#define PW_CARD_RESET_Pin GPIO_PIN_9
#define PW_CARD_RESET_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
