/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tim.h"
#include "../User/joystick.h"
#include "../User/debug.h"
#include "../User/adc-process.h"
#include "../../3rdParty/elog.h"
#include "../../3rdParty/lwbtn_opts.h"
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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId debugTaskHandle;
osThreadId joystickTaskHandle;
osThreadId elog_taskHandle;
osSemaphoreId elog_lockHandle;
osSemaphoreId elog_asyncHandle;
osSemaphoreId elog_dma_lockHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDebugTask(void const * argument);
void StartJoystickTask(void const * argument);
void StartELogTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of elog_lock */
  osSemaphoreDef(elog_lock);
  elog_lockHandle = osSemaphoreCreate(osSemaphore(elog_lock), 1);

  /* definition and creation of elog_async */
  osSemaphoreDef(elog_async);
  elog_asyncHandle = osSemaphoreCreate(osSemaphore(elog_async), 1);

  /* definition and creation of elog_dma_lock */
  osSemaphoreDef(elog_dma_lock);
  elog_dma_lockHandle = osSemaphoreCreate(osSemaphore(elog_dma_lock), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of debugTask */
  osThreadDef(debugTask, StartDebugTask, osPriorityNormal, 0, 512);
  debugTaskHandle = osThreadCreate(osThread(debugTask), NULL);

  /* definition and creation of joystickTask */
  osThreadDef(joystickTask, StartJoystickTask, osPriorityHigh, 0, 256);
  joystickTaskHandle = osThreadCreate(osThread(joystickTask), NULL);

  /* definition and creation of elog_task */
  osThreadDef(elog_task, StartELogTask, osPriorityLow, 0, 512);
  elog_taskHandle = osThreadCreate(osThread(elog_task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDebugTask */
/**
  * @brief  Function implementing the debugTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDebugTask */
void StartDebugTask(void const * argument)
{
  /* USER CODE BEGIN StartDebugTask */
  //HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  // static const char* TAG = "freertos.c";
  /* 测试日志输出 */
  // log_i("System", "EasyLogger initialized with USART1 output");
  // log_i("System", "FreeRTOS version: %s", tskKERNEL_VERSION_NUMBER);
  // log_i("System", "System clock: %lu Hz", HAL_RCC_GetSysClockFreq());
  uint32_t count = 0;
  /* Infinite loop */
  for(;;)
  {
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    // ADC_PROCESS_GetBatteryVoltage();
    // adc_print_raw_buffer_simple("raw_buffer",
    //                              (uint16_t*)hadcProc.raw_buffer,
    //                              ADC_PROCESS_NUM_CHANNELS);
    // log_system_status();
    // DebugPrint("Debug Task \r\n");
    // elog_v(TAG, "HelloWorld");
    // elog_d(TAG, "HelloWorld");
    // elog_i(TAG, "HelloWorld");
    // elog_w(TAG, "HelloWorld");
    // elog_e(TAG, "HelloWorld");
    // elog_a(TAG, "0123456789");
  //   /* 输出调试信息 */
  //   elog_i("Debug", "LED toggled, count: %lu", count++);
  //   elog_v("Debug", "debugTask stack free: %lu", uxTaskGetStackHighWaterMark(debugTaskHandle));
  //   elog_v("Debug", "joystick stack free: %lu", uxTaskGetStackHighWaterMark(joystickTaskHandle));
  //   elog_v("Debug", "elog stack free: %lu", uxTaskGetStackHighWaterMark(elog_taskHandle));
  //
  //   if (count % 10 == 0) {
  //     elog_i("System", "System running for %lu seconds", HAL_GetTick() / 1000);
  //   }
  //
  //   if (count % 20 == 0) {
  //     elog_w("System", "This is a warning message at count %lu", count);
  //   }

    osDelay(1000);
  }
  /* USER CODE END StartDebugTask */
}

/* USER CODE BEGIN Header_StartJoystickTask */
/**
* @brief Function implementing the joystickTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartJoystickTask */
void StartJoystickTask(void const * argument)
{
  /* USER CODE BEGIN StartJoystickTask */
  button_init();
  /* Infinite loop */
  for(;;)
  {
    Joy_Update();
    get_btn();
//    log_i("Pressed the button %d", lwbtn_keys);
    osDelay(10);
  }
  /* USER CODE END StartJoystickTask */
}

/* USER CODE BEGIN Header_StartELogTask */
/**
* @brief Function implementing the elog_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartELogTask */
__weak void StartELogTask(void const * argument)
{
  /* USER CODE BEGIN StartELogTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartELogTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

