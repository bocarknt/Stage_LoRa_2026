/*
 * sht31.c
 *
 *  Created on: May 11, 2026
 *      Author: benka
 */


#include "sht31.h"
#include <stdio.h>
#include <string.h>
#include "lora.h"
//extern char lcd_buf[254];

/********************fonction sht31************/
/************** Demande d'effectuer des mesures *************/
void sht31_request(I2C_HandleTypeDef* I2Cx)
{
	uint8_t cmd_rep[2] = {0x21, 0x30};//cmd[2] = {0x2C, 0x06}

	//Demande de mesure
	HAL_I2C_Master_Transmit(I2Cx,0x44<<1, cmd_rep,2,100);//0x2130: 1 high repeatability 1mps
	//HAL_I2C_Master_Transmit(&hi2c1,0x44<<1, cmd,2,100);//0x2C06: high repeatability measurement with clock stretching enabled

	HAL_Delay(20);//attente
}
/*******Lecture des mesures  effectuées ***********/
void sht31_read(I2C_HandleTypeDef* I2Cx,float * temperature, float * humidite)
{
	uint16_t rawT, mesH;
	uint8_t data[6];
		HAL_I2C_Master_Receive(I2Cx,0x44<<1, data,6,100);
		  //atttente
		  HAL_Delay(20);

		  //reccuperation des données, conversion binaire
		  rawT = (data[0] << 8) | data[1];
		  //Calcule de la valeur de la temperature
		  *temperature = -45.0f + 175.0f * ((float)rawT / 65535.0f);
		  //T =temperature *100;//pour l'affichage
		  //reccuperation des données, conversion binaire
		  mesH= (data[3]<<8|data[4]);
		  //Calcule de la valeur de l'himudite
		  *humidite=100*((float)mesH/65535);
		 // H=humidite*100;//pour l'affichage
}
/*************** Affichage des mesure au LCD ***************/
void lcd_affichage(I2C_HandleTypeDef* I2Cx, float temperature, float humidite)
{
	//Affichage de la temperature au centiéme près car la resolution 0.015°C; precision + ou - 0.3°c
	char lcd_data[254];
	int T,H;
	T =temperature *100;//pour l'affichage
	H=humidite*100;//pour l'affichage
		  clearlcd();
		  snprintf(lcd_data, sizeof(lcd_data),"Temp: %d.%d%cC",(T/100),(T%100),0xDF);
		  //LoRa_send_data(lcd_data);
		  //loraWAN_send_data(lcd_data);
		  lcd_position(I2Cx,0, 0) ;
		  lcd_print(I2Cx, lcd_data);
		  //resolution 0.01 % / precision + ou - 2%
		  snprintf(lcd_data, sizeof(lcd_data),"Himd: %d.%d%%",(H/100),(H%100));
		  //LoRa_send_data(lcd_data);
		  //loraWAN_send_data(lcd_data);
		  lcd_position(I2Cx,0, 1) ;
		  lcd_print(I2Cx, lcd_data);
		  HAL_Delay(1000);
}
