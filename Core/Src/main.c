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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "LCD_I2C.h"
#include "RC522.h"
#include "string.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define AUTHORIZED_CARD_1 {0x12, 0x34, 0x56, 0x78, 0x90} // V� d? UID th? 1
#define AUTHORIZED_CARD_2 {0xAB, 0xCD, 0xEF, 0x12, 0x34} // V� d? UID th? 2
#define AUTHORIZED_CARD_3 {0x71, 0xAB, 0xE9, 0x06}       // V� d? UID th? 3
#define AUTHORIZED_CARD_4 {0xF2, 0xD7, 0x22, 0x07}       // V� d? UID th? 4

#define MAX_AUTHORIZED_CARDS 4
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */
LCD_HandleTypeDef hlcd1;
RC522_HandleTypeDef hrc522;

MFRC522_Status status;
uint8_t str[MAX_LEN]; // Max_LEN = 16
uint8_t sNum[5];
uint8_t lastCard[5] = {0}; // Luu th? cu?i c�ng d� d?c d? tr�nh d?c tr�ng
uint8_t cardPresent = 0;   // Flag ki?m tra c� th? hay kh�ng
char displayBuffer[17];

const uint8_t authorizedCards[MAX_AUTHORIZED_CARDS][5] = {
    AUTHORIZED_CARD_1,
    AUTHORIZED_CARD_2,
    AUTHORIZED_CARD_3,
    AUTHORIZED_CARD_4

};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t isCardAuthorized(uint8_t *cardUID)
{
  for (int i = 0; i < MAX_AUTHORIZED_CARDS; i++)
  {
    if (memcmp(cardUID, authorizedCards[i], 4) == 0) // So s�nh 4 byte UID
    {
      return 1; // Th? du?c ph�p
    }
  }
  return 0; // Th? kh�ng du?c ph�p
}

void DisplayCardInfo(uint8_t *cardUID, uint8_t isAuthorized)
{
  char uidStr[17];

  // X�a m�n h�nh
  LCD_Clear_Display(&hlcd1);

  // Hi?n th? UID ? d�ng 1
  sprintf(uidStr, "UID: %02X%02X%02X%02X",
          cardUID[0], cardUID[1], cardUID[2], cardUID[3]);
  LCD_SetCursor(&hlcd1, 0, 0);
  LCD_Print(&hlcd1, uidStr);

  // Hi?n th? tr?ng th�i ? d�ng 2
  LCD_SetCursor(&hlcd1, 0, 1);
  HAL_Delay(3000);
  if (isAuthorized)
  {
    LCD_Print(&hlcd1, "ACCESS GRANTED!");
    HAL_Delay(3000);

    // TODO: Th�m h�nh d?ng khi du?c ph�p
    // V� d?: B?t d�n LED trong 2 gi�y
    // HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    // HAL_Delay(2000);
    // HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
  }
  else
  {
    LCD_Print(&hlcd1, "ACCESS DENIED!");
    HAL_Delay(3000);
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
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  LCD_Init(&hlcd1, &hi2c1, LCD_ADDR);
  LCD_Backlight(&hlcd1);

  LCD_Clear_Display(&hlcd1);
  MFRC522_Init(&hrc522, &hspi1);

  // �?c version register
  uint8_t version = Read_MFRC522(VERSION_REG);

  sprintf(displayBuffer, "Ver:0x%02X", version);
  LCD_SetCursor(&hlcd1, 0, 0);
  LCD_Print(&hlcd1, displayBuffer);

  HAL_Delay(3000);
  LCD_Clear_Display(&hlcd1);

  // if (version == 0x92 || version == 0x12 || version == 0x18) {
  //     LCD_SetCursor(&hlcd1, 1, 0);
  //     LCD_Print(&hlcd1, "RC522 OK!");
  // } else {
  //     LCD_SetCursor(&hlcd1, 1, 0);
  //     LCD_Print(&hlcd1, "RC522 ERROR!");
  // }
  HAL_Delay(2000);

  LCD_Clear_Display(&hlcd1);
  LCD_SetCursor(&hlcd1, 0, 0);
  LCD_Print(&hlcd1, "RFID RC522!");
  LCD_SetCursor(&hlcd1, 0, 1);
  LCD_Print(&hlcd1, "Place your card...");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    status = MFRC522_Request(PICC_REQIDL, str);

    if (status == MI_OK)
    {
      // �� t�m th?y th?, th?c hi?n ch?ng va ch?m d? d?c UID
      status = MFRC522_Anticoll(str);

      //    char dbg[17];
      //    sprintf(dbg, "n=%d , lb=%d", debug_n, debug_lastBits);
      //
      //		LCD_Clear_Display(&hlcd1);
      //    LCD_SetCursor(&hlcd1, 0, 0);
      //    LCD_Print(&hlcd1, dbg);
      //    HAL_Delay(1000);
      //
      //		char dbg1[17];
      //    sprintf(dbg1, "%02X%02X%02X%02X%02X", debug_raw[0], debug_raw[1], debug_raw[2], debug_raw[3], debug_raw[4]);
      //    LCD_SetCursor(&hlcd1, 0, 1);
      //    LCD_Print(&hlcd1, dbg1);
      //    HAL_Delay(1000);

      if (status == MI_OK)
      {
        // uint8_t bcc = str[0] ^ str[1] ^ str[2] ^ str[3];
        char dbg[17];
        sprintf(dbg, "%02X%02X%02X%02X/%02X", str[0], str[1], str[2], str[3], str[4]);
        LCD_Clear_Display(&hlcd1);
        LCD_SetCursor(&hlcd1, 0, 0);
        LCD_Print(&hlcd1, dbg);
        HAL_Delay(1000);
        // Sao ch�p UID (5 byte: 4 byte UID + 1 byte checksum)
        memcpy(sNum, str, 5);

        // Ki?m tra xem c� ph?i th? m?i kh�ng (tr�nh d?c tr�ng)
        if (memcmp(sNum, lastCard, 5) != 0)
        {
          // Luu l?i th? v?a d?c
          memcpy(lastCard, sNum, 5);
          cardPresent = 1;

          // Ki?m tra quy?n truy c?p
          uint8_t authorized = isCardAuthorized(sNum);

          // Hi?n th? th�ng tin th? l�n LCD
          DisplayCardInfo(sNum, authorized);

          // N?u du?c ph�p, hi?n th? Access Granted
          if (authorized)
          {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            HAL_Delay(2000);
          }
        }
      }
    }
    else
    {
      // Kh�ng c� th?, reset flag
      if (cardPresent == 1)
      {
        cardPresent = 0;
        memset(lastCard, 0, 5);

        // X�a m�n h�nh v� hi?n th? th�ng b�o ch?
        LCD_Clear_Display(&hlcd1);
        LCD_SetCursor(&hlcd1, 0, 0);
        LCD_Print(&hlcd1, "RFID RC522!");
        LCD_SetCursor(&hlcd1, 0, 1);
        LCD_Print(&hlcd1, "Place your card...");
      }
    }

    HAL_Delay(100);
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

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */
}

/**
 * @brief SPI1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RESET_GPIO_Port, RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : RESET_Pin */
  GPIO_InitStruct.Pin = RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CS_Pin */
  GPIO_InitStruct.Pin = CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(CS_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
