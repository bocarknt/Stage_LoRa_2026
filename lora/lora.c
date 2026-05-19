/*
 * lora.c
 *
 *  Created on: May 11, 2026
 *      Author: benka
 */


#include"lora.h"
#include "lib_lcd.h"
#include <stdio.h>
#include <string.h>
#include "stm32l4xx_hal.h"
#include "usart.h"
#include "i2c.h"


extern uint8_t rxBuf[254];
uint8_t test_cmd[] ="AT\r\n";
uint8_t test_TX[] ="AT+TEST=TXLRPKT\r\n";
uint8_t check_id[]="AT+ID\r\n";
uint8_t lora_reset[]="AT+RESET\r\n";
extern char lcd_buf[254];
extern rgb_lcd lcddata;


void LoRa_identifier(uint8_t* cmd)
{
	 memset(rxBuf, 0, sizeof(rxBuf));
	 HAL_UART_Transmit(&huart3, cmd, strlen((char*)cmd), 1000);
	 HAL_UART_Receive(&huart3,rxBuf , sizeof(rxBuf), 500);
	 HAL_UART_Transmit(&huart2, rxBuf, strlen((char*)rxBuf), 1000);
	 clearlcd();
	 lcd_position(&hi2c1,0, 0) ;
	 lcd_print(&hi2c1,(char*)rxBuf);
	 HAL_Delay(20);
}

void LoRa_envoyer(char* msg)
{
	uint8_t cmd[128];
	sprintf((char*)cmd,"%s",msg);
	memset(rxBuf, 0, sizeof(rxBuf));
	HAL_UART_Transmit(&huart3, cmd, strlen((char*)cmd), 1000);
	HAL_UART_Receive(&huart3,rxBuf , sizeof(rxBuf), 500);
	HAL_UART_Transmit(&huart2, rxBuf, strlen((char*)rxBuf), 1000);
	/*
	clearlcd();
	lcd_position(&hi2c1,0, 0) ;
	lcd_print(&hi2c1,(char*)rxBuf);
	*/

}
void LoRa_send_data(char* data)
{
	uint8_t cmd[254];
	snprintf(cmd, sizeof(cmd), "AT+TEST=TXLRSTR,\"%s\"\r\n", data);
	memset(rxBuf, 0, sizeof(rxBuf));
	HAL_UART_Transmit(&huart3, cmd, strlen((char*)cmd), 1000);
	HAL_UART_Receive(&huart3,rxBuf , sizeof(rxBuf), 500);
	HAL_UART_Transmit(&huart2, rxBuf, strlen((char*)rxBuf), 1000);
	HAL_Delay(2000);
}
void LoRa_recepteur(void)
{
	LoRa_envoyer("AT+MODE=TEST\r\n");
	HAL_Delay(200);
	LoRa_envoyer("AT+TEST=?\r\n");
	HAL_Delay(200);
	LoRa_envoyer("AT+TEST=RFCFG,868,SF7,125,8,8,14,ON,OFF,OFF\r\n");
	HAL_Delay(200);
	LoRa_envoyer("AT+TEST=RXLRPKT\r\n");
	HAL_Delay(200);

}
void LoRa_emetteur(void)
{
	LoRa_envoyer("AT+MODE=TEST\r\n");
	HAL_Delay(200);
	LoRa_envoyer("AT+TEST=RFCFG,868,SF7,125,8,8,14,ON,OFF,OFF\r\n");
	HAL_Delay(200);
	LoRa_envoyer("AT+TEST=TXLRSTR\r\n");
	HAL_Delay(200);
}

void LoRa_afficher_rx(void)
{
    char* pos = strstr((char*)rxBuf, "+TEST: RX \"");
    if (pos != NULL) {
        pos += strlen("+TEST: RX \"");

        char hex_data[128] = {0};
        size_t i = 0;
        while (*pos != '"' && *pos != '\0' && i < sizeof(hex_data)-1) {
            hex_data[i++] = *pos++;
        }

        char ascii_data[32] = {0};
        hex_to_ascii(hex_data, ascii_data, sizeof(ascii_data));

        // Retour à la ligne sur UART
        char output[36] = {0};
        snprintf(output, sizeof(output), "%s\r\n", ascii_data);

        // Afficher sur UART2 avec retour à la ligne
        HAL_UART_Transmit(&huart2, (uint8_t*)output, strlen(output), 1000);

        // Afficher sur LCD ligne 0 : données brutes HEX
        clearlcd();
        lcd_position(&hi2c1, 0, 0);
        lcd_print(&hi2c1, hex_data);

        //  Afficher sur LCD ligne 1 : données décodées ASCII
        lcd_position(&hi2c1, 0, 1);
        lcd_print(&hi2c1, ascii_data);
    }
}

void hex_to_ascii(const char* hex_str, char* dst, size_t dst_size)
{
    memset(dst, 0, dst_size);
    size_t i = 0;
    const char* p = hex_str;
    while (*p && i < dst_size - 1) {
        while (*p == ' ') p++;          // sauter les espaces
        if (*p == '"' || *p == '\0') break;
        char byte_str[3] = {p[0], p[1], '\0'};
        dst[i++] = (char) strtol(byte_str, NULL, 16);
        p += 2;
    }
}
void LoRa_P2P_Init(UART_HandleTypeDef *huart)
{
    uint8_t cmd[64];

    // Mode P2P
    snprintf((char*)cmd, sizeof(cmd), "AT+MODE=TEST\r\n");
    HAL_UART_Transmit(huart, cmd, strlen((char*)cmd), 1000);
    HAL_Delay(100);

    // Config radio
    snprintf((char*)cmd, sizeof(cmd), "AT+TEST=RFCFG,868,SF7,125,8,8,14,ON,OFF,OFF\r\n");
    HAL_UART_Transmit(huart, cmd, strlen((char*)cmd), 1000);
    HAL_Delay(100);
}
