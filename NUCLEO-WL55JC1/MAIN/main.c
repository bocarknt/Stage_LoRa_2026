/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "subghz.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hx711.h"
#include "dht11.h"
#include "sht31.h"
#include "lcd.h"
#include "lora.h"
#include <stdio.h>
#include <string.h>
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

COM_InitTypeDef BspCOMInit;
static uint32_t delay = 250;

/* USER CODE BEGIN PV */
rgb_lcd lcd;
HX711_HandleTypeDef hx711;

// PIR
uint8_t pir_detected = 0;
uint32_t pir_last_trigger = 0;

// DHT11 — inside
uint8_t RH1, RH2, TC1, TC2, SUM, Check;
int temp_in = 0, hum_in = 0;

// SHT31 — outside
float temp_out = 0.0f, hum_out = 0.0f;

// Display switching — 3 screens now
uint32_t display_timer = 0;
uint8_t display_mode = 0;
// 0 = weight + PIR
// 1 = inside temp + humidity (DHT11)
// 2 = outside temp + humidity (SHT31)

char lcd_buf[32];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void PIR_Handler(void)
{
    if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET)
    {
        pir_detected = 1;
        pir_last_trigger = HAL_GetTick();
    }
    else
    {
        if(pir_detected && (HAL_GetTick() - pir_last_trigger > 5000))
        {
            pir_detected = 0;
            reglagecouleur(255, 255, 255);
        }
    }
}
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
  MX_SUBGHZ_Init();
  MX_TIM16_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  // LCD init
  HAL_Delay(500);
  lcd_init(&hi2c1, &lcd);
  HAL_Delay(100);
  lcd_position(&hi2c1, 0, 0);
  lcd_print(&hi2c1, "  Ruche LoRa    ");
  lcd_position(&hi2c1, 0, 1);
  lcd_print(&hi2c1, "  Initializing..");
  HAL_Delay(1000);

  //LoRa init
  LoRa_envoyer("AT+RESET\r\n"); //faire reset
  HAL_Delay(2000);
  LoRa_emetteur();
  HAL_Delay(200);

  // HX711 init
  HAL_TIM_Base_Start(&htim16);
  hx711.dt_port  = GPIOB;
  hx711.dt_pin   = GPIO_PIN_2;
  hx711.sck_port = GPIOB;
  hx711.sck_pin  = GPIO_PIN_13;
  HX711_Init(&hx711);

  // Tare
  lcd_position(&hi2c1, 0, 1);
  lcd_print(&hi2c1, "  Taring...     ");
  printf("Taring...\r\n");
  long tare_sum = 0;
  for(int i = 0; i < 10; i++)
  {
      tare_sum += HX711_ReadRaw(&hx711);
      HAL_Delay(100);
  }
  hx711.offset = tare_sum / 10;
  printf("Tare offset: %ld\r\n", hx711.offset);

  // Ready
  clearlcd();
  lcd_position(&hi2c1, 0, 0);
  lcd_print(&hi2c1, "  Ruche Ready!  ");
  HAL_Delay(1000);
  clearlcd();

  display_timer = HAL_GetTick();

  /* USER CODE END 2 */

  /* Initialize leds */
  BSP_LED_Init(LED_BLUE);
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_RED);

  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_SW1, BUTTON_MODE_EXTI);
  BSP_PB_Init(BUTTON_SW2, BUTTON_MODE_EXTI);
  BSP_PB_Init(BUTTON_SW3, BUTTON_MODE_EXTI);

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN BSP */

  /* -- Sample board code to send message over COM1 port ---- */
  printf("Welcome to STM32 world !\n\r");

  /* -- Sample board code to switch on leds ---- */
  BSP_LED_On(LED_BLUE);
  BSP_LED_On(LED_GREEN);
  BSP_LED_On(LED_RED);

  /* USER CODE END BSP */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while(1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

      // Switch display mode every 5 seconds — 3 screens
      if(HAL_GetTick() - display_timer > 5000)
      {
          display_mode = (display_mode + 1) % 3;
          display_timer = HAL_GetTick();
          clearlcd();
      }

      // Always read weight
      float weight_g  = HX711_GetWeight(&hx711);
      float weight_kg = weight_g / 1000.0f;
      int kg_int = (int)weight_kg;
      int kg_dec = (int)((weight_kg - kg_int) * 1000);
      if(kg_dec < 0) kg_dec = -kg_dec;

      // Always read DHT11 (inside)
      if(DHT11_Init())
      {
          RH1 = DHT11_Read();
          RH2 = DHT11_Read();
          TC1 = DHT11_Read();
          TC2 = DHT11_Read();
          SUM = DHT11_Read();
          Check = RH1 + RH2 + TC1 + TC2;

          if(Check == SUM)
          {
              if(TC1 > 127)
                  temp_in = -(int)TC2;
              else
                  temp_in = (int)TC1;

              hum_in = (int)RH1;
          }
      }

      // Always read SHT31 (outside)
      sht31_request(&hi2c1);
      sht31_read(&hi2c1, &temp_out, &hum_out);

      // Poll PIR
      PIR_Handler();

      // Update display
      if(display_mode == 0)
      {
          // Screen 1 — Weight + PIR
          snprintf(lcd_buf, sizeof(lcd_buf), "Poids:%d.%03dkg", kg_int, kg_dec);
          lcd_position(&hi2c1, 0, 0);
          lcd_print(&hi2c1, lcd_buf);

          LoRa_send_data(lcd_buf);

          lcd_position(&hi2c1, 0, 1);
          if(pir_detected)
          {
              lcd_print(&hi2c1, "Motion detecte! ");
              snprintf(lcd_buf, sizeof(lcd_buf), "Motion detecte! ");
              LoRa_send_data(lcd_buf);
              reglagecouleur(255, 0, 0);
          }
          else
          {
              lcd_print(&hi2c1, "Pas de mouvement");
              snprintf(lcd_buf, sizeof(lcd_buf), "Pas de mouvement");
              LoRa_send_data(lcd_buf);
              reglagecouleur(255, 255, 255);
          }

          //Putty
          printf("Weight: %d.%03d kg | PIR: %s\r\n",
              kg_int, kg_dec,
              pir_detected ? "MOTION" : "clear");
      }

      else if(display_mode == 1)
      {
          // Screen 2 — Inside temp + humidity (DHT11)
          snprintf(lcd_buf, sizeof(lcd_buf), "T.int: %d C     ", temp_in);
          lcd_position(&hi2c1, 0, 0);
          lcd_print(&hi2c1, lcd_buf);

          snprintf(lcd_buf, sizeof(lcd_buf), "H.int: %d%%      ", hum_in);
          lcd_position(&hi2c1, 0, 1);
          lcd_print(&hi2c1, lcd_buf);

          printf("Inside  -> Temp: %d C | Humi: %d%%\r\n", temp_in, hum_in);
      }
      else
      {
          // Screen 3 — Outside temp + humidity (SHT31)
          int t_int = (int)temp_out;
          int t_dec = (int)((temp_out - t_int) * 10);
          int h_int = (int)hum_out;
          int h_dec = (int)((hum_out - h_int) * 10);

          snprintf(lcd_buf, sizeof(lcd_buf), "T.ext: %d.%d C   ", t_int, t_dec);
          lcd_position(&hi2c1, 0, 0);
          lcd_print(&hi2c1, lcd_buf);

          snprintf(lcd_buf, sizeof(lcd_buf), "H.ext: %d.%d%%    ", h_int, h_dec);
          lcd_position(&hi2c1, 0, 1);
          lcd_print(&hi2c1, lcd_buf);

          printf("Outside -> Temp: %d.%d C | Humi: %d.%d%%\r\n",
              t_int, t_dec, h_int, h_dec);
      }

      HAL_Delay(200);
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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3|RCC_CLOCKTYPE_HCLK
                              |RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
* @brief BSP Push Button callback
* @param Button Specifies the pressed button
* @retval None
*/
void BSP_PB_Callback(Button_TypeDef Button)
{
  switch(Button)
  {
    case BUTTON_SW1:
      /* Change the period to 100 ms */
      delay = 100;
      break;
    case BUTTON_SW2:
      /* Change the period to 500 ms */
      delay = 500;
      break;
    case BUTTON_SW3:
      /* Change the period to 1000 ms */
      delay = 1000;
      break;
    default:
      break;
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();
  while(1){}
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
