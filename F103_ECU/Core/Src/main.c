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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Can.h"
#include "isotp.h"
#include "UDS.h"
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
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static uint8_t Boot_IsAppValid(void);
static void Boot_JumpToApp(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t RXData[1024];
CAN_TxHeaderTypeDef TXHeader={0x7E8,0,CAN_ID_STD,CAN_RTR_DATA,8,DISABLE};
CAN_RxHeaderTypeDef CAN_RxHeader;
uint32_t pTxMailbox;
uint32_t pRxMailbox;
uint8_t SN;

#define APP_START_ADDRESS 0x08008000U

__attribute__((naked,noreturn)) static void Boot_StartApp(uint32_t AppStack,uint32_t AppResetHandler)
{
  __asm volatile(
    "msr msp,r0\n"
    "cpsie i\n"
    "bx r1\n"
  );
}

static uint8_t Boot_IsAppValid(void)
{
  uint32_t AppStack=*(uint32_t *)APP_START_ADDRESS;
  uint32_t AppResetHandler=*(uint32_t *)(APP_START_ADDRESS+4U);
  if ((AppStack<0x20000000U)||(AppStack>0x20005000U))
  {
    return 0;
  }
  if ((AppResetHandler<APP_START_ADDRESS)||(AppResetHandler>=0x08010000U))
  {
    return 0;
  }
  return 1;
}

static void Boot_JumpToApp(void)
{
  uint32_t AppStack=*(uint32_t *)APP_START_ADDRESS;
  uint32_t AppResetHandler=*(uint32_t *)(APP_START_ADDRESS+4U);

  if (Boot_IsAppValid()==0)
  {
    return;
  }

  HAL_CAN_Stop(&hcan1);
  HAL_RCC_DeInit();
  HAL_DeInit();

  __disable_irq();


  SysTick->CTRL=0;
  SysTick->LOAD=0;
  SysTick->VAL=0;

  SCB->ICSR=SCB_ICSR_PENDSTCLR_Msk|SCB_ICSR_PENDSVCLR_Msk;

  NVIC->ICER[0]=0xFFFFFFFFU;
  NVIC->ICER[1]=0xFFFFFFFFU;
  NVIC->ICPR[0]=0xFFFFFFFFU;
  NVIC->ICPR[1]=0xFFFFFFFFU;

  SCB->VTOR=APP_START_ADDRESS;
  __DSB();
  __ISB();
  Boot_StartApp(AppStack,AppResetHandler);
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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  UDS_Init();

  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    UDS_Divide_ID(RXData);
    if (AppJumpRequested==1)
    {
      HAL_Delay(100);
      Boot_JumpToApp();
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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
