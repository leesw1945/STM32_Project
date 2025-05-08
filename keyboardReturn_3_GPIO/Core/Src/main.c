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
    
    KEY_IDLE,
    KEY_PUSH,
    KEY_HOLD,
    KEY_FINISH
        
} KeyState_t;

typedef struct {
    
    char key_char;
    KeyState_t state;
    uint32_t hold_time;
    uint8_t active;
    
}KeyInfo_t;

#define MAX_KEYS 16
#define DEBOUNCE_TIME 40
#define HOLD_TIME 500
#define KEY_QUEUE_SIZE 32

// KeyInfo_t 구조체 16개 공간을 만듬
KeyInfo_t keys[MAX_KEYS]; // 전체 키 상태 저장

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
