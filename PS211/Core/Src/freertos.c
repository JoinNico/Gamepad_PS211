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
#include "../User/oled.h"
#include "../User/adc-process.h"
#include "../User/icm20602.h"
#include "../User/nRF24L01.h"
#include "../../3rdParty/elog/elog.h"
#include "../../3rdParty/lwbtn/lwbtn_opts.h"
#include "../../3rdParty/WouoUI/WouoUI.h"
#include "../../3rdParty/WouoUI/WouoUI_user.h"
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
osThreadId WouoUITaskHandle;
osSemaphoreId elog_lockHandle;
osSemaphoreId elog_asyncHandle;
osSemaphoreId elog_dma_lockHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDebugTask(void const * argument);
void StartJoystickTask(void const * argument);
void StartELogTask(void const * argument);
void StartWouoUITask(void const * argument);

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
  osThreadDef(debugTask, StartDebugTask, osPriorityHigh, 0, 256);
  debugTaskHandle = osThreadCreate(osThread(debugTask), NULL);

  /* definition and creation of joystickTask */
  osThreadDef(joystickTask, StartJoystickTask, osPriorityNormal, 0, 256);
  joystickTaskHandle = osThreadCreate(osThread(joystickTask), NULL);

  /* definition and creation of elog_task */
  osThreadDef(elog_task, StartELogTask, osPriorityRealtime, 0, 512);
  elog_taskHandle = osThreadCreate(osThread(elog_task), NULL);

  /* definition and creation of WouoUITask */
  osThreadDef(WouoUITask, StartWouoUITask, osPriorityLow, 0, 512);
  WouoUITaskHandle = osThreadCreate(osThread(WouoUITask), NULL);

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
  while(nRF24_Check())
  {
    log_i("NRF24L01 Error");
    osDelay(500);
  }

  nRF24_TX_Mode();
  log_i("NRF24L01 TX Mode OK");
  uint8_t tmp_buf[33];
  /* Infinite loop */
  for(;;)
  {
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    sprintf(tmp_buf, "%s", "Hello world\r\n");
    nRF24_TXPacket(tmp_buf);

    // elog_v(TAG, "HelloWorld");
    // elog_d(TAG, "HelloWorld");
    // elog_i(TAG, "HelloWorld");
    // elog_w(TAG, "HelloWorld");
    // elog_e(TAG, "HelloWorld");
    // elog_a(TAG, "0123456789");

    // elog_v("Debug", "debugTask stack free: %lu", uxTaskGetStackHighWaterMark(debugTaskHandle));
    // elog_v("Debug", "joystick stack free: %lu", uxTaskGetStackHighWaterMark(joystickTaskHandle));
    // elog_v("Debug", "elog stack free: %lu", uxTaskGetStackHighWaterMark(elog_taskHandle));

    // adc_print_raw_buffer_simple("ADC", (uint16_t* )hadcProc.raw_buffer, 11);
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

    // icm20602_data_change();
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

/* USER CODE BEGIN Header_StartWouoUITask */
/**
* @brief Function implementing the WouoUITask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartWouoUITask */
void StartWouoUITask(void const * argument)
{
  /* USER CODE BEGIN StartWouoUITask */
  WouoUI_AttachSendBuffFun(OLED_SendBuff);
  TestUI_Init();
  /* Infinite loop */
  for(;;)
  {
    WouoUI_Proc(5);
    osDelay(5);
  }
  /* USER CODE END StartWouoUITask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

