/**
 * @file    delay.h
 * @author  Ryosuke Matsui
 * @version V1.0.0
 * @date    2017/02/11
 * @brief
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef DELAY_H_
#define DELAY_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stm32g4xx_hal.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void delay_us(uint32_t us);
void delay_usFromCount(uint32_t us, uint32_t startCount);
void delay_ms(uint32_t ms);
void delay_msFromCount(uint32_t ms, uint32_t startCount);
uint32_t delay_getCount(void);

#ifdef __cplusplus
}
#endif

#endif /* DELAY_H_ */
