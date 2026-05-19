/*
 * sht31.h
 *
 *  Created on: May 11, 2026
 *      Author: Bocar
 */



#ifndef INC_SHT31_H_
#define INC_SHT31_H_

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_i2c.h"
#include "lib_lcd.h"



extern rgb_lcd lcddata;

void sht31_request(I2C_HandleTypeDef* I2Cx);
void sht31_read(I2C_HandleTypeDef* I2Cx,float * temperature, float * humidite);
void lcd_affichage(I2C_HandleTypeDef* I2Cx, float temperature, float humidite);


#endif /* INC_SHT31_H_ */
