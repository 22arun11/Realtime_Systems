/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_adc.h"
#include "stm32f4xx_hal_uart.h"
#include "stm32f4xx_hal_conf.h"

/* Private defines ------------------------------------------------------------*/
#define TASK_STACK_SIZE        128
#define ADC_TASK_PRIORITY      3    // Highest priority
#define LED_HIGH_PRIORITY      2    // Medium priority
#define LED_LOW_PRIORITY       1    // Lowest priority
#define MUTEX_CEILING_PRIORITY ADC_TASK_PRIORITY  // Priority ceiling

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart2;  // For Bluetooth
UART_HandleTypeDef huart1;

SemaphoreHandle_t adc_mutex;
static uint32_t shared_adc_value;
static uint8_t led_pattern_selection = 0;

/* Task original priorities */
static UBaseType_t adc_task_original_priority;
static UBaseType_t led_high_task_original_priority;
static UBaseType_t led_low_task_original_priority;

static uint8_t rx_buffer[1];  // UART receive buffer
static void MX_USART2_UART_Init(void);

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void adc_reading_task(void* parameters);
static void led_pattern_high_task(void* parameters);
static void led_pattern_low_task(void* parameters);
void raise_priority_to_ceiling(TaskHandle_t task_handle);
void restore_task_priority(TaskHandle_t task_handle, UBaseType_t original_priority);
static void MX_USART2_UART_Init(void);
static void bluetooth_task(void* parameters);
void uart_print(const char* str);

/* UART2 Initialization (For Bluetooth) */
static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 9600;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
}


int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick */
    HAL_Init();
    SystemClock_Config();
    MX_USART2_UART_Init();


    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_ADC1_Init();

    /* Create mutex for PCP */
    adc_mutex = xSemaphoreCreateMutex();
    if (adc_mutex == NULL) {
        Error_Handler();
    }

    /* Create the three tasks with different priorities */
    TaskHandle_t adc_task_handle, led_high_task_handle, led_low_task_handle;

    adc_task_original_priority = ADC_TASK_PRIORITY;
    led_high_task_original_priority = LED_HIGH_PRIORITY;
    led_low_task_original_priority = LED_LOW_PRIORITY;

    xTaskCreate(adc_reading_task, "ADCTask", TASK_STACK_SIZE, &adc_task_handle, ADC_TASK_PRIORITY, NULL);
    xTaskCreate(led_pattern_high_task, "LEDHighTask", TASK_STACK_SIZE, &led_high_task_handle, LED_HIGH_PRIORITY, NULL);
    xTaskCreate(led_pattern_low_task, "LEDLowTask", TASK_STACK_SIZE, &led_low_task_handle, LED_LOW_PRIORITY, NULL);
    vTaskSetDeadline(adc_task_handle,2000);
    vTaskSetDeadline(led_high_task_handle,3000);
    vTaskSetDeadline(adc_task_handle,1000);
    /* Start scheduler */
    vTaskStartScheduler();

    /* We should never get here as control is now taken by the scheduler */
    while (1)
    {
    }
}

/**
  * @brief  ADC Reading Task - Highest Priority
  * @param  parameters: Not used
  * @retval None
  */
static void adc_reading_task(void* parameters)
{
    uint32_t local_adc_value;
    const uint32_t threshold1 = 1365;  // One-third of max (4095/3)
    const uint32_t threshold2 = 2730;  // Two-thirds of max (2*4095/3)
    char uart_buffer[50];  // Buffer to hold the UART message

    while (1)
    {
        TaskHandle_t current_task_handle = xTaskGetCurrentTaskHandle();

        /* Take mutex with PCP - Temporarily raise to ceiling priority */
        if (xSemaphoreTake(adc_mutex, portMAX_DELAY) == pdTRUE)
        {
            raise_priority_to_ceiling(current_task_handle);  // Elevate priority to ceiling


            /* Read ADC */
            HAL_ADC_Start(&hadc1);
            if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK)
            {
                local_adc_value = HAL_ADC_GetValue(&hadc1);
                shared_adc_value = local_adc_value;

                /* Update LED pattern based on ADC value */
                if (local_adc_value < threshold1) {
                    led_pattern_selection = 1;  // Slow pattern
                } else if (local_adc_value < threshold2) {
                    led_pattern_selection = 2;  // Medium pattern
                } else {
                    led_pattern_selection = 3;  // Fast pattern
                }

            }
            HAL_ADC_Stop(&hadc1);

            xSemaphoreGive(adc_mutex);  // Release the mutex
            restore_task_priority(current_task_handle, adc_task_original_priority);  // Restore original priority
        }

        /* ADC reading interval */
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
  * @brief  LED Pattern High Priority Task
  * @param  parameters: Not used
  * @retval None
  */
static void led_pattern_high_task(void* parameters)
{
    uint8_t local_pattern;
    TaskHandle_t current_task_handle = xTaskGetCurrentTaskHandle();
    char uart_buffer[50];
    while (1)
    {
        /* Take mutex with PCP - Temporarily raise to ceiling priority */
        if (xSemaphoreTake(adc_mutex, portMAX_DELAY) == pdTRUE)
        {
            raise_priority_to_ceiling(current_task_handle);  // Elevate priority to ceiling

            local_pattern = led_pattern_selection;

            xSemaphoreGive(adc_mutex);  // Release the mutex
            restore_task_priority(current_task_handle, led_high_task_original_priority);  // Restore original priority

            /* High priority pattern - Quick double blink */
            if (local_pattern == 3)  // Only run when ADC is in highest range
            {
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
                vTaskDelay(pdMS_TO_TICKS(50));
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
                vTaskDelay(pdMS_TO_TICKS(50));
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
                vTaskDelay(pdMS_TO_TICKS(50));
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
                snprintf(uart_buffer, sizeof(uart_buffer), "High Priority \r\n");
                uart_print(uart_buffer);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(200));  // Medium delay between patterns
    }
}

/**
  * @brief  LED Pattern Low Priority Task
  * @param  parameters: Not used
  * @retval None
  */
static void led_pattern_low_task(void* parameters)
{
    uint8_t local_pattern;
    TaskHandle_t current_task_handle = xTaskGetCurrentTaskHandle();
    char uart_buffer[50];
    while (1)
    {

        /* Take mutex with PCP - Temporarily raise to ceiling priority */
        if (xSemaphoreTake(adc_mutex, portMAX_DELAY) == pdTRUE)
        {
            raise_priority_to_ceiling(current_task_handle);  // Elevate priority to ceiling

            local_pattern = led_pattern_selection;

            xSemaphoreGive(adc_mutex);  // Release the mutex
            restore_task_priority(current_task_handle, led_low_task_original_priority);  // Restore original priority

            /* Low priority patterns */
            switch(local_pattern)
            {
                case 1:  // Slow single blink
                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
//                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, GPIO_PIN_SET);
                    vTaskDelay(pdMS_TO_TICKS(500));
                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
                    snprintf(uart_buffer, sizeof(uart_buffer), "Low Priority\r\n");
                    uart_print(uart_buffer);
                    break;

                case 2:  // Medium single blink
                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
                    vTaskDelay(pdMS_TO_TICKS(200));
                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
                    snprintf(uart_buffer, sizeof(uart_buffer), "Medium Priority\r\n");
                    uart_print(uart_buffer);
                    break;

                default:
                    // Do nothing when pattern 3 is active (handled by high priority task)
                    break;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500));  // Longer delay for low priority task
    }
}

/* Raise task priority to the ceiling */
void raise_priority_to_ceiling(TaskHandle_t task_handle)
{
    vTaskPrioritySet(task_handle, MUTEX_CEILING_PRIORITY);
}

/* Restore task priority after mutex release */
void restore_task_priority(TaskHandle_t task_handle, UBaseType_t original_priority)
{
    vTaskPrioritySet(task_handle, original_priority);
}
/* UART printing function */
void uart_print(const char* str)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);

    /* Configure GPIO pin : PD13 */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* Configure GPIO pin : PD14 */
	GPIO_InitStruct.Pin = GPIO_PIN_14;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/* Configure GPIO pin : PD12 */
	GPIO_InitStruct.Pin = GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* Configure GPIO pin : PA0 */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* Initialize ADC1 */
static void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    /* Enable ADC1 clock */
    __HAL_RCC_ADC1_CLK_ENABLE();

    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DMAContinuousRequests = DISABLE;

    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }

    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}
