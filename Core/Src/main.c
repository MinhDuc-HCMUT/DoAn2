/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
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
#include "stdio.h"
#include "lcd.h"
#include "rc522.h"
#include "flash.h"
#include "keypad4x4.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define StartAddressUID 0x0800A000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c2;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint32_t starButtonPressStartTime = 0;
uint8_t CardID[MFRC522_MAX_LEN];
uint8_t exitmenu = 255;
LCD_I2C_HandleTypeDef LCD1;
uint32_t AddressUID = StartAddressUID;
uint8_t PassWord[6] = {0,1,2,3,4,5};
uint32_t delayloa = 100;
uint8_t doorStatus = 0;
char str[20];


uint8_t state_esp_rifd = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM2_Init(void);
static void MX_I2C2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
char opened[] = "OPENED";
char wrong[]="WRONG-CARD";
char wrongpass[]="WRONG-PASSWORD";
char admin1[]="ADMIN1";
char admin2[]="ADMIN2";
char admin3[]="ADMIN3";
char admin4[]="ADMIN4";
char guest1[]="GUEST1";
char guest2[]="GUEST2";
char guest3[]="GUEST3";
char guest4[]="GUEST4";
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void seclectmenu(void);
void adduid(uint8_t key);
void removeuid(uint32_t address);
void checkuser(void);
void checkthe(void);
void startadd(void);
void setaddress(void);
void password_keypad(void);
void password_button(void);
void resetflash(void);
uint8_t checkMENU(void);
uint8_t checkSELECT(void);
uint8_t checkcountUID(void);
uint32_t CheckKey(uint8_t key);
uint8_t CheckUID(uint8_t *data, uint32_t address);
uint8_t CheckListUID(uint8_t *data);
uint8_t checkbuton(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
	if(exitmenu > 0)
		exitmenu --;
	else exitmenu = 0;
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
  MX_SPI1_Init();
  MX_TIM2_Init();
  MX_I2C2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  TM_MFRC522_Init();

  CLCD_I2C_Init(&LCD1, &hi2c2, 16, 2, 0x27<<1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	HAL_TIM_Base_Start_IT(&htim2);// khoi dong timer interrupt
	HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, 0);
	if (checkcountUID() == 0)
	{
		startadd();
	}
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

		CLCD_SetCursor(&LCD1, 0, 0);
		CLCD_WriteString(&LCD1, " SCAN YOUR CARD");
		if (checkbuton(MENU_GPIO_Port, MENU_Pin) != 0)
		{
			exitmenu = 15;
			CLCD_SetCursor(&LCD1, 0, 1);
			CLCD_WriteString(&LCD1, "   ADMIN CARD");
			uint8_t key = 0;
			uint8_t stat ;
			while (exitmenu )
			{
				if (TM_MFRC522_Check(CardID) == MI_OK)
				{
					key = CheckListUID(CardID);
					key = key >> 4;
					break;
				}
				stat = checkbuton(MENU_GPIO_Port, MENU_Pin);
				switch(stat)
				{
				case 0:
					break;
				case 3:
					password_button();
					key = 99;
					exitmenu = 0;
					break;
				default :
					key = 2;
					exitmenu = 0;
					break;
				}
			}
			switch (key){
			case 1:
				seclectmenu();
				break;
			case 99:
				CLCD_Clear(&LCD1);
				break;
			default:
				CLCD_Clear(&LCD1);
				CLCD_SetCursor(&LCD1, 0, 0);
				CLCD_WriteString(&LCD1, "NOT ACCCESSIBLE");
				HAL_Delay(2000);
				CLCD_Clear(&LCD1);
				break;
			}
		}
		if (TM_MFRC522_Check(CardID) == MI_OK)
		{
//			sprintf(str, "ID: 0x%02X%02X%02X%02X%02X", CardID[0], CardID[1], CardID[2], CardID[3], CardID[4]);
			if (CheckListUID(CardID) != 0)
			{
				HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, 1);
				CLCD_Clear(&LCD1);
				CLCD_SetCursor(&LCD1, 0, 0);
				CLCD_WriteString(&LCD1, "    WELCOME");
				HAL_Delay(500);
				HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, 0);
				HAL_UART_Transmit(&huart1, opened, sizeof(opened)-1, HAL_MAX_DELAY);
				HAL_Delay(4000);
				checkuser();
			}
			else
			{
				CLCD_Clear(&LCD1);
				CLCD_SetCursor(&LCD1, 0, 0);
				CLCD_WriteString(&LCD1, "   WRONG CARD");
				HAL_UART_Transmit(&huart1, wrong, sizeof(wrong)-1, HAL_MAX_DELAY);
				HAL_Delay(3000);
			}

//			CLCD_Clear(&LCD1);
//			CLCD_SetCursor(&LCD1, 0, 0);
//			CLCD_WriteString(&LCD1, " MOI QUET THE ");
		}
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
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
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
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

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
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
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
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 35999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : CS_Pin */
  GPIO_InitStruct.Pin = CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BUZZER_Pin */
  GPIO_InitStruct.Pin = BUZZER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(BUZZER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SELECCT_Pin */
  GPIO_InitStruct.Pin = SELECCT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SELECCT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : MENU_Pin */
  GPIO_InitStruct.Pin = MENU_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(MENU_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

uint8_t checkMENU(void) {


	if (KEYPAD_Read() != '*')
		return 0;
	else
	{
		HAL_Delay(50);
		if(KEYPAD_Read() != '*')	return 0;
		uint8_t i = 50;
		while (KEYPAD_Read() == '*')
		{
			HAL_Delay(10);
			i--;
			if (i == 0)
			{
				i = 150;
				while (KEYPAD_Read() == '*')
				{
					HAL_Delay(10);
					i--;
					if(i == 0)	return 3;
				}
				return 2;
			}
		}
		return 1;
	}
//    if (buttonPressed == '*') {
//        starButtonPressStartTime = HAL_GetTick();
//    } else {
//        starButtonPressStartTime = 0;
//    }
//
//    currentTime = HAL_GetTick();
//    if (starButtonPressStartTime != 0 && (currentTime - starButtonPressStartTime) >= 500)
//        return 3;
//    else
//    	return 1;
}
uint8_t checkSELECT(void) {
	if (KEYPAD_Read() != '#')
		return 0;
	else
	{
		HAL_Delay(50);
		if(KEYPAD_Read() != '#')	return 0;
		uint8_t i = 50;
		while (KEYPAD_Read() == '#')
		{
			HAL_Delay(10);
			i--;
			if (i == 0)
			{
				i = 250;
				while (KEYPAD_Read() == '#')
				{
					HAL_Delay(10);
					i--;
					if(i == 0)	return 3;
				}
				return 2;
			}
		}
		return 1;
	}
}

uint8_t checkbuton(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	if (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == 1)
		return 0;
	else
	{
		HAL_Delay(50);
		if(HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == 1)	return 0;
		uint8_t i = 50;
		while (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == 0)
		{
			HAL_Delay(10);
			i--;
			if (i == 0)
			{
				i = 250;
				while (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == 0)
				{
					HAL_Delay(10);
					i--;
					if(i == 0)	return 3;
				}
				return 2;
			}
		}
		return 1;
	}
}

void seclectmenu(void)
{
	exitmenu = 15;
	uint8_t status = -1;
	CLCD_Clear(&LCD1);
	CLCD_SetCursor(&LCD1, 0, 0);
	CLCD_WriteString(&LCD1, "  SELECT MENU");
	CLCD_SetCursor(&LCD1, 0, 1);
	CLCD_WriteString(&LCD1, "PLEASE PRESS #");
	while (exitmenu )
	{
		if (checkbuton(SELECT_GPIO_Port, SELECT_Pin) != 0)
		{
			exitmenu = 15;
			status++;
			status = (status > 3) ? 0 : status;
			switch (status)
			{
			case 0:
				CLCD_Clear(&LCD1);
				CLCD_SetCursor(&LCD1, 0, 0);
				CLCD_WriteString(&LCD1, "=>  ADD CARD");
				CLCD_SetCursor(&LCD1, 0, 1);
				CLCD_WriteString(&LCD1, "    REMOVE CARD");
				break;
			case 1:
				CLCD_Clear(&LCD1);
				CLCD_SetCursor(&LCD1, 0, 0);
				CLCD_WriteString(&LCD1, "    ADD CARD");
				CLCD_SetCursor(&LCD1, 0, 1);
				CLCD_WriteString(&LCD1, "=>  REMOVE CARD");
				break;
			case 2:
				CLCD_Clear(&LCD1);
				CLCD_SetCursor(&LCD1, 0, 0);
				CLCD_WriteString(&LCD1, "    REMOVE CARD");
				CLCD_SetCursor(&LCD1, 0, 1);
				CLCD_WriteString(&LCD1, "=>  CHECK CARD");
				break;
			default:
				CLCD_Clear(&LCD1);
				CLCD_SetCursor(&LCD1, 0, 0);
				CLCD_WriteString(&LCD1, "    CHECK CARD");
				CLCD_SetCursor(&LCD1, 0, 1);
				CLCD_WriteString(&LCD1, "=>  BACK");
				break;
			}
		}
		if (checkbuton(MENU_GPIO_Port, MENU_Pin) != 0)
		{
			exitmenu = 15;
			switch (status)
			{
			case 0:
				CLCD_Clear(&LCD1);
				CLCD_SetCursor(&LCD1, 0, 0);
				CLCD_WriteString(&LCD1, "    SELECT ");
				CLCD_SetCursor(&LCD1, 0, 1);
				CLCD_WriteString(&LCD1, "PLEASE PRESS #");
				uint8_t statusadd = 0;
				uint8_t back = 1;
				while (back == 1)
				{
					if (exitmenu == 0)
					{
						CLCD_Clear(&LCD1);
						HAL_Delay(1000);
						return;
					}
					if (checkbuton(SELECT_GPIO_Port, SELECT_Pin) != 0)
					{
						exitmenu = 15;
						statusadd++;
						statusadd = (statusadd > 2) ? 0 : statusadd;
						switch (statusadd)
						{
						case 1:
							CLCD_Clear(&LCD1);
							CLCD_SetCursor(&LCD1, 0, 0);
							CLCD_WriteString(&LCD1, "=> ADMIN CARD");
							CLCD_SetCursor(&LCD1, 0, 1);
							CLCD_WriteString(&LCD1, "   GUEST CARD");
							break;
						case 2:
							CLCD_Clear(&LCD1);
							CLCD_SetCursor(&LCD1, 0, 0);
							CLCD_WriteString(&LCD1, "   ADMIN CARD");
							CLCD_SetCursor(&LCD1, 0, 1);
							CLCD_WriteString(&LCD1, "=> GUEST CARD");
							break;
						default:
							CLCD_Clear(&LCD1);
							CLCD_SetCursor(&LCD1, 0, 0);
							CLCD_WriteString(&LCD1, "   GUEST CARD");
							CLCD_SetCursor(&LCD1, 0, 1);
							CLCD_WriteString(&LCD1, "=> BACK");
							break;
						}
					}
					if (checkbuton(MENU_GPIO_Port, MENU_Pin) != 0)
					{
						exitmenu = 15;
						switch (statusadd)
						{
						case 1:
							CLCD_Clear(&LCD1);
							CLCD_SetCursor(&LCD1, 0, 0);
							CLCD_WriteString(&LCD1, "=>  ADMIN CARD 1 ");
							CLCD_SetCursor(&LCD1, 0, 1);
							CLCD_WriteString(&LCD1, "    ADMIN CARD 2 ");
							uint8_t statusadd1 = 1;
							uint8_t back11 = 1;
							while (back11 == 1)
							{
								if (exitmenu == 0)
								{
									CLCD_Clear(&LCD1);
									HAL_Delay(1000);
									return;
								}
								if (checkbuton(SELECT_GPIO_Port, SELECT_Pin) != 0)
								{
									exitmenu = 15;
									statusadd1++;
									statusadd1 = (statusadd1 > 4) ? 0 : statusadd1;
									switch (statusadd1)
									{
									case 1:
										CLCD_Clear(&LCD1);
										CLCD_SetCursor(&LCD1, 0, 0);
										CLCD_WriteString(&LCD1, "=>  ADMIN CARD 1 ");
										CLCD_SetCursor(&LCD1, 0, 1);
										CLCD_WriteString(&LCD1, "    ADMIN CARD 2 ");
										break;
									case 2:
										CLCD_Clear(&LCD1);
										CLCD_SetCursor(&LCD1, 0, 0);
										CLCD_WriteString(&LCD1, "=>  ADMIN CARD 2 ");
										CLCD_SetCursor(&LCD1, 0, 1);
										CLCD_WriteString(&LCD1, "    ADMIN CARD 3 ");
										break;
									case 3:
										CLCD_Clear(&LCD1);
										CLCD_SetCursor(&LCD1, 0, 0);
										CLCD_WriteString(&LCD1, "=>  ADMIN CARD 3 ");
										CLCD_SetCursor(&LCD1, 0, 1);
										CLCD_WriteString(&LCD1, "    ADMIN CARD 4 ");
										break;
									case 4:
										CLCD_Clear(&LCD1);
										CLCD_SetCursor(&LCD1, 0, 0);
										CLCD_WriteString(&LCD1, "=>  ADMIN CARD 4 ");
										CLCD_SetCursor(&LCD1, 0, 1);
										CLCD_WriteString(&LCD1, "    BACK ");
										break;
									default:
										CLCD_Clear(&LCD1);
										CLCD_SetCursor(&LCD1, 0, 0);
										CLCD_WriteString(&LCD1, "    ADMIN CARD 4 ");
										CLCD_SetCursor(&LCD1, 0, 1);
										CLCD_WriteString(&LCD1, "=>  BACK");
										break;
									}
								}
								if (checkbuton(MENU_GPIO_Port, MENU_Pin) != 0)
								{
									exitmenu = 15;
									uint8_t keyadd1 = (statusadd << 4) + statusadd1;
									switch (statusadd1)
									{
									case 1:
										if (CheckKey(keyadd1) != 0)
										{
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "ADMIN 1");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "AVAILABLE");
											HAL_Delay(1000);
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "=>  ADMIN CARD 1 ");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "    ADMIN CARD 2 ");
										}
										else
										{
											adduid(keyadd1);
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "=>  ADMIN CARD 1 ");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "    ADMIN CARD 2 ");
										}
										break;
									case 2:
										if (CheckKey(keyadd1) != 0)
										{
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "ADMIN 2");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "AVAILABLE");
											HAL_Delay(1000);
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "=>  ADMIN CARD 2 ");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "    ADMIN CARD 3 ");
										}
										else
										{
											adduid(keyadd1);
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "=>  ADMIN CARD 2 ");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "    ADMIN CARD 3 ");
										}
										break;
									case 3:
										if (CheckKey(keyadd1) != 0)
										{
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "ADMIN 3");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "AVAILABLE");
											HAL_Delay(1000);
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "=>  ADMIN CARD 3 ");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "    ADMIN CARD 4 ");
										}
										else
										{
											adduid(keyadd1);
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "=>  ADMIN CARD 3 ");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "    ADMIN CARD 4 ");
										}
										break;
									case 4:
										if (CheckKey(keyadd1) != 0)
										{
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "ADMIN 4");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "AVAILABLE");
											HAL_Delay(1000);
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "=>  ADMIN CARD 4 ");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "    BACK ");
										}
										else
										{
											adduid(keyadd1);
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "=>  ADMIN CARD 4 ");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "    BACK ");
										}
										break;
									default:
										back11 = 0;
										break;
									}
								}
							}
							CLCD_Clear(&LCD1);
							CLCD_SetCursor(&LCD1, 0, 0);
							CLCD_WriteString(&LCD1, "=> ADMIN CARD");
							CLCD_SetCursor(&LCD1, 0, 1);
							CLCD_WriteString(&LCD1, "   GUEST CARD");
							break;
						case 2:
							CLCD_Clear(&LCD1);
							CLCD_SetCursor(&LCD1, 0, 0);
							CLCD_WriteString(&LCD1, "=>  GUEST CARD 1 ");
							CLCD_SetCursor(&LCD1, 0, 1);
							CLCD_WriteString(&LCD1, "    GUEST CARD 2 ");
							uint8_t statusadd2 = 1;
							uint8_t back12 = 1;
							while (back12 == 1)
							{
								if (exitmenu == 0)
								{
									CLCD_Clear(&LCD1);
									HAL_Delay(1000);
									return;
								}
								if (checkbuton(SELECT_GPIO_Port, SELECT_Pin) != 0)
								{
									exitmenu = 15;
									statusadd2++;
									statusadd2 = (statusadd2 > 4) ? 0 : statusadd2;
									switch (statusadd2)
									{
									case 1:
										CLCD_Clear(&LCD1);
										CLCD_SetCursor(&LCD1, 0, 0);
										CLCD_WriteString(&LCD1, "=>  GUEST CARD 1 ");
										CLCD_SetCursor(&LCD1, 0, 1);
										CLCD_WriteString(&LCD1, "    GUEST CARD 2 ");
										break;
									case 2:
										CLCD_Clear(&LCD1);
										CLCD_SetCursor(&LCD1, 0, 0);
										CLCD_WriteString(&LCD1, "=>  GUEST CARD 2 ");
										CLCD_SetCursor(&LCD1, 0, 1);
										CLCD_WriteString(&LCD1, "    GUEST CARD 3 ");
										break;
									case 3:
										CLCD_Clear(&LCD1);
										CLCD_SetCursor(&LCD1, 0, 0);
										CLCD_WriteString(&LCD1, "=>  GUEST CARD 3 ");
										CLCD_SetCursor(&LCD1, 0, 1);
										CLCD_WriteString(&LCD1, "    GUEST CARD 4 ");
										break;
									case 4:
										CLCD_Clear(&LCD1);
										CLCD_SetCursor(&LCD1, 0, 0);
										CLCD_WriteString(&LCD1, "=>  GUEST CARD 4 ");
										CLCD_SetCursor(&LCD1, 0, 1);
										CLCD_WriteString(&LCD1, "    BACK ");
										break;
									default:
										CLCD_Clear(&LCD1);
										CLCD_SetCursor(&LCD1, 0, 0);
										CLCD_WriteString(&LCD1, "    GUEST CARD 4 ");
										CLCD_SetCursor(&LCD1, 0, 1);
										CLCD_WriteString(&LCD1, "=>  BACK");
										break;
									}
								}
								if (checkbuton(MENU_GPIO_Port, MENU_Pin) != 0)
								{
									exitmenu = 15;
									uint8_t keyadd2 = (statusadd << 4) + statusadd2;
									switch (statusadd2)
									{
									case 1:
										if (CheckKey(keyadd2) != 0)
										{
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "GUEST 1");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "AVAILABLE");
											HAL_Delay(1000);
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "=>  GUEST CARD 1 ");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "    GUEST CARD 2 ");
										}
										else
										{
											adduid(keyadd2);
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "=>  GUEST CARD 1 ");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "    GUEST CARD 2 ");
										}
										break;
									case 2:
										if (CheckKey(keyadd2) != 0)
										{
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "GUEST 2");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "AVAILABLE");
											HAL_Delay(1000);
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "=>  GUEST CARD 2 ");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "    GUEST CARD 3 ");
										}
										else
										{
											adduid(keyadd2);
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "=>  GUEST CARD 2 ");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "    GUEST CARD 3 ");
										}
										break;
									case 3:
										if (CheckKey(keyadd2) != 0)
										{
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "GUEST 3");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "AVAILABLE");
											HAL_Delay(1000);
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "=>  GUEST CARD 3 ");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "    GUEST CARD 4 ");
										}
										else
										{
											adduid(keyadd2);
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "=>  GUEST CARD 3 ");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "    GUEST CARD 4 ");
										}
										break;
									case 4:
										if (CheckKey(keyadd2) != 0)
										{
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "GUEST");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "AVAILABLE");
											HAL_Delay(1000);
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "=>  THE 4 ");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "    BACK ");
										}
										else
										{
											adduid(keyadd2);
											CLCD_Clear(&LCD1);
											CLCD_SetCursor(&LCD1, 0, 0);
											CLCD_WriteString(&LCD1, "=>  GUEST CARD 4 ");
											CLCD_SetCursor(&LCD1, 0, 1);
											CLCD_WriteString(&LCD1, "    BACK ");
										}
										break;
									default:
										back12 = 0;
										break;
									}
								}
							}
							CLCD_Clear(&LCD1);
							CLCD_SetCursor(&LCD1, 0, 0);
							CLCD_WriteString(&LCD1, "   ADMIN CARD");
							CLCD_SetCursor(&LCD1, 0, 1);
							CLCD_WriteString(&LCD1, "=> GUEST CARD");
							break;
						default:
							back = 0;
							break;
						}
					}
				}
				CLCD_Clear(&LCD1);
				CLCD_SetCursor(&LCD1, 0, 0);
				CLCD_WriteString(&LCD1, "=>  ADD CARD");
				CLCD_SetCursor(&LCD1, 0, 1);
				CLCD_WriteString(&LCD1, "    REMOVE CARD");
				break;
			case 1:
				CLCD_Clear(&LCD1);
				CLCD_SetCursor(&LCD1, 0, 0);
				CLCD_WriteString(&LCD1, "     SELECT ");
				CLCD_SetCursor(&LCD1, 0, 1);
				CLCD_WriteString(&LCD1, "PLEASE PRESS #");
				uint8_t statusremove = -1;
				uint8_t backrm = 1;
				while (backrm == 1)
				{
					if (exitmenu == 0)
					{
						CLCD_Clear(&LCD1);
						HAL_Delay(1000);
						return;
					}
					if (checkbuton(SELECT_GPIO_Port, SELECT_Pin) != 0)
					{
						exitmenu = 15;
						statusremove++;
						statusremove = (statusremove > 2) ? 0 : statusremove;
						switch (statusremove)
						{
						case 0:
							CLCD_Clear(&LCD1);
							CLCD_SetCursor(&LCD1, 0, 0);
							CLCD_WriteString(&LCD1, "=> REMOVE CARD");
							CLCD_SetCursor(&LCD1, 0, 1);
							CLCD_WriteString(&LCD1, "   REMOVE ALL");
							break;
						case 1:
							CLCD_Clear(&LCD1);
							CLCD_SetCursor(&LCD1, 0, 0);
							CLCD_WriteString(&LCD1, "   REMOVE CARD");
							CLCD_SetCursor(&LCD1, 0, 1);
							CLCD_WriteString(&LCD1, "=> REMOVE ALL");
							break;
						default:
							CLCD_Clear(&LCD1);
							CLCD_SetCursor(&LCD1, 0, 0);
							CLCD_WriteString(&LCD1, "   REMOVE ALL");
							CLCD_SetCursor(&LCD1, 0, 1);
							CLCD_WriteString(&LCD1, "=> BACK");
							break;
						}
					}
					if (checkbuton(MENU_GPIO_Port, MENU_Pin) != 0)
					{
						exitmenu = 15;
						switch (statusremove)
						{
						case 0:
							CLCD_Clear(&LCD1);
							CLCD_SetCursor(&LCD1, 0, 0);
							CLCD_WriteString(&LCD1, "=> SELECT CARD");
							CLCD_SetCursor(&LCD1, 0, 1);
							CLCD_WriteString(&LCD1, "   SCAN CARD");
							uint8_t statusrm1 = 0;
							uint8_t backrm1 = 1;
							while (backrm1 == 1)
							{
								if (exitmenu == 0)
								{
									CLCD_Clear(&LCD1);
									HAL_Delay(1000);
									return;
								}
								if (checkbuton(SELECT_GPIO_Port, SELECT_Pin) != 0)
								{
									statusrm1++;
									statusrm1 = (statusrm1 > 2) ? 0 : statusrm1;
									switch (statusrm1)
									{
									case 0:
										CLCD_Clear(&LCD1);
										CLCD_SetCursor(&LCD1, 0, 0);
										CLCD_WriteString(&LCD1, "=> SELECT CARD");
										CLCD_SetCursor(&LCD1, 0, 1);
										CLCD_WriteString(&LCD1, "   SCAN CARD");
										break;
									case 1:
										CLCD_Clear(&LCD1);
										CLCD_SetCursor(&LCD1, 0, 0);
										CLCD_WriteString(&LCD1, "   SELECT CARD");
										CLCD_SetCursor(&LCD1, 0, 1);
										CLCD_WriteString(&LCD1, "=> SCAN CARD");
										;
										break;
									default:
										CLCD_Clear(&LCD1);
										CLCD_SetCursor(&LCD1, 0, 0);
										CLCD_WriteString(&LCD1, "   SCAN CARD");
										CLCD_SetCursor(&LCD1, 0, 1);
										CLCD_WriteString(&LCD1, "=> BACK");
										break;
									}
								}
								if (checkbuton(MENU_GPIO_Port, MENU_Pin) != 0)
								{
									exitmenu = 15;
									switch (statusrm1)
									{
									case 0:
										CLCD_Clear(&LCD1);
										CLCD_SetCursor(&LCD1, 0, 0);
										CLCD_WriteString(&LCD1, "=> ADMIN CARD");
										CLCD_SetCursor(&LCD1, 0, 1);
										CLCD_WriteString(&LCD1, "   GUEST CARD");
										uint8_t statusadd = 1;
										uint8_t backrm10 = 1;
										while (backrm10 == 1)
										{
											if (exitmenu == 0)
											{
												CLCD_Clear(&LCD1);
												HAL_Delay(1000);
												return;
											}
											if (checkbuton(SELECT_GPIO_Port, SELECT_Pin) != 0)
											{
												exitmenu = 15;
												statusadd++;
												statusadd = (statusadd > 2) ? 0 : statusadd;
												switch (statusadd)
												{
												case 1:
													CLCD_Clear(&LCD1);
													CLCD_SetCursor(&LCD1, 0, 0);
													CLCD_WriteString(&LCD1, "=> ADMIN CARD");
													CLCD_SetCursor(&LCD1, 0, 1);
													CLCD_WriteString(&LCD1, "   GUEST CARD");
													break;
												case 2:
													CLCD_Clear(&LCD1);
													CLCD_SetCursor(&LCD1, 0, 0);
													CLCD_WriteString(&LCD1, "   ADMIN CARD");
													CLCD_SetCursor(&LCD1, 0, 1);
													CLCD_WriteString(&LCD1, "=> GUEST CARD");
													break;
												default:
													CLCD_Clear(&LCD1);
													CLCD_SetCursor(&LCD1, 0, 0);
													CLCD_WriteString(&LCD1, "   GUEST CARD");
													CLCD_SetCursor(&LCD1, 0, 1);
													CLCD_WriteString(&LCD1, "=> BACK");
													break;
												}
											}
											if (checkbuton(MENU_GPIO_Port, MENU_Pin) != 0)
											{
												exitmenu = 15;
												switch (statusadd)
												{
												case 1:
													CLCD_Clear(&LCD1);
													CLCD_SetCursor(&LCD1, 0, 0);
													CLCD_WriteString(&LCD1, "=>  ADMIN CARD 1 ");
													CLCD_SetCursor(&LCD1, 0, 1);
													CLCD_WriteString(&LCD1, "    ADMIN CARD 2 ");
													uint8_t statusadd1 = 1;
													uint8_t back11 = 1;
													while (back11 == 1)
													{
														if (exitmenu == 0)
														{
															CLCD_Clear(&LCD1);
															HAL_Delay(1000);
															return;
														}
														if (checkbuton(SELECT_GPIO_Port, SELECT_Pin) != 0)
														{
															exitmenu = 15;
															statusadd1++;
															statusadd1 = (statusadd1 > 4) ? 0 : statusadd1;
															switch (statusadd1)
															{
															case 1:
																CLCD_Clear(&LCD1);
																CLCD_SetCursor(&LCD1, 0, 0);
																CLCD_WriteString(&LCD1, "=>  ADMIN CARD 1 ");
																CLCD_SetCursor(&LCD1, 0, 1);
																CLCD_WriteString(&LCD1, "    ADMIN CARD 2 ");
																break;
															case 2:
																CLCD_Clear(&LCD1);
																CLCD_SetCursor(&LCD1, 0, 0);
																CLCD_WriteString(&LCD1, "=>  ADMIN CARD 2 ");
																CLCD_SetCursor(&LCD1, 0, 1);
																CLCD_WriteString(&LCD1, "    ADMIN CARD 3 ");
																break;
															case 3:
																CLCD_Clear(&LCD1);
																CLCD_SetCursor(&LCD1, 0, 0);
																CLCD_WriteString(&LCD1, "=>  ADMIN CARD 3 ");
																CLCD_SetCursor(&LCD1, 0, 1);
																CLCD_WriteString(&LCD1, "    ADMIN CARD 4 ");
																break;
															case 4:
																CLCD_Clear(&LCD1);
																CLCD_SetCursor(&LCD1, 0, 0);
																CLCD_WriteString(&LCD1, "=>  ADMIN CARD 4 ");
																CLCD_SetCursor(&LCD1, 0, 1);
																CLCD_WriteString(&LCD1, "    BACK ");
																break;
															default:
																CLCD_Clear(&LCD1);
																CLCD_SetCursor(&LCD1, 0, 0);
																CLCD_WriteString(&LCD1, "    ADMIN CARD 4 ");
																CLCD_SetCursor(&LCD1, 0, 1);
																CLCD_WriteString(&LCD1, "=>  BACK");
																break;
															}
														}
														if (checkbuton(MENU_GPIO_Port, MENU_Pin) != 0)
														{
															exitmenu = 15;
															uint8_t keyadd1 = (statusadd << 4) + statusadd1;
															switch (statusadd1)
															{
															case 1:
																if (CheckKey(keyadd1) == 0)
																{
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "NO ADMIN CARD 1 ");
																	HAL_Delay(1000);
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "=>  ADMIN CARD 1 ");
																	CLCD_SetCursor(&LCD1, 0, 1);
																	CLCD_WriteString(&LCD1, "    ADMIN CARD 2 ");
																}
																else
																{
																	removeuid(CheckKey(keyadd1));
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "  CARD 1 DELETION ");
																	CLCD_SetCursor(&LCD1, 0, 1);
																	CLCD_WriteString(&LCD1, "   SUCCESSFUL  ");
																	HAL_Delay(1000);
																	if (checkcountUID() == 0)
																	{
																		startadd();
																		exitmenu = 0;
																	}
																	else
																	{
																		CLCD_Clear(&LCD1);
																		CLCD_SetCursor(&LCD1, 0, 0);
																		CLCD_WriteString(&LCD1, "=>  ADMIN CARD 1 ");
																		CLCD_SetCursor(&LCD1, 0, 1);
																		CLCD_WriteString(&LCD1, "    ADMIN CARD 2 ");
																	}
																}
																break;
															case 2:
																if (CheckKey(keyadd1) == 0)
																{
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "NO ADMIN CARD 2 ");
																	HAL_Delay(1000);
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "=>  ADMIN CARD 2 ");
																	CLCD_SetCursor(&LCD1, 0, 1);
																	CLCD_WriteString(&LCD1, "    ADMIN CARD 3 ");
																}
																else
																{
																	removeuid(CheckKey(keyadd1));
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "  CARD 2 DELETION ");
																	CLCD_SetCursor(&LCD1, 0, 1);
																	CLCD_WriteString(&LCD1, "   SUCCESSFUL  ");
																	HAL_Delay(1000);
																	if (checkcountUID() == 0)
																	{
																		startadd();
																		exitmenu = 0;
																	}
																	else
																	{
																		CLCD_Clear(&LCD1);
																		CLCD_SetCursor(&LCD1, 0, 0);
																		CLCD_WriteString(&LCD1, "=>  ADMIN CARD 2 ");
																		CLCD_SetCursor(&LCD1, 0, 1);
																		CLCD_WriteString(&LCD1, "    ADMIN CARD 3 ");
																	}
																}
																break;
															case 3:
																if (CheckKey(keyadd1) == 0)
																{
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "NO ADMIN CARD 3 ");
																	HAL_Delay(1000);
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "=>  ADMIN CARD 3 ");
																	CLCD_SetCursor(&LCD1, 0, 1);
																	CLCD_WriteString(&LCD1, "    ADMIN CARD 4 ");
																}
																else
																{
																	removeuid(CheckKey(keyadd1));
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "  CARD 3 DELETION ");
																	CLCD_SetCursor(&LCD1, 0, 1);
																	CLCD_WriteString(&LCD1, "   SUCCESSFUL  ");
																	HAL_Delay(1000);
																	if (checkcountUID() == 0)
																	{
																		startadd();
																		exitmenu = 0;
																	}
																	else
																	{
																		CLCD_Clear(&LCD1);
																		CLCD_SetCursor(&LCD1, 0, 0);
																		CLCD_WriteString(&LCD1, "=>  ADMIN CARD 3 ");
																		CLCD_SetCursor(&LCD1, 0, 1);
																		CLCD_WriteString(&LCD1, "    ADMIN CARD 4 ");
																	}
																}
																break;
															case 4:
																if (CheckKey(keyadd1) == 0)
																{
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "NO ADMIN CARD 4 ");
																	HAL_Delay(1000);
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "=>  ADMIN CARD 4 ");
																	CLCD_SetCursor(&LCD1, 0, 1);
																	CLCD_WriteString(&LCD1, "    BACK ");
																}
																else
																{
																	removeuid(CheckKey(keyadd1));
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "  CARD 4 DELETION ");
																	CLCD_SetCursor(&LCD1, 0, 1);
																	CLCD_WriteString(&LCD1, "   SUCCESSFUL  ");
																	HAL_Delay(1000);
																	if (checkcountUID() == 0)
																	{
																		startadd();
																		exitmenu = 0;
																	}
																	else
																	{
																		CLCD_Clear(&LCD1);
																		CLCD_SetCursor(&LCD1, 0, 0);
																		CLCD_WriteString(&LCD1, "=>  ADMIN CARD 4 ");
																		CLCD_SetCursor(&LCD1, 0, 1);
																		CLCD_WriteString(&LCD1, "    BACK ");
																	}
																}
																break;
															default:
																back11 = 0;
																break;
															}
														}
													}
													CLCD_Clear(&LCD1);
													CLCD_SetCursor(&LCD1, 0, 0);
													CLCD_WriteString(&LCD1, "=> ADMIN CARD");
													CLCD_SetCursor(&LCD1, 0, 1);
													CLCD_WriteString(&LCD1, "   GUEST CARD");
													break;
												case 2:
													CLCD_Clear(&LCD1);
													CLCD_SetCursor(&LCD1, 0, 0);
													CLCD_WriteString(&LCD1, "=>  GUEST CARD 1 ");
													CLCD_SetCursor(&LCD1, 0, 1);
													CLCD_WriteString(&LCD1, "    GUEST CARD 2 ");
													uint8_t statusadd2 = 1;
													uint8_t back12 = 1;
													while (back12 == 1)
													{
														if (exitmenu == 0)
														{
															CLCD_Clear(&LCD1);
															HAL_Delay(1000);
															return;
														}
														if (checkbuton(SELECT_GPIO_Port, SELECT_Pin) != 0)
														{
															exitmenu = 15;
															statusadd2++;
															statusadd2 = (statusadd2 > 4) ? 0 : statusadd2;
															switch (statusadd2)
															{
															case 1:
																CLCD_Clear(&LCD1);
																CLCD_SetCursor(&LCD1, 0, 0);
																CLCD_WriteString(&LCD1, "=>  GUEST CARD 1 ");
																CLCD_SetCursor(&LCD1, 0, 1);
																CLCD_WriteString(&LCD1, "    GUEST CARD 2 ");
																break;
															case 2:
																CLCD_Clear(&LCD1);
																CLCD_SetCursor(&LCD1, 0, 0);
																CLCD_WriteString(&LCD1, "=>  GUEST CARD 2 ");
																CLCD_SetCursor(&LCD1, 0, 1);
																CLCD_WriteString(&LCD1, "    GUEST CARD 3 ");
																break;
															case 3:
																CLCD_Clear(&LCD1);
																CLCD_SetCursor(&LCD1, 0, 0);
																CLCD_WriteString(&LCD1, "=>  GUEST CARD 3 ");
																CLCD_SetCursor(&LCD1, 0, 1);
																CLCD_WriteString(&LCD1, "    GUEST CARD 4 ");
																break;
															case 4:
																CLCD_Clear(&LCD1);
																CLCD_SetCursor(&LCD1, 0, 0);
																CLCD_WriteString(&LCD1, "=>  GUEST CARD 4 ");
																CLCD_SetCursor(&LCD1, 0, 1);
																CLCD_WriteString(&LCD1, "    BACK ");
																break;
															default:
																CLCD_Clear(&LCD1);
																CLCD_SetCursor(&LCD1, 0, 0);
																CLCD_WriteString(&LCD1, "    GUEST CARD 4 ");
																CLCD_SetCursor(&LCD1, 0, 1);
																CLCD_WriteString(&LCD1, "=>  BACK");
																break;
															}
														}
														if (checkbuton(MENU_GPIO_Port, MENU_Pin) != 0)
														{
															exitmenu = 15;
															uint8_t keyadd2 = (statusadd << 4) + statusadd2;
															switch (statusadd2)
															{
															case 1:
																if (CheckKey(keyadd2) == 0)
																{
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "NO GUEST CARD 1 ");
																	HAL_Delay(1000);
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "=>  GUEST CARD 1 ");
																	CLCD_SetCursor(&LCD1, 0, 1);
																	CLCD_WriteString(&LCD1, "    GUEST CARD 2 ");
																}
																else
																{
																	removeuid(CheckKey(keyadd2));
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "  CARD 1 DELETION ");
																	CLCD_SetCursor(&LCD1, 0, 1);
																	CLCD_WriteString(&LCD1, "   SUCCESSFUL  ");
																	HAL_Delay(1000);
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "=>  GUEST CARD 1 ");
																	CLCD_SetCursor(&LCD1, 0, 1);
																	CLCD_WriteString(&LCD1, "    GUEST CARD 2 ");
																}
																break;
															case 2:
																if (CheckKey(keyadd2) == 0)
																{
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "NO GUEST CARD 2 ");
																	HAL_Delay(1000);
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "=>  GUEST CARD 2 ");
																	CLCD_SetCursor(&LCD1, 0, 1);
																	CLCD_WriteString(&LCD1, "    GUEST CARD 3 ");
																}
																else
																{
																	removeuid(CheckKey(keyadd2));
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "  CARD 2 DELETION ");
																	CLCD_SetCursor(&LCD1, 0, 1);
																	CLCD_WriteString(&LCD1, "   SUCCESSFUL  ");
																	HAL_Delay(1000);
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "=>  GUEST CARD 2 ");
																	CLCD_SetCursor(&LCD1, 0, 1);
																	CLCD_WriteString(&LCD1, "    GUEST CARD 3 ");
																}
																break;
															case 3:
																if (CheckKey(keyadd2) == 0)
																{
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "NO GUEST CARD 3 ");
																	HAL_Delay(1000);
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "=>  GUEST CARD 3 ");
																	CLCD_SetCursor(&LCD1, 0, 1);
																	CLCD_WriteString(&LCD1, "    GUEST CARD 4 ");
																}
																else
																{
																	removeuid(CheckKey(keyadd2));
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "  CARD 3 DELETION ");
																	CLCD_SetCursor(&LCD1, 0, 1);
																	CLCD_WriteString(&LCD1, "   SUCCESSFUL  ");
																	HAL_Delay(1000);
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "=>  GUEST CARD 3 ");
																	CLCD_SetCursor(&LCD1, 0, 1);
																	CLCD_WriteString(&LCD1, "    GUEST CARD 4 ");
																}
																break;
															case 4:
																if (CheckKey(keyadd2) == 0)
																{
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "NO GUEST CARD 4 ");
																	HAL_Delay(1000);
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "=>  GUEST CARD 4 ");
																	CLCD_SetCursor(&LCD1, 0, 1);
																	CLCD_WriteString(&LCD1, "    BACK ");
																}
																else
																{
																	removeuid(CheckKey(keyadd2));
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "  CARD 4 DELETION ");
																	CLCD_SetCursor(&LCD1, 0, 1);
																	CLCD_WriteString(&LCD1, "   SUCCESSFUL  ");
																	HAL_Delay(1000);
																	CLCD_Clear(&LCD1);
																	CLCD_SetCursor(&LCD1, 0, 0);
																	CLCD_WriteString(&LCD1, "=>  GUEST CARD 4 ");
																	CLCD_SetCursor(&LCD1, 0, 1);
																	CLCD_WriteString(&LCD1, "    BACK ");
																}
																break;
															default:
																back12 = 0;
																break;
															}
														}
													}
													CLCD_Clear(&LCD1);
													CLCD_SetCursor(&LCD1, 0, 0);
													CLCD_WriteString(&LCD1, "   ADMIN CARD");
													CLCD_SetCursor(&LCD1, 0, 1);
													CLCD_WriteString(&LCD1, "=> GUEST CARD");
													break;
												default:
													backrm10 = 0;
													break;
												}
											}
										}
										CLCD_Clear(&LCD1);
										CLCD_SetCursor(&LCD1, 0, 0);
										CLCD_WriteString(&LCD1, "=> SELECT CARD");
										CLCD_SetCursor(&LCD1, 0, 1);
										CLCD_WriteString(&LCD1, "   SCAN CARD");
										break;
									case 1:
										CLCD_Clear(&LCD1);
										CLCD_SetCursor(&LCD1, 0, 0);
										CLCD_WriteString(&LCD1, "SCAN CARD");
										CLCD_SetCursor(&LCD1, 0, 1);
										CLCD_WriteString(&LCD1, "=>  BACK");
										uint8_t rmquet = 1;
										while (rmquet)
										{
											if (TM_MFRC522_Check(CardID) == MI_OK)
											{
												if (CheckListUID(CardID) != 0)
												{
													removeuid(CheckKey(CheckListUID(CardID)));
													CLCD_Clear(&LCD1);
													CLCD_SetCursor(&LCD1, 0, 0);
													CLCD_WriteString(&LCD1, "  CARD DELETION ");
													CLCD_SetCursor(&LCD1, 0, 1);
													CLCD_WriteString(&LCD1, "   SUCCESSFUL  ");
													HAL_Delay(1000);
													if (checkcountUID() == 0)
													{
														startadd();
														rmquet = 1;
														exitmenu = 0;
														return;
													}else{
														CLCD_Clear(&LCD1);
														CLCD_SetCursor(&LCD1, 0, 0);
														CLCD_WriteString(&LCD1, "SCAN CARD");
														CLCD_SetCursor(&LCD1, 0, 1);
														CLCD_WriteString(&LCD1, "=>  BACK");
													}

												}
												else
												{
													CLCD_Clear(&LCD1);
													CLCD_SetCursor(&LCD1, 0, 0);
													CLCD_WriteString(&LCD1, "CARD UNAVAILABLE");
													HAL_Delay(1000);
													CLCD_Clear(&LCD1);
													CLCD_SetCursor(&LCD1, 0, 0);
													CLCD_WriteString(&LCD1, "SCAN CARD");
													CLCD_SetCursor(&LCD1, 0, 1);
													CLCD_WriteString(&LCD1, "=>  BACK");
												}
											}
											if (checkbuton(MENU_GPIO_Port, MENU_Pin) == 1)
											{
												rmquet = 0;
											}
										}
										CLCD_Clear(&LCD1);
										CLCD_SetCursor(&LCD1, 0, 0);
										CLCD_WriteString(&LCD1, "   SELECT CARD");
										CLCD_SetCursor(&LCD1, 0, 1);
										CLCD_WriteString(&LCD1, "=> SCAN CARD");
										break;
									default:
										backrm1 = 0;
										break;
									}
								}
							}
							CLCD_Clear(&LCD1);
							CLCD_SetCursor(&LCD1, 0, 0);
							CLCD_WriteString(&LCD1, "=> REMOVE CARD");
							CLCD_SetCursor(&LCD1, 0, 1);
							CLCD_WriteString(&LCD1, "   REMOVE ALL");
							break;
						case 1:
							resetflash();
							startadd();
							exitmenu = 0;
							break;
						default:
							backrm = 0;
							break;
						}
					}
				}
				CLCD_Clear(&LCD1);
				CLCD_SetCursor(&LCD1, 0, 0);
				CLCD_WriteString(&LCD1, "    ADD CARD");
				CLCD_SetCursor(&LCD1, 0, 1);
				CLCD_WriteString(&LCD1, "=>  REMOVE CARD");
				break;
			case 2:
				checkthe();
				CLCD_Clear(&LCD1);
				CLCD_SetCursor(&LCD1, 0, 0);
				CLCD_WriteString(&LCD1, "    REMOVE CARD");
				CLCD_SetCursor(&LCD1, 0, 1);
				CLCD_WriteString(&LCD1, "=>  CHECK CARD");
				break;
				break;
			default:
				exitmenu = 0;
				break;
			}
		}
	}
	CLCD_Clear(&LCD1);
	HAL_Delay(1000);
}
uint8_t CheckUID(uint8_t *data, uint32_t address)
{
	uint8_t arr[8];
	Flash_Read_Array(address, arr, 8);
	if (arr[6] != 0xFF)
		return 0;
	for (uint8_t i = 0; i < 5; i++)
	{
		if (data[i] != arr[i])
			return 0;
	}
	return 1;
}

uint8_t CheckListUID(uint8_t *data)
{
	uint32_t pt = StartAddressUID;
	while (Flash_Read_Byte(pt + 5) != 0xFF)
	{
		if(Flash_Read_2Byte(pt + 6) == 0xFFFF){
			if (CheckUID(data, pt) == 1)
				return *(uint8_t *)(pt + 5);
		}
		pt = pt + 8;
	}
	return 0;
}

uint8_t checkcountUID(void)
{
	uint32_t pt = StartAddressUID;
	uint8_t count = 0;
	while (Flash_Read_Byte(pt + 5) != 0xFF)
	{
		if(Flash_Read_2Byte(pt + 6) == 0xFFFF){
			if ((Flash_Read_Byte(pt + 5) >> 4) == 1)
			{
				count++;
			}
		}
		pt = pt + 8;
	}
	return count;
}

void adduid(uint8_t key)
{
	setaddress();
	CLCD_Clear(&LCD1);
	CLCD_SetCursor(&LCD1, 0, 0);
	CLCD_WriteString(&LCD1, "SCAN CARD");
	CLCD_SetCursor(&LCD1, 0, 1);
	CLCD_WriteString(&LCD1, "=>  BACK");
	while (exitmenu)
	{
		if (TM_MFRC522_Check(CardID) == MI_OK)
		{
			HAL_Delay(delayloa);
			if (CheckListUID(CardID) == 0)
			{
				CardID[5] = key;
				Flash_Write_Array(AddressUID, CardID, 6);
				AddressUID += 8;
				CLCD_Clear(&LCD1);
				CLCD_SetCursor(&LCD1, 0, 0);
				CLCD_WriteString(&LCD1, "SUCCESSFUL");
				HAL_Delay(1000);
				return;
			}
			else
			{
				CLCD_Clear(&LCD1);
				CLCD_SetCursor(&LCD1, 0, 0);
				CLCD_WriteString(&LCD1, "CARD AVAILABLE");
				HAL_Delay(1000);
				CLCD_Clear(&LCD1);
				CLCD_SetCursor(&LCD1, 0, 0);
				CLCD_WriteString(&LCD1, "SCAN CARD");
				CLCD_SetCursor(&LCD1, 0, 1);
				CLCD_WriteString(&LCD1, "=>  BACK");
			}
		}
		if (checkbuton(MENU_GPIO_Port, MENU_Pin) == 1)
		{
			return;
		}
	}
}
void checkuser(void)
{
	uint8_t key = CheckListUID(CardID);
	uint8_t key2 = key & 0x0f;
	uint8_t key1 = key >> 4;
	if(key1==1)
	{
		switch(key2)
		{
		case 1:
			HAL_UART_Transmit(&huart1, admin1, sizeof(admin1)-1, HAL_MAX_DELAY);
			break;
		case 2:
			HAL_UART_Transmit(&huart1, admin2, sizeof(admin2)-1, HAL_MAX_DELAY);
			break;
		case 3:
			HAL_UART_Transmit(&huart1, admin3, sizeof(admin3)-1, HAL_MAX_DELAY);
			break;
		default:
			HAL_UART_Transmit(&huart1, admin4, sizeof(admin4)-1, HAL_MAX_DELAY);
			break;
		}
	}
	if(key1!=1)
	{
		switch(key2)
		{
		case 1:
			HAL_UART_Transmit(&huart1, guest1, sizeof(guest1)-1, HAL_MAX_DELAY);
			break;
		case 2:
			HAL_UART_Transmit(&huart1, guest2, sizeof(guest2)-1, HAL_MAX_DELAY);
			break;
		case 3:
			HAL_UART_Transmit(&huart1, guest3, sizeof(guest3)-1, HAL_MAX_DELAY);
			break;
		default:
			HAL_UART_Transmit(&huart1, guest4, sizeof(guest4)-1, HAL_MAX_DELAY);
			break;
		}
	}

}
void checkthe(void)
{
	exitmenu = 30;
	CLCD_Clear(&LCD1);
	CLCD_SetCursor(&LCD1, 0, 0);
	CLCD_WriteString(&LCD1, "SCAN CARD");
	CLCD_SetCursor(&LCD1, 0, 1);
	CLCD_WriteString(&LCD1, "=>  BACK");
	while (exitmenu )
	{
		if (TM_MFRC522_Check(CardID) == MI_OK)
		{
			if (CheckListUID(CardID) == 0)
			{
				CLCD_Clear(&LCD1);
				CLCD_SetCursor(&LCD1, 0, 0);
				CLCD_WriteString(&LCD1, "CARD NOT ADDED");
				HAL_Delay(1000);
				CLCD_Clear(&LCD1);
				CLCD_SetCursor(&LCD1, 0, 0);
				CLCD_WriteString(&LCD1, "SCAN CARD");
				CLCD_SetCursor(&LCD1, 0, 1);
				CLCD_WriteString(&LCD1, "=>  BACK");
				HAL_Delay(1000);
			}
			else
			{
				uint8_t key = CheckListUID(CardID);
				uint8_t key2 = key & 0x0f;
				uint8_t key1 = key >> 4;
				CLCD_Clear(&LCD1);
				switch (key1)
				{
				case 1:
					CLCD_SetCursor(&LCD1, 0, 0);
					CLCD_WriteString(&LCD1, "ADMIN CARD");
					break;
				default:
					CLCD_SetCursor(&LCD1, 0, 0);
					CLCD_WriteString(&LCD1, "GUEST CARD");
					break;
				}
				switch (key2)
				{
				case 1:
					CLCD_SetCursor(&LCD1, 0, 1);
					CLCD_WriteString(&LCD1, "CARD 1");
					break;
				case 2:
					CLCD_SetCursor(&LCD1, 0, 1);
					CLCD_WriteString(&LCD1, "CARD 2");
					break;
				case 3:
					CLCD_SetCursor(&LCD1, 0, 1);
					CLCD_WriteString(&LCD1, "CARD 3");
					break;
				default:
					CLCD_SetCursor(&LCD1, 0, 1);
					CLCD_WriteString(&LCD1, "CARD 4");
					break;
				}
				HAL_Delay(1000);
				CLCD_Clear(&LCD1);
				CLCD_SetCursor(&LCD1, 0, 0);
				CLCD_WriteString(&LCD1, "SCAN CARD");
				CLCD_SetCursor(&LCD1, 0, 1);
				CLCD_WriteString(&LCD1, "=>  BACK");
			}
		}
		if (checkbuton(MENU_GPIO_Port, MENU_Pin) == 1)
		{
			return;
		}
	}
}
uint32_t CheckKey(uint8_t key)
{
	uint32_t pt = StartAddressUID;
	while (Flash_Read_Byte(pt + 5) != 0xFF)
	{
		if(Flash_Read_2Byte(pt + 6) == 0xFFFF){
			if (*(uint8_t *)(pt + 5) == key)
				return pt;
		}
		pt = pt + 8;
	}
	return 0;
}
void removeuid(uint32_t addressrm)
{
	Flash_Write_2Byte(addressrm + 6, 0x0000);
}
void startadd(void)
{
	CLCD_Clear(&LCD1);
	CLCD_SetCursor(&LCD1, 0, 0);
	CLCD_WriteString(&LCD1, "SCAN CARD");
	CLCD_SetCursor(&LCD1, 0, 1);
	CLCD_WriteString(&LCD1, "ADMIN CARD");
	setaddress();
	while (1)
		{
			if (TM_MFRC522_Check(CardID) == MI_OK)
			{
				if (CheckListUID(CardID) == 0)
				{
					CardID[5] = 0x11;
					Flash_Write_Array(AddressUID, CardID, 6);
					AddressUID += 8;
					break;
				}
				else
				{
					CLCD_Clear(&LCD1);
					CLCD_SetCursor(&LCD1, 0, 0);
					CLCD_WriteString(&LCD1, "CARD AVAILABLE");
					HAL_Delay(1000);
					CLCD_Clear(&LCD1);
					CLCD_SetCursor(&LCD1, 0, 0);
					CLCD_WriteString(&LCD1, "SCAN CARD");
					CLCD_SetCursor(&LCD1, 0, 1);
					CLCD_WriteString(&LCD1, "ADMIN CARD");
				}
			}
		}
	CLCD_Clear(&LCD1);
	CLCD_SetCursor(&LCD1, 0, 0);
	CLCD_WriteString(&LCD1, "ADD SUCCESSFUL");
	CLCD_SetCursor(&LCD1, 0, 1);
	CLCD_WriteString(&LCD1, "ADMIN CARD");
	HAL_Delay(1000);
	CLCD_Clear(&LCD1);
}
void setaddress(void){
	uint32_t pt = StartAddressUID;
	while (Flash_Read_Byte(pt + 5) != 0xFF)
	{
		pt = pt + 8;
	}
	AddressUID = pt;
}
void password_keypad(void){
	CLCD_Clear(&LCD1);
	  CLCD_SetCursor(&LCD1, 0, 0);
	  CLCD_WriteString(&LCD1, "PASSWORD :");
	  exitmenu = 120;

	  uint8_t pass[16] = {-1};
	  uint8_t k = 0;

	  HAL_Delay(1000);
//	  while(checkbuton(MENU_GPIO_Port, MENU_Pin) != 0){}
		CLCD_SetCursor(&LCD1, k, 1);
		CLCD_WriteChar(&LCD1, 95);
	  while (exitmenu) {
	    // Read key from keypad
	    char key = KEYPAD_Read();

	    if (key != '#' && key != '*' && key != KEYPAD_NOT_PRESSED) {
	      CLCD_SetCursor(&LCD1, k, 1);
	      CLCD_WriteChar(&LCD1, '*');

	      pass[k] = key ;
	      k++;

	      if (k == 6) {
	        for (uint8_t i = 0; i < 6; i++) {
	          if (pass[i] != PassWord[i])
	          {
	           return;
	          }
	        }


//	        resetflash();
//	        startadd();
	    	HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, 1);
			CLCD_Clear(&LCD1);
			CLCD_SetCursor(&LCD1, 0, 0);
			CLCD_WriteString(&LCD1, "    WELCOME");
			HAL_Delay(500);
			HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, 0);
			HAL_UART_Transmit(&huart1, opened, sizeof(opened)-1, HAL_MAX_DELAY);
			HAL_Delay(4000);
			checkuser();
	        exitmenu = 0;
	      }

	      CLCD_SetCursor(&LCD1, k, 1);
	      CLCD_WriteChar(&LCD1, 95);
	      while(KEYPAD_Read() != KEYPAD_NOT_PRESSED );
	    }
	  }
}
void password_button(void){
	CLCD_Clear(&LCD1);
	CLCD_SetCursor(&LCD1, 0, 0);
	CLCD_WriteString(&LCD1, "NHAP PASSWORD :");
	exitmenu =120;
	uint8_t pass[16] = {-1};
	uint8_t k=0;
	uint8_t ascii = -1;
	HAL_Delay(1000);
	CLCD_SetCursor(&LCD1, k, 1);
	CLCD_WriteChar(&LCD1, 95);
	while(exitmenu){
		if(checkbuton(SELECT_GPIO_Port, SELECT_Pin) == 1){
			ascii++;
			if(ascii == 10)	ascii = 0;
			CLCD_SetCursor(&LCD1, k, 1);
			CLCD_WriteChar(&LCD1, ascii + 0x30);
		}
		if(checkbuton(MENU_GPIO_Port, MENU_Pin) == 1){
			CLCD_SetCursor(&LCD1, k, 1);
			CLCD_WriteChar(&LCD1, '*');
			pass[k] = ascii;
			k = k+1;
			ascii = -1;
			if(k == 6){
				for(uint8_t i = 0; i<6; i++){
					if(pass[i] != PassWord[i]){
						CLCD_Clear(&LCD1);
						CLCD_SetCursor(&LCD1, 0, 0);
						CLCD_WriteString(&LCD1, "WRONG PASSWORD!!");
						HAL_UART_Transmit(&huart1, wrongpass, sizeof(wrongpass)-1, HAL_MAX_DELAY);
						HAL_Delay(3000);
						return;
					}
				}
			HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, 1);
			CLCD_Clear(&LCD1);
			CLCD_SetCursor(&LCD1, 0, 0);
			CLCD_WriteString(&LCD1, "    WELCOME");
			HAL_Delay(500);
			HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, 0);
			HAL_UART_Transmit(&huart1, opened, sizeof(opened)-1, HAL_MAX_DELAY);
			HAL_Delay(4000);
	        exitmenu = 0;
			}
			CLCD_SetCursor(&LCD1, k, 1);
			CLCD_WriteChar(&LCD1, 95);
			while(checkbuton(SELECT_GPIO_Port, SELECT_Pin) != KEYPAD_NOT_PRESSED );

		}

	}

}
//void password(void){
//	CLCD_Clear(&LCD1);
//	CLCD_SetCursor(&LCD1, 0, 0);
//	CLCD_WriteString(&LCD1, "NHAP PASSWORD :");
//	exitmenu =120;
//	uint8_t pass[16] = {-1};
//	uint8_t k=0;
//	uint8_t pas = -1;
//	HAL_Delay(1000);
//	while(checkbuton(MENU_GPIO_Port, MENU_Pin) != 0){}
//	CLCD_SetCursor(&LCD1, k, 1);
//	CLCD_WriteChar(&LCD1, 95);
//	while(exitmenu){
//		if(checkbuton(SELECT_GPIO_Port, SELECT_Pin) == 1){
//			pas++;
//			if(pas == 10)	pas = 0;
//			CLCD_SetCursor(&LCD1, k, 1);
//			CLCD_WriteChar(&LCD1, pas + 0x30);
//		}
//		if(checkbuton(MENU_GPIO_Port, MENU_Pin) == 1){
//			pass[k] = pas;
//			k = k+1;
//			pas = -1;
//			if(k == 16){
//				for(uint8_t i = 0; i<16; i++){
//					if(pass[i] != PassWord[i]){
//						return;
//					}
//				}
//				HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, 1);
//				HAL_Delay(5000);
//				HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, 0);
//				resetflash();
//				startadd();
//				exitmenu =0;
//			}
//			CLCD_SetCursor(&LCD1, k, 1);
//			CLCD_WriteChar(&LCD1, 95);
//		}
//
//	}
//
//}

void resetflash()
{
	uint32_t pt = StartAddressUID;
	while(Flash_Read_8Byte(pt) != 0xFFFFFFFFFFFFFFFF){
		Flash_Erase(pt);
	}
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

#ifdef  USE_FULL_ASSERT
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
