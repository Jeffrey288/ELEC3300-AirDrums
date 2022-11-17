/*
 * MPU9250_Config.h
 *
 *  Created on: Feb 28, 2019
 *      Author: Desert
 */

#include "stm32f1xx_hal.h"

#ifndef UTIL_MPU9250_CONFIG_H_
#define UTIL_MPU9250_CONFIG_H_

#define MPU9250_SPI			hspi1
extern GPIO_TypeDef *MPU9250_CS_GPIO;
extern uint16_t MPU9250_CS_PIN;


#endif /* UTIL_MPU9250_CONFIG_H_ */
