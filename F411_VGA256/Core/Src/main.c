/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "usb_host.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbh_hid.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vga256.h"
#include "flower.h"
#include "color.h"
#include "rgb.h"
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
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim9;
TIM_HandleTypeDef htim10;
DMA_HandleTypeDef hdma_tim1_up;

/* USER CODE BEGIN PV */
uint16_t VOFFSET=0;
uint16_t vga_stop=0;
int16_t line=0;
uint16_t firstTrig=1;

uint16_t vga_voff11=0;		//offset for 1st line
uint16_t vga_voff12=VGA_LBUFFERSIZE;
uint16_t vga_voff21=VGA_LBUFFERSIZE+VGA_LBUFFERSIZE;
uint16_t vga_voff22=VGA_LBUFFERSIZE+VGA_LBUFFERSIZE+VGA_LBUFFERSIZE;
uint32_t GPIOB_ODR;

char kBuffer[16];
int kBin=0;
int kBout=0;
char msg[64];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM10_Init(void);
static void MX_TIM9_Init(void);
static void MX_TIM1_Init(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */
extern void tetris();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void VGA_update(){
	   vga_voff11=VOFFSET;		//offset for 1st line
	   vga_voff12=VOFFSET+VGA_LBUFFERSIZE;
	   vga_voff21=VOFFSET+VGA_LBUFFERSIZE+VGA_LBUFFERSIZE;
	   vga_voff22=VOFFSET+VGA_LBUFFERSIZE+VGA_LBUFFERSIZE+VGA_LBUFFERSIZE;
}

void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin){
	if(GPIO_Pin==GPIO_PIN_9){
		if(VOFFSET>0)VOFFSET-=1;
		vga_stop=1;
		VGA_update();
	}
	else if(GPIO_Pin==GPIO_PIN_10){
		if(VOFFSET<(VGA_LBUFFERSIZE-VGA_WIDTH))VOFFSET+=1;
		vga_stop=1;
		VGA_update();
	}
	   sprintf(msg,"%d ",VOFFSET);
	   SetCursor(3,50);
	   WriteString(msg,Font_7x10,VGA_WHITE);

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
     if(htim==&htim9){
		line=-14;
		if(firstTrig){
			if(
            HAL_DMA_Start_IT(&hdma_tim1_up,(uint32_t)VGA_obuffer,GPIOB_ODR,VGA_FULL)
			!= HAL_OK){
				while(1){
					HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
					HAL_Delay(1000);
				}
			}
			firstTrig=0;
		}
	}
}

static void DMA_HalfCpltCallback(DMA_HandleTypeDef *hdma){
     //fill in line1
	if((line<0)||(line>=VGA_VBUFFER)){
		memset((uint8_t *)VGA_obuffer,0,VGA_FULL);
	}else{
	memcpy((uint8_t *)VGA_obuffer + vga_voff11,VGA_buffer[line],VGA_LBUFFER);
	memcpy((uint8_t *)VGA_obuffer + vga_voff12,VGA_buffer[line],VGA_LBUFFER);
	}
	line++;
	if(vga_stop){
		HAL_DMA_Abort_IT(&hdma_tim1_up);
		firstTrig=1;
		vga_stop=0;
		VGA_update();
	}
	if(line>=VGA_VBUFFER && !firstTrig){
		HAL_DMA_Abort_IT(&hdma_tim1_up);
		firstTrig=1;
	}
}

static void DMA_CpltCallback(DMA_HandleTypeDef *hdma){
    //fill in line2 (later half)
	if((line<0)||(line>=VGA_VBUFFER)){
		memset((uint8_t *)VGA_obuffer,0,VGA_FULL);
	}else{
	memcpy((uint8_t *)VGA_obuffer + vga_voff21,VGA_buffer[line],VGA_LBUFFER);
	memcpy((uint8_t *)VGA_obuffer + vga_voff22,VGA_buffer[line],VGA_LBUFFER);
	}
	line++;
	if(vga_stop){
		HAL_DMA_Abort_IT(&hdma_tim1_up);
		firstTrig=1;
		vga_stop=0;
		VGA_update();
	}
	if(line>=VGA_VBUFFER && !firstTrig){
		HAL_DMA_Abort_IT(&hdma_tim1_up);
		firstTrig=1;
	}
}

void USBH_HID_EventCallback(USBH_HandleTypeDef *phost){
	if(USBH_HID_GetDeviceType(phost) == HID_KEYBOARD){
		HID_KEYBD_Info_TypeDef *Keyboard_Info;
		Keyboard_Info = USBH_HID_GetKeybdInfo(phost);
		uint8_t key = USBH_HID_GetASCIICode(Keyboard_Info);
		if(key==0)return;
		if(((kBin+1)&0xf)==kBout)return;
		kBuffer[kBin]=key;
		kBin=(kBin+1)&0xf;
		HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
	}
}

extern uint8_t getch(char *ch){
	for(int i=0;i<10;i++){
	   MX_USB_HOST_Process();
	   HAL_Delay(2);
	}
	if(kBin==kBout) return 0;
	*ch = kBuffer[kBout];
	kBout = (kBout+1)&0xf;
	return 1;
}

extern void myDelay(int ms){
	HAL_Delay(ms);
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
  MX_DMA_Init();
  MX_TIM10_Init();
  MX_TIM9_Init();
  MX_TIM1_Init();
  MX_USB_HOST_Init();
  /* USER CODE BEGIN 2 */
  HAL_DMA_Abort(&hdma_tim1_up);
  if(
  	  HAL_DMA_RegisterCallback(&hdma_tim1_up,HAL_DMA_XFER_HALFCPLT_CB_ID,DMA_HalfCpltCallback)
  	  !=HAL_OK){
  	  while(1){
  		  HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
  		  HAL_Delay(500);
  	  }
  }
  if(
        HAL_DMA_RegisterCallback(&hdma_tim1_up,HAL_DMA_XFER_CPLT_CB_ID,DMA_CpltCallback)
        !=HAL_OK){
  	  while(1){
  		  HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
  		  HAL_Delay(250);
  	  }
  }
  GPIOB_ODR = (uint32_t)&(GPIOB->ODR);
  HAL_TIM_Base_Start_IT(&htim9);
  HAL_TIM_PWM_Start(&htim9,TIM_CHANNEL_1);

  HAL_TIM_Base_Start_IT(&htim10);
  HAL_TIM_PWM_Start(&htim10,TIM_CHANNEL_1);


  __HAL_DMA_ENABLE_IT(&hdma_tim1_up,DMA_IT_TC);
  __HAL_DMA_ENABLE_IT(&hdma_tim1_up,DMA_IT_HT);
  __HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_UPDATE);
//  firstTrig=0;

//  __HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_UPDATE);
//  __HAL_TIM_ENABLE(&htim1);
  HAL_TIM_Base_Start(&htim1);
//  HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);

//  while(1)
  HAL_Delay(100);
  vga_stop=1;

  ClearScreen(VGA_BLACK);
  DrawRectangle(0,0,VGA_WIDTH-1,VGA_HEIGHT-1,VGA_WHITE);
  char msg[32]="Testing";
  SetCursor(3,3);
  WriteString(msg,Font_7x10,VGA_GREEN);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  int r,x,y,w,h,z=0,c;
  while (1)
  {
	  char cc;
//	  MX_USB_HOST_Process();
//	  continue;
	  r = rand()%50;
	  x = rand()%VGA_WIDTH;
	  y = rand()%VGA_HEIGHT;
	  c = rand()%256;
	  DrawCircle(x,y,r,c);
	  r = rand()%50;
	  x = rand()%VGA_WIDTH;
	  y = rand()%VGA_HEIGHT;
	  c = rand()%256;
	  FillCircle(x,y,r,c);
	  x = rand()%VGA_WIDTH;
	  y = rand()%VGA_HEIGHT;
	  w = rand()%50;
	  h = rand()%50;
	  c = rand()%256;
	  DrawRectangle(x,y,x+w,y+h,c);
	  x = rand()%VGA_WIDTH;
	  y = rand()%VGA_HEIGHT;
	  w = rand()%50;
	  h = rand()%50;
	  c = rand()%256;
	  FillRectangle(x,y,w,h,c);

	  if(getch(&cc)){
		FillRectangle(VGA_WIDTH-100,0,VGA_WIDTH-1,20,VGA_BLACK);
		sprintf(msg,"KEY:%d:%c",cc,cc);
		SetCursor(VGA_WIDTH-100,5);
		WriteString(msg,Font_7x10,VGA_GREEN);
	  }
	  z=z+1;
	  if(z>=100){
		  /////////
		  ClearScreen(VGA_BLACK);
		  ShowImage((uint8_t *)color,320,240,0,0);
		  HAL_Delay(5000);
		  ShowImage((uint8_t *)image,360,228,0,0);
		  HAL_Delay(5000);
		  ShowImage((uint8_t *)rgb,320,240,0,0);
		  HAL_Delay(5000);

		  tetris();
		  //////////
		  z=0;
		  ClearScreen(VGA_BLACK);
		  HAL_Delay(100);
		  vga_stop=1;
		  sprintf(msg,"Testing");
		  DrawRectangle(0,0,VGA_WIDTH-1,VGA_HEIGHT-1,VGA_WHITE);
		  SetCursor(3,3);
		  WriteString(msg,Font_7x10,VGA_GREEN);
		  sprintf(msg,"%d ",VOFFSET);
		  SetCursor(3,50);
		  WriteString(msg,Font_7x10,VGA_WHITE);
	  }
	  HAL_Delay(100);
    /* USER CODE END WHILE */
    MX_USB_HOST_Process();

    /* USER CODE BEGIN 3 */
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 4-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM9 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM9_Init(void)
{

  /* USER CODE BEGIN TIM9_Init 0 */

  /* USER CODE END TIM9_Init 0 */

  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM9_Init 1 */

  /* USER CODE END TIM9_Init 1 */
  htim9.Instance = TIM9;
  htim9.Init.Prescaler = 0;
  htim9.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim9.Init.Period = 523;
  htim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim9.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim9) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim9) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
  sSlaveConfig.InputTrigger = TIM_TS_ITR2;
  if (HAL_TIM_SlaveConfigSynchro(&htim9, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 2-1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim9, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM9_Init 2 */

  /* USER CODE END TIM9_Init 2 */
  HAL_TIM_MspPostInit(&htim9);

}

/**
  * @brief TIM10 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM10_Init(void)
{

  /* USER CODE BEGIN TIM10_Init 0 */

  /* USER CODE END TIM10_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM10_Init 1 */

  /* USER CODE END TIM10_Init 1 */
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 4-1;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 800-1;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 96-1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim10, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM10_Init 2 */

  /* USER CODE END TIM10_Init 2 */
  HAL_TIM_MspPostInit(&htim10);

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, B0_Pin|B1_Pin|G0_Pin|G1_Pin
                          |G2_Pin|R0_Pin|R1_Pin|R2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : B0_Pin B1_Pin G0_Pin G1_Pin
                           G2_Pin R0_Pin R1_Pin R2_Pin */
  GPIO_InitStruct.Pin = B0_Pin|B1_Pin|G0_Pin|G1_Pin
                          |G2_Pin|R0_Pin|R1_Pin|R2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

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
