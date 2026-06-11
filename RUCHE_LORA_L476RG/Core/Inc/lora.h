/*
 * lora.h
 *
 *  Created on: May 11, 2026
 *      Author: benka
 */

#ifndef INC_LORA_H_
#define INC_LORA_H_


#include <stdint.h>
#include <stddef.h>
#include "usart.h"

#define DEV_EUI  "2CF7F120720010BB" //"70B3D57ED0077E46"
#define APP_EUI  "526973696E674846"
#define APP_KEY  "8F45AEBEAFB0F41CCEA57AE260AC9C1D" //"A109F5276AA0523CD238E6705E4C35A1"

void LoRa_identifier(uint8_t* cmd);
void LoRa_envoyer(char* msg);
void LoRa_recevoir(void);
void LoRa_emetteur(void);
void LoRa_afficher_rx(void);
void LoRa_send_data(char* data);
void hex_to_ascii(const char* hex_str, char* dst, size_t dst_size);
void LoRa_P2P_Send(UART_HandleTypeDef *huart, uint8_t *data, uint8_t len);
void LoRa_P2P_Init(UART_HandleTypeDef *huart);
void loraWAN_init(void);
void loraWAN_send_data(char* data);


#endif /* INC_LORA_H_ */
