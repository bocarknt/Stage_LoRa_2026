/*
 * dht11.c
 *
 *  Created on: May 11, 2026
 *      Author: audib
 */

#include "dht11.h"
#include "tim.h"

// Using htim16 just like HX711
void dht_delay(uint16_t temp)
{
    __HAL_TIM_SET_COUNTER(&htim16, 0);
    while((__HAL_TIM_GET_COUNTER(&htim16)) < temp);
}

uint8_t DHT11_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    uint8_t ans = 0;
    uint32_t pMillis, cMillis;

    // Set pin as output and pull LOW for 18ms
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
    dht_delay(18000);

    // Pull HIGH for 10us
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    dht_delay(10);

    // Switch to input to receive response
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);

    dht_delay(40);

    if(!(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)))
    {
        dht_delay(80);
        if(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) ans = 1;
    }

    pMillis = HAL_GetTick();
    cMillis = HAL_GetTick();
    while((HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) && pMillis + 2 > cMillis)
    {
        cMillis = HAL_GetTick();
    }

    return ans;
}

uint8_t DHT11_Read(void)
{
    uint8_t a, b = 0;
    uint32_t cMillis, pMillis;

    for(a = 0; a < 8; a++)
    {
        pMillis = HAL_GetTick();
        cMillis = HAL_GetTick();
        while(!(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) && pMillis + 2 > cMillis)
        {
            cMillis = HAL_GetTick();
        }

        dht_delay(40);

        if(!(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)))
            b &= ~(1 << (7 - a));
        else
            b |= (1 << (7 - a));

        pMillis = HAL_GetTick();
        cMillis = HAL_GetTick();
        while((HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) && pMillis + 2 > cMillis)
        {
            cMillis = HAL_GetTick();
        }
    }
    return b;
}


