/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32f1xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <string.h>

#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

extern uint8_t rxData; // 수신한 데이터 변수
extern char txMsg[50]; // 송신할 문자열 버퍼
extern volatile uint8_t rxReadyFlag;
extern volatile uint8_t uartTxDone;

// volatile 컴파일러 최적화 방지 -> 인터럽트에서 바뀐 값 실시간으로 반영
// 초기값은 1 송신 가능한 상태
//volatile uint8_t uartTxDone = 1;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern PCD_HandleTypeDef hpcd_USB_FS;
extern UART_HandleTypeDef huart1;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles USB low priority or CAN RX0 interrupts.
  */
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  /* USER CODE BEGIN USB_LP_CAN1_RX0_IRQn 0 */

  /* USER CODE END USB_LP_CAN1_RX0_IRQn 0 */
  HAL_PCD_IRQHandler(&hpcd_USB_FS);
  /* USER CODE BEGIN USB_LP_CAN1_RX0_IRQn 1 */

  /* USER CODE END USB_LP_CAN1_RX0_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */
  
  // main에서 HAL_UART_Receive_IT(&huart1, &rxData, 1); 함수가 실행되면 
  // 자동으로 USART1_IRQHandler() 함수가 자동으로 실행됨
  // HAL 내부 상태를 분석함
  // STM32 내부 NVIC 시스템이 자동으로 이 함수를 호출
  
  //if (USART1->SR & USART_SR_RXNE) {
    //HAL_UART_IRQHandler(&huart1); // UART 인터럽트 발생 시 HAL 내부 처리 함수 호출
  //}
    
  /* USER CODE END USART1_IRQn 1 */
}

/* USER CODE BEGIN 1 */

// HAL_UART_Transmit_IT() 송신이 완료되면 자동 호출되는 콜백 함수
// 여기서 uartTxDone을 1로 만들어서 새로운 메시지를 보내도 안전하다 표시

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1) {
    uartTxDone = 1;  // 송신이 끝났으니, 다시 보내도 됨
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
  }
}


// UART 수신 완료 시 자동으로 호출되는 콜백 함수
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  
  // UART가 여러 개일 수 있으니 지금 인터럽트가 발생한 UART가 USART1인지 확인
  // huart->Instance는 내부적으로 USART1, USART2 등을 가리킴 
  // 이 조건이 있어야 다른 UART를 쓸 때도 코드 충돌 없이 처리 가능
  if (huart->Instance == USART1) 
  {
    //if (uartTxDone) {
      
    // 수신한 데이터를 문자열로 만들어 전송할 준비
    // snprintf()는 안전하게 문자열을 만들어주는 함수
    // txMsg 문자열을 저장할 버퍼 (extern으로 main에 선언된 변수 사용)
    // sizeof(txMsg) 버퍼 크기만큼만 출력 (버퍼 오버플로우 방지)
    // rxData 수신한 실제 문자
    //snprintf(txMsg, sizeof(txMsg), "You typed: %c\r\n", rxData);
    //strcpy(txMsg, "OK\r\n");
    
    //uartTxDone = 0;
    rxReadyFlag = 1;

    // 마지막에 IT  접미사가 붙은 함수는 인터럽트 기반 논블로킹 방식
    // 준비된 문자열(txMsg)을 pc로 전송하는 함수
    // Transmit_IT는 인터럽트 기반 송신 (논블로킹) 방식이기 때문에 송신이 끝나기를 기다리지 않고
    // 바로 return됨
    //HAL_UART_Transmit_IT(&huart1, (uint8_t*)txMsg, strlen(txMsg));
    
   // }

    // 다시 수신 대기 (1바이트 수신 대기)
    HAL_UART_Receive_IT(&huart1, &rxData, 1);
    
    
    
  }
}


/*
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    // 간단한 디버깅 메시지
    char errMsg[] = "UART ERROR\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t*)errMsg, strlen(errMsg), 100);

    // 에러 이후 수신 다시 등록
    HAL_UART_Receive_IT(&huart1, &rxData, 1);
  }
}
*/
/* USER CODE END 1 */
