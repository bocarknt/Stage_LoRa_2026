/*
 * hx711.c
 *
 *  Created on: May 7, 2026
 *      Author: audib
 */

#include "hx711.h"

void delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim16, 0);
    while(__HAL_TIM_GET_COUNTER(&htim16) < us);
}

void HX711_Init(HX711_HandleTypeDef *hx)
{
    // These will be set properly after calibration (DONE! here)
    // For now offset = 0 and scale = 1 to read raw values
	hx->offset = 0;    // tare handles this at startup
	hx->scale  = 211.1f;
}

long HX711_ReadRaw(HX711_HandleTypeDef *hx)
{
    long value = 0;

    // Wait until HX711 is ready (DOUT goes LOW)
    while(HAL_GPIO_ReadPin(hx->dt_port, hx->dt_pin));

    // Read 24 bits
    for(int i = 0; i < 24; i++)
    {
        delay_us(1);
        HAL_GPIO_WritePin(hx->sck_port, hx->sck_pin, GPIO_PIN_SET);
        value = value << 1;
        delay_us(1);
        HAL_GPIO_WritePin(hx->sck_port, hx->sck_pin, GPIO_PIN_RESET);
        if(HAL_GPIO_ReadPin(hx->dt_port, hx->dt_pin))
            value++;
    }

    // 1 extra pulse for Gain 128 on Channel A
    HAL_GPIO_WritePin(hx->sck_port, hx->sck_pin, GPIO_PIN_SET);
    delay_us(1);
    HAL_GPIO_WritePin(hx->sck_port, hx->sck_pin, GPIO_PIN_RESET);
    delay_us(1);

    // Sign extend 24-bit to 32-bit
    if(value & 0x800000)
        value |= 0xFF000000;

    return value;
}

float HX711_GetWeight(HX711_HandleTypeDef *hx)
{
    // Average 10 readings for stability
    long sum = 0;
    for(int i = 0; i < 10; i++)
    {
        sum += HX711_ReadRaw(hx);
        HAL_Delay(10);
    }
    long raw = sum / 10;
    return (float)(raw - hx->offset) / hx->scale;
}
