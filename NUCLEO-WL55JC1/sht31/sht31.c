/*
 * sht31.c
 *
 *  Created on: May 11, 2026
 *      Author: audib
 */

#include "sht31.h"

void sht31_request(I2C_HandleTypeDef* I2Cx)
{
    uint8_t cmd_rep[2] = {0x21, 0x30};
    HAL_I2C_Master_Transmit(I2Cx, 0x44<<1, cmd_rep, 2, 100);
    HAL_Delay(20);
}

void sht31_read(I2C_HandleTypeDef* I2Cx, float* temperature, float* humidite)
{
    uint8_t data[6];
    uint16_t rawT, rawH;

    HAL_I2C_Master_Receive(I2Cx, 0x44<<1, data, 6, 100);
    HAL_Delay(20);

    rawT = (data[0] << 8) | data[1];
    *temperature = -45.0f + 175.0f * ((float)rawT / 65535.0f);

    rawH = (data[3] << 8) | data[4];
    *humidite = 100.0f * ((float)rawH / 65535.0f);
}

