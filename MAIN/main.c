/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lib_lcd.h"
#include "hx711.h"
#include "sht31.h"
#include "stdio.h"
#include "string.h"
#include"lora.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t rxBuf[254];
char lcd_buf[254];
rgb_lcd lcddata;
int32_t raw_value=0;
//Weight Sensor HX711
HX711_HandleTypeDef hx711;

// PIR Motion Sensor
volatile uint8_t pir_detected = 0;
uint32_t pir_last_trigger = 0;

char uart_buf[254];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_TIM16_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  /**************** LCD init *************/
  lcd_init(&hi2c1, &lcddata);
  HAL_Delay(200);
  reglagecouleur(255, 255, 255); // red backlight
  HAL_Delay(200);

  /**************** LORA Init ***********/

  // LoRa_identifier("AT+VER\r\n"); // Version de LORA E5
  LoRa_identifier("AT+RESET\r\n"); // Reset le module LORA E5
  HAL_Delay(2000);
  LoRa_emetteur();// configurer le module en émetteur
  HAL_Delay(200);



  lcd_position(&hi2c1, 0, 0);
  lcd_print(&hi2c1, "HX711 Test");
  lcd_position(&hi2c1, 0, 1);
  lcd_print(&hi2c1, "Initializing...");

  lcd_position(&hi2c1, 0, 0);
  lcd_print(&hi2c1, "  Poids / Weight");
  lcd_position(&hi2c1, 0, 1);
  lcd_print(&hi2c1, "  No motion     ");
  HAL_Delay(1000);

  // HX711 init
  HAL_TIM_Base_Start(&htim16);
  hx711.dt_port  = GPIOA;
  hx711.dt_pin   = GPIO_PIN_9;   // PA9 = D8 = DOUT
  hx711.sck_port = GPIOA;
  hx711.sck_pin  = GPIO_PIN_8;  // PA8 = D7 = SCK
  HX711_Init(&hx711);

  // Tare with nothing on scale
  lcd_position(&hi2c1, 0, 1);
  lcd_print(&hi2c1, "Taring...       ");
  printf("Taring...\r\n");

  long tare_sum = 0;
  for(int i = 0; i < 10; i++)
  {
      tare_sum += HX711_ReadRaw(&hx711);
      HAL_Delay(100);
  }
  hx711.offset = tare_sum / 10;
  printf("Tare offset: %ld\r\n", hx711.offset);

  lcd_position(&hi2c1, 0, 1);
  lcd_print(&hi2c1, "Ready!          ");
  HAL_Delay(1000);
  clearlcd();

  //Activation du capteur SHT31
  float temperature, humidite;
  sht31_request(&hi2c1);

  HAL_Delay(100);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */


	 //snprintf(lcd_buf, strlen(lcd_buf), "AT+TEST=TXLSTR,\"salut\"\r\n");
	   //  HAL_UART_Transmit(&huart3, (uint8_t *)lcd_buf, strlen(lcd_buf), 1000);

	//  LoRa_envoyer("AT+TEST=TXLRSTR,\"salut\"\r\n");

	    // Weight reading
	    float weight_g = HX711_GetWeight(&hx711);
	    float weight_kg = weight_g / 1000.0f;
	    int kg_int = (int)weight_kg;
	    int kg_dec = (int)((weight_kg - kg_int) * 1000);
	    if(kg_dec < 0) kg_dec = -kg_dec;

	    // PuTTY
	   /* printf("Weight: %d.%03d kg | PIR: %s\r\n",
	        kg_int, kg_dec,
	        pir_detected ? "MOTION!" : "clear");
		*/
	    // LCD line 1 — weight

	    snprintf(lcd_buf, sizeof(lcd_buf), "P:%d.%03d kg", kg_int, kg_dec);
	    LoRa_send_data(lcd_buf);
	    clearlcd();
	    lcd_position(&hi2c1, 0, 0);
	    lcd_print(&hi2c1, lcd_buf);

	    // LCD line 2 — PIR status
	    lcd_position(&hi2c1, 0, 1);
	    if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET)
	    {
	        lcd_print(&hi2c1, "  Motion detect!");
	        snprintf(lcd_buf, sizeof(lcd_buf), "  Motion detect  ");
	        LoRa_send_data(lcd_buf);
	    }
	    else
	    {
	        lcd_print(&hi2c1, "  No motion     ");
	        snprintf(lcd_buf, sizeof(lcd_buf), "  No Motion  ");
	        LoRa_send_data(lcd_buf);
	    }

	    if(pir_detected && (HAL_GetTick() - pir_last_trigger > 5000))
	    {
	        pir_detected = 0;
	        //reglagecouleur(255, 255, 255);
	    }
	    //HAL_Delay(2000);
	    //Lecture de la température et de l'humidite
	    sht31_read(&hi2c1,&temperature, &humidite);

	    //affichage de la température et de l'humidite
	    lcd_affichage(&hi2c1, temperature, humidite);



	    HAL_Delay(1000);

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	pir_detected = 1;
	            pir_last_trigger = HAL_GetTick();
   /* if(GPIO_Pin == GPIO_PIN_5)
    {

    	// Read actual pin state
        if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET)
        {
            pir_detected = 1;
            pir_last_trigger = HAL_GetTick();
        }
    }*/
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
