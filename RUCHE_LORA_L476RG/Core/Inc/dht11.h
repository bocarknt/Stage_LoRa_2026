/*
 * dht11.h
 *
 *  Created on: May 11, 2026
 *      Author: audib
 */

#ifndef INC_DHT11_H_
#define INC_DHT11_H_
#include "stm32l4xx_hal.h"
//#include "stm32wlxx_hal.h"

// Change this pin to wherever you wire the DHT11 signal pin
#define DHT11_PORT      GPIOB
#define DHT11_PIN       GPIO_PIN_10

typedef struct {
    uint8_t humidity_int;
    uint8_t humidity_dec;
    uint8_t temperature_int;
    uint8_t temperature_dec;
    uint8_t checksum;
} DHT11_Data;

uint8_t DHT11_Init(void);
uint8_t DHT11_Read(void);
void dht_delay(uint16_t temp);

#endif /* INC_DHT11_H_ */
