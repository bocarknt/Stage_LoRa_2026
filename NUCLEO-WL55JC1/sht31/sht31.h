/*
 * sht31.h
 *
 *  Created on: May 11, 2026
 *      Author: audib
 */

#ifndef INC_SHT31_H_
#define INC_SHT31_H_

#include "stm32wlxx_hal.h"
#include "stm32wlxx_hal_i2c.h"

void sht31_request(I2C_HandleTypeDef* I2Cx);
void sht31_read(I2C_HandleTypeDef* I2Cx, float* temperature, float* humidite);

#endif /* INC_SHT31_H_ */
