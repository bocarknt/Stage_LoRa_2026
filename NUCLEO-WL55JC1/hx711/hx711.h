/*
 * hx711.h
 *
 *  Created on: May 7, 2026
 *      Author: audib
 */

#ifndef HX711_H
#define HX711_H

#include "stm32wlxx_hal.h"

extern TIM_HandleTypeDef htim16;

typedef struct {
    GPIO_TypeDef* dt_port;
    uint16_t      dt_pin;
    GPIO_TypeDef* sck_port;
    uint16_t      sck_pin;
    long          offset;
    float         scale;
} HX711_HandleTypeDef;

void  delay_us(uint16_t us);
void  HX711_Init(HX711_HandleTypeDef *hx);
long  HX711_ReadRaw(HX711_HandleTypeDef *hx);
float HX711_GetWeight(HX711_HandleTypeDef *hx);

#endif /* INC_HX711_H_ */
