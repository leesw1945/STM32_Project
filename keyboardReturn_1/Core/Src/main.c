/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum {
  
    KEY_IDLE, // 키가 눌리지 않음
    KEY_PUSH, // 키가 눌림
    KEY_HOLD, // 키가 눌러져 있는 상태
    KEY_FINISH // 키가 떼진 상태
  
} KeyState; // 키 상태 enum

#define KEY_QUEUE_SIZE 32

typedef struct {
  
    char buffer[KEY_QUEUE_SIZE];
    uint8_t fidx;
    uint8_t ridx;
    uint8_t count;
    
} KeyQueue_t; // 키 큐 구조체



/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define DEBOUNCE_TIME 40 // 채터링 방지 시간 (ms)
#define HOLD_CHECK 1000 // 1초 이상 누르고 있으면 HOLD

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

// 키 상태 확인
uint8_t last_key_state = 0; // 새로운 키 값을 비교하여 바꼈는지 같은 키가 눌린 건지 확인
KeyState current_key_state = KEY_IDLE; // 현재 키의 상태
uint32_t last_key_time = 0; // 채터링 방지 40ms 지났는지 확인용

// 키 큐
KeyQueue_t rx_queue = {0};
KeyQueue_t tx_queue = {0};

// uart 수신/송신
uint8_t rx_data = 0;
volatile uint8_t uart_tx_ready = 1;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void ProcessKey(uint8_t key_value);
void EnqueueKey(KeyQueue_t queue, char key_value);
char DequeueKey(KeyQueue_t queue);
void SendNextKey(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/*
    * Description : 키 큐에 키 값을 넣는 함수
    * Param       : queue 대상 큐 포인터
    *               key_value 큐에 넣을 키 값
    * return      : void
*/
void EnqueueKey(KeyQueue_t* queue, char key_value) {
    
    if (queue->count < KEY_QUEUE_SIZE) {
        
        queue->buffer[queue->ridx] = key_value;
        queue->ridx = (queue->ridx + 1) % KEY_QUEUE_SIZE;
        queue->count++;
    }
    
}


/*
    * Description : 키 큐에서 키 값을 꺼내는 함수
    * Param       : queue 대상 큐 포인터
    * return      : 큐에서 꺼낸 키 값, 값이 없을 경우 0
*/
char DequeueKey(KeyQueue_t* queue) {
    
    char key = 0;
    
    if (queue->count > 0) {
        
        key = queue->buffer[queue->fidx];
        queue->fidx = (queue->fidx + 1) % KEY_QUEUE_SIZE;
        queue->count--;
        
    }
    
    return key;
    
}


/*
    * Description : 수신된 키 값을 처리하는 함수 / 채터링 처리
    * Param       : key_state 수신된 키 값
    * return      : void
*/
void ProcessKey(uint8_t key_state) {
    
    uint8_t current_tick = HAL_GetTick();
    
    // 채터링 방지 및 키 상태 처리
    if (key_state != last_key_state) {
        
        last_key_state = key_state;
        last_key_time = current_tick;
        
        if (current_tick - last_key_time >= DEBOUNCE_TIME) {
            
            // 키 상태가 변했고 IDLE 이거나 FINISH 상태면 현재 키 상태를 누름 상태로 변경
            if (current_key_state == KEY_IDLE || current_key_state == KEY_FINISH) {
                
                current_key_state = KEY_PUSH;
                EnqueueKey(&tx_queue, key_value);
                
            } else {
                
                current_key_state = KEY_FINISH;
                EnqueueKey(&tx_queue, key_value);
                
            }
            
        }
        
    } else {
        
        if (current_tick - last_key_time >= DEBOUNCE_TIME) {
            
            if (current_tick - last_key_time >= HOLD_CHECK) {
                
                if (current_key_state == KEY_PUSH) {
                    
                    current_key_state = KEY_HOLD;
                    EnqueueKey(&tx_queue, key_value);
                    
                }
            }
            
        }
        
    }
    
    // 수신된 키 값을 수신 큐에 저장
    EnqueueKey(&rx_queue, key_value);
    
}


/*
    * Description : 다음 키를 UART로 전송
    * Param       : none
    * return      : void
*/
void SendNextKey(void) {
    
    if (tx_queue.count > 0) {
        
        char key = DequeueKey(&tx_queue);
        uart_tx_ready = 0; // UART 전송이 끝나면 다음 키 전송 플래그 변수
        HAL_UART_Transmit_IT(&huart1, (uint8_t*)&key, 1);
        
    }
    
}


/*
    * Description : UART 수신 완료 콜백 함수
    * Param       : huart - UART 핸들
    * return      : void
*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    
    if (huart->Instance == USART1) {
        
        // 수신된 키 값 처리
        ProcessKey(rx_data);
        
        HAL_UART_Receive_IT(&huart, &rx_data, 1);
    
}


/*
    * Description : UART 송신 완료 콜백 함수
    * Param       : huart - UART 핸들
    * return      : void
*/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    
    if (huart->Instance == USART1) {
        
        // 송신 완료 후 플래그 변수 값 변경
        uart_tx_ready = 1;
        
        // 큐에 키가 있으면 다음 키 전송
        if (tx_queue.count > 0) {
            
            SendNextKey();
            
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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
