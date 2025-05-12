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

#include <stdio.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum {
    
    KEY_IDLE,
    KEY_PUSH,
    KEY_HOLD,
    KEY_FINISH
        
} KeyState_t;

typedef struct {
    
    char key_char; // 키 값
    KeyState_t state; // 현재 상태 (PUSH, HOLD)
    uint32_t tick; // 누르기 시작한 시간
    uint8_t active; // 현재 눌려있는지 여부
    
}KeyInfo_t;

#define MAX_KEYS 16
#define DEBOUNCE_TIME 40 
#define HOLD_TIME 500
#define KEY_QUEUE_SIZE 32
#define MAX_MSG_LEN 32

typedef struct {
    
    char buffer[KEY_QUEUE_SIZE][MAX_MSG_LEN];
    uint8_t fidx;
    uint8_t ridx;
    uint8_t count;
    
} KeyQueue_t;

KeyQueue_t tx_queue = {0};
// KeyInfo_t 구조체 16개 공간을 만듬
KeyInfo_t keys[MAX_KEYS] = {0}; // 전체 키 상태 저장

const char keymap[4][4] = {
    
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
    
};

// GPIO_PIN_0은 HAL 라이브러리에 이미 정의 되있음
// #define GPIO_PIN_0    ((uint16_t)0x0001)  // 0번 핀
// GPIO는 16개씩 핀을 가지고 있음 그래서 uint16_t으로 사용함
// GPIO 제어는 보통 하나의 레지스터로 여러 핀을 동시에 제어함
// 그래서 각 핀을 비트 단위로 표현해서 제어함
uint16_t row_pins[4] = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3};
uint16_t col_pins[4] = {GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7};

uint8_t uart_tx_ready = 1;


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

/*
// 16키패드 모듈 테스트
char ScanKeypad(void) {
    for (int row = 0; row < 4; row++) {
        
        // 모든 행 HIGH, 현재 행만 LOW
        for (int r = 0; r < 4; r++) {
            // GPIO 핀은 16비트 정수의 한 비트에 대응됨
            // 핀 0은 0000 0000 0000 0001
            HAL_GPIO_WritePin(GPIOA, (1 << r), GPIO_PIN_SET);
        }
        HAL_GPIO_WritePin(GPIOA, (1 << row), GPIO_PIN_RESET); // 현재 행만 LOW
        
        // 열 스캔
        for (int col = 0; col < 4; col++) {
            if (HAL_GPIO_ReadPin(GPIOA, (1 << (col + 4))) == GPIO_PIN_RESET) {
                HAL_Delay(40); // 디바운싱
                if (HAL_GPIO_ReadPin(GPIOA, (1 << (col + 4))) == GPIO_PIN_RESET) {
                    return keymap[row][col];
                }
            }
        }
    }
    return 0;
}
*/

/*
// 큐에 키 값 추가
void EnqueueKey(KeyQueue_t *q, char key) {
    
    if (q->count < KEY_QUEUE_SIZE) {
        
        q->buffer[q->ridx++] = key;
        q->ridx %= KEY_QUEUE_SIZE;  // 오버플로우 방지
        q->count++;
        
    }
    
}

// 큐에서 값 꺼냄
char DequeueKey(KeyQueue_t *q) {
    
    if (q->count == 0) return 0;
    char key = q->buffer[q->fidx++];
    q->fidx %= KEY_QUEUE_SIZE; 
    q->count--;
    return key;
    
}
*/

// 메시지를 큐에 넣는 함수
void EnqueueMessage(const char *msg) {
    if (tx_queue.count < KEY_QUEUE_SIZE) {
        strncpy(tx_queue.buffer[tx_queue.ridx], msg, MAX_MSG_LEN - 1);
        tx_queue.buffer[tx_queue.ridx][MAX_MSG_LEN - 1] = '\0'; // NULL 종료 보장
        tx_queue.ridx = (tx_queue.ridx + 1) % KEY_QUEUE_SIZE;
        tx_queue.count++;
    }
}

// 큐에서 메시지를 꺼내는 함수
char* DequeueMessage(void) {
    if (tx_queue.count == 0) return NULL;
    char* msg = tx_queue.buffer[tx_queue.fidx];
    tx_queue.fidx = (tx_queue.fidx + 1) % KEY_QUEUE_SIZE;
    tx_queue.count--;
    return msg;
}

// 키 상태 업데이트
void UpdateKeyState(uint8_t key_index, uint8_t pressed, uint32_t tick) {
    
    KeyInfo_t *k = &keys[key_index];  // 해당 키에 대한  포인터
    
    if (pressed) {  // 키가 눌려 있을 경우
        
        if (!k->active && (tick - k->tick >= DEBOUNCE_TIME)) {
            
            k->active = 1;
            k->tick = tick;
            k->state = KEY_PUSH;
            //EnqueueKey(&tx_queue, k->key_char);
            
            char msg[32];
            snprintf(msg, sizeof(msg), "KEY: %c, STATE: PUSH\r\n", k->key_char);
            EnqueueMessage(msg);
            
        } else if (k->active && k->state == KEY_PUSH && (tick - k->tick) >= HOLD_TIME) {
            
            k->state = KEY_HOLD;
            //EnqueueKey(&tx_queue, k->key_char);
            
            char msg[32];
            snprintf(msg, sizeof(msg), "KEY: %c, STATE: HOLD\r\n", k->key_char);
            EnqueueMessage(msg);
            
        }
        
    } else { // 키가 떨어져 있을 경우
        
        if (k->active && (tick - k->tick) >= DEBOUNCE_TIME) {
            
            k->active = 0;
            if (k->state != KEY_IDLE) {
            k->state = KEY_FINISH;
            //EnqueueKey(&tx_queue, k->key_char);
            
            char msg[32];
            snprintf(msg, sizeof(msg), "KEY: %c, STATE: FINISH\r\n", k->key_char);
            EnqueueMessage(msg);
            
            }
            
        } else {
            
            k->state = KEY_IDLE;
            
        }
        
    }
    
}

// 키패드 스캔 함수
void ScanKeypad() {
    
    uint32_t now = HAL_GetTick();  // 외부 인터럽트로 ScanKeypad() 호출 시점의 시간
    
    for (int row = 0; row < 4; row++) {
        
        
        for (int r = 0; r < 4; r++) {
            
            // 모든 행을 일단 HIGH로 설정
            HAL_GPIO_WritePin(GPIOA, row_pins[r], GPIO_PIN_SET);
            
        }
        
        // 한 행씩 LOW로 설정 / COL에서 LOW값을 받아서  행열 모두 LOW인 값 찾기 위해
        HAL_GPIO_WritePin(GPIOA, row_pins[row], GPIO_PIN_RESET);
        
        for (int col = 0; col < 4; col++) {
            
            // 키가 눌렸을 경우 key_down에 1 대입
            uint8_t key_down = (HAL_GPIO_ReadPin(GPIOA, col_pins[col]) == GPIO_PIN_RESET);
            
            /*
                        Col0  Col1  Col2  Col3
                       +-----+-----+-----+-----+
                Row0   |  0  |  1  |  2  |  3  |
                Row1   |  4  |  5  |  6  |  7  |
                Row2   |  8  |  9  | 10  | 11  |
                Row3   | 12  | 13  | 14  | 15  |
            */
            // 2차원 배열이지만 인덱스 부여 가능
            // 아래 공식으로 인덱스 계산 가능
            // RTOS에서도 테이블 기반 스케줄링, 이벤트 맵 등을 이런 식으로 계산
            // 하드웨어 제어에서도 레지스터 배열 접근 시 row-major(행 기준 순서) 접근 방식이 매우 유용
            uint8_t key_index = row * 4 + col;  // 키 인덱스 계산
            
            keys[key_index].key_char = keymap[row][col];  // 문자 매핑
            UpdateKeyState(key_index, key_down, now);  // 상태 업데이트

        }
        
    }
    
}

// 키 값 UART 전송
void SendNextMessage(void) {
    
    if (tx_queue.count > 0 && uart_tx_ready) {           
        //char key = DequeueKey(&tx_queue);                
        //char msg[32];
        char *msg = DequeueMessage();
        uart_tx_ready = 0;  
        //snprintf(msg, sizeof(msg), "KEY: %c\r\n", key);
        HAL_UART_Transmit_IT(&huart1, (uint8_t*)msg, strlen(msg));  
    }
    
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    for (int col = 0; col < 4; col++) {
        if (GPIO_Pin == col_pins[col]) {   // 어떤 Col 핀에서 인터럽트 발생했는지 확인
            ScanKeypad();                  // 키패드 스캔 수행
            break;
        }
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        uart_tx_ready = 1;   // 다음 전송 가능 상태로 설정
    }
}

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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
    
    for (int i = 0; i < 4; i++) {
    HAL_GPIO_WritePin(GPIOA, row_pins[i], GPIO_PIN_SET);  // 시작 시 모든 Row를 HIGH로 설정
}
    
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
        
        /*
        // 16키패드 모듈 테스트
        char key = ScanKeypad();
        if (key) {
            char msg[20];
            snprintf(msg, sizeof(msg), "Pressed: %c\r\n", key);
            HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
            HAL_Delay(200); // 반복 방지
        }
        */
        
        ScanKeypad();
        SendNextMessage();
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
