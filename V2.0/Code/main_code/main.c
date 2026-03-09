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
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ButtonProcess.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

const char heart_pong[] = {0xaa, 0x06, 0x00, 0x9f, 0x02, 0x02, 0x3c, 0xc5};
const char heart_ping[] = {0xaa, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x7f, 0x30};
const char spin_dry[] = {0xaa, 0x06, 0x01, 0x98, 0x04, 0x00, 0x75, 0x03};
const char speed_wash[] = {0xaa, 0x06, 0x01, 0x98, 0x03, 0x01, 0xd0, 0xac};
const char standard_wash[] = {0xaa, 0x06, 0x01, 0x98, 0x02, 0x02, 0xb2, 0x0a};

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

char receiveData[32] = {0};

BUTTON* button1;
bool button1_source = false;

uint8_t washMech_mode = 2;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{ 
    if(huart == &huart2)
    {
        // heart_ping

        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, (uint8_t *)receiveData, sizeof(receiveData));
    }
}

void button1_shortPress_callback(void* context)
{
    uint8_t* mode = (uint8_t *)context;

    *mode += 1;
    if(*mode >= 3)
        *mode = 0;

    switch(*mode)
    {
        case 2: HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1); HAL_Delay(150); HAL_TIM_PWM_Stop(&htim16, TIM_CHANNEL_1); HAL_Delay(150);
        case 1: HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1); HAL_Delay(150); HAL_TIM_PWM_Stop(&htim16, TIM_CHANNEL_1); HAL_Delay(150);
        case 0: HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1); HAL_Delay(150); HAL_TIM_PWM_Stop(&htim16, TIM_CHANNEL_1); break;
        default : *mode = 0;
    }
}

void button1_longPress_callback(void* context)
{
    uint8_t* mode = (uint8_t *)context;

    switch(*mode)
    {
        case 0: HAL_UART_Transmit(&huart2, (uint8_t *)spin_dry, sizeof(spin_dry), 0x1000); break;
        case 1: HAL_UART_Transmit(&huart2, (uint8_t *)speed_wash, sizeof(speed_wash), 0x1000); break;
        case 2: HAL_UART_Transmit(&huart2, (uint8_t *)standard_wash, sizeof(standard_wash), 0x1000); break;
        default: break;
    }

    HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
    HAL_Delay(600);
    HAL_TIM_PWM_Stop(&htim16, TIM_CHANNEL_1);
    HAL_Delay(500);
    HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
    HAL_Delay(300);
    HAL_TIM_PWM_Stop(&htim16, TIM_CHANNEL_1);
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
  MX_USART2_UART_Init();
  MX_TIM16_Init();
  /* USER CODE BEGIN 2 */

  button1 = create_button(BUTTON_Pin, false, true, High_Active, 40, 30, 650, 350, 30);
  set_shortPress_event(button1, button1_shortPress_callback, &washMech_mode);
  set_longPress_event(button1, button1_longPress_callback, &washMech_mode);

  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, (uint8_t *)receiveData, sizeof(receiveData));

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    updata_button(button1);

    /* USER CODE END WHILE */

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

  __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_0);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
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
