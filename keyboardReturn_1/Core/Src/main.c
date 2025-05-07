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
#include "stdio.h"
#include "string.h"

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
#define HOLD_CHECK 600 // 0.6초 이상 누르고 있으면 HOLD
#define FINISH_CHECK 1000 // 1초 이상 입력 없으면 FINISH
//#define PUSH_CHECK 30

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

// 키 상태 확인
uint8_t last_key_value = 0; // 새로운 키 값을 비교하여 바꼈는지 같은 키가 눌린 건지 확인
KeyState current_key_state = KEY_IDLE; // 현재 키의 상태
uint32_t last_key_time = 0; // 채터링 방지 40ms 지났는지 확인용
//uint16_t push_tick = 0;
uint8_t hold_key_sent = 0; // HOLD 상태에서 키가 전송 되었는지 확인

// 키 큐
//KeyQueue_t rx_queue = {0};
KeyQueue_t tx_queue = {0};

// uart 수신/송신
uint8_t rx_data = 0;
volatile uint8_t uart_tx_ready = 1;

char msg[32];
//uint8_t key_value = 0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void ProcessKey(uint8_t key_value);
void EnqueueKey(KeyQueue_t* queue, char key_value);
char DequeueKey(KeyQueue_t* queue);
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
void ProcessKey(uint8_t key_value) {
    
    uint32_t current_tick = HAL_GetTick();
    
    // 40ms 마다 동일한 값이 들어오면 리턴
    if (key_value == last_key_value && current_tick - last_key_time < DEBOUNCE_TIME) {
        
        return;
        
    }
    
    // 입력된 값이 기존 값과 다른 경우
    if (key_value != last_key_value) {
        
        last_key_time = current_tick;
        last_key_value = key_value;
        
        current_key_state = KEY_PUSH;
        hold_key_sent = 0;
        EnqueueKey(&tx_queue, key_value);
        
      // PUSH인데 키 값이 기존 값과 같을 경우 출력
    } else if (key_value == last_key_value && current_key_state == KEY_PUSH && current_tick - last_key_time > HOLD_CHECK ) {
        
        current_key_state = KEY_PUSH;
        hold_key_sent = 0;
        EnqueueKey(&tx_queue, key_value);
        
      // HOLD 체크 (키가 계속 눌리고 있는지)
    } else if (key_value == last_key_value && current_key_state == KEY_PUSH) {
        
        if (current_tick - last_key_time <= HOLD_CHECK && hold_key_sent == 0) {
            
            current_key_state = KEY_HOLD;
            hold_key_sent = 1;
            EnqueueKey(&tx_queue, key_value);
            
        }
        
      // FINISH 처리
    } else if (current_key_state == KEY_HOLD && (current_tick - last_key_time >= FINISH_CHECK)) {
            
            current_key_state = KEY_FINISH;
            hold_key_sent = 0;
        
    }
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
        
        //char msg[32];
        
        if (current_key_state == KEY_PUSH) {
            
            snprintf(msg, sizeof(msg), "PUSH: %c\r\n", key);
            
        } else if (current_key_state == KEY_HOLD) {
            
            snprintf(msg, sizeof(msg), "HOLD: %c\r\n", key);
            
        } else if (current_key_state == KEY_FINISH) {
            
            snprintf(msg, sizeof(msg), "FINISH: %c\r\n", key);
        
        } else {

            snprintf(msg, sizeof(msg), "KEY: %c\r\n", key);
            
        }
            
        HAL_UART_Transmit_IT(&huart1, (uint8_t*)msg, strlen(msg));
        

    } 
    
}


/*
* Description : UART 수신 완료 콜백 함수
* Param       : huart - UART 핸들
* return      : void
*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    
    if (huart->Instance == USART1) {
        
        //push_tick = HAL_GetTick();
        // 수신된 키 값 처리
        ProcessKey(rx_data);
        
        HAL_UART_Receive_IT(&huart1, &rx_data, 1);
        
    }
    
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
    
    // 시작 메시지 전송
    char start_msg[] = "Keyboard UART System Ready\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t*)start_msg, sizeof(start_msg)-1, 100);
    
    // UART 수신 인터럽트 시작
    HAL_UART_Receive_IT(&huart1, &rx_data, 1);
    
    /* USER CODE END 2 */
    
    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */
        
        /* USER CODE BEGIN 3 */
        
        // UART 송신 준비되었고 큐에 데이터 있으면 다음 키 전송
        if (uart_tx_ready && tx_queue.count > 0) {
            
            SendNextKey();
            
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
