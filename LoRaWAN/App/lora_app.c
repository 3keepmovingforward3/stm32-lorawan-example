/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    lora_app.c
  * @author  MCD Application Team
  * @brief   Application of the LRWAN Middleware
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "platform.h"
#include "Region.h" /* Needed for LORAWAN_DEFAULT_DATA_RATE */
#include "sys_app.h"
#include "lora_app.h"
#include "stm32_seq.h"
#include "LmHandler.h"

/* USER CODE BEGIN Includes */
#include "stm32_seq.h"
#include "stm32_timer.h"
#include "utilities_def.h"
#include "lora_app_version.h"
#include "lorawan_version.h"
#include "subghz_phy_version.h"
#include "lora_info.h"
#include "stm32_lpm.h"
#include "sys_conf.h"
#include "sys_sensors.h"
/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/**
  * @brief LoRa State Machine states
  */
typedef enum TxEventType_e
{
  /**
    * @brief AppdataTransmition issue based on timer every TxDutyCycleTime
    */
  TX_ON_TIMER,
  /**
    * @brief AppdataTransmition external event plugged on OnSendEvent( )
    */
  TX_ON_EVENT
  /* USER CODE BEGIN TxEventType_t */

  /* USER CODE END TxEventType_t */
} TxEventType_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  join event callback function
  * @param  joinParams status of join
  */
static void OnJoinRequest(LmHandlerJoinParams_t *joinParams);

/**
  * @brief  tx event callback function
  * @param  params status of last Tx
  */
static void OnTxData(LmHandlerTxParams_t *params);

/**
  * @brief callback when LoRa application has received a frame
  * @param appData data received in the last Rx
  * @param params status of last Rx
  */
static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params);

/*!
 * Will be called each time a Radio IRQ is handled by the MAC layer
 *
 */
static void OnMacProcessNotify(void);

/* USER CODE BEGIN PFP */
/**
  * @brief  LoRa endNode send request
  * @param  none
  * @retval none
  */
static void SendTxData(void);

/**
  * @brief  TX timer callback function
  * @param  timer context
  * @retval none
  */
static void OnTxTimerEvent(void *context);

/**
  * @brief  LED Tx timer callback function
  * @param  LED context
  * @retval none
  */
static void OnTxTimerLedEvent(void *context);

/**
  * @brief  LED Rx timer callback function
  * @param  LED context
  * @retval none
  */
static void OnRxTimerLedEvent(void *context);

/**
  * @brief  LED Join timer callback function
  * @param  LED context
  * @retval none
  */
static void OnJoinTimerLedEvent(void *context);

/* USER CODE END PFP */

/* Private variables ---------------------------------------------------------*/
/**
  * @brief LoRaWAN handler Callbacks
  */
static LmHandlerCallbacks_t LmHandlerCallbacks =
{
  .GetBatteryLevel =           GetBatteryLevel,
  .GetTemperature =            GetTemperatureLevel,
  .GetUniqueId =               GetUniqueId,
  .GetDevAddr =                GetDevAddr,
  .OnMacProcess =              OnMacProcessNotify,
  .OnJoinRequest =             OnJoinRequest,
  .OnTxData =                  OnTxData,
  .OnRxData =                  OnRxData
};

/* USER CODE BEGIN PV */
/**
  * @brief User application buffer
  */
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

/**
  * @brief User application data structure
  */
static LmHandlerAppData_t AppData = { 0, 0, AppDataBuffer };

static ActivationType_t ActivationType = LORAWAN_DEFAULT_ACTIVATION_TYPE; //OTAA

/**
  * @brief LoRaWAN handler parameters
  */
static LmHandlerParams_t LmHandlerParams =
{
  .ActiveRegion =             ACTIVE_REGION,
  .DefaultClass =             LORAWAN_DEFAULT_CLASS,
  .AdrEnable =                LORAWAN_ADR_STATE,
  .TxDatarate =               LORAWAN_DEFAULT_DATA_RATE,
  .PingPeriodicity =          LORAWAN_DEFAULT_PING_SLOT_PERIODICITY
};

/**
  * @brief Specifies the state of the application LED
  */
static uint8_t AppLedStateOn = RESET;

/**
  * @brief Type of Event to generate application Tx
  */
static TxEventType_t EventType = TX_ON_TIMER;

/**
  * @brief Timer to handle the application Tx
  */
static UTIL_TIMER_Object_t TxTimer;

/**
  * @brief Timer to handle the application Tx Led to toggle
  */
static UTIL_TIMER_Object_t TxLedTimer;

/**
  * @brief Timer to handle the application Rx Led to toggle
  */
static UTIL_TIMER_Object_t RxLedTimer;

/**
  * @brief Timer to handle the application Join Led to toggle
  */
static UTIL_TIMER_Object_t JoinLedTimer;

/* USER CODE END PV */

/* Exported functions ---------------------------------------------------------*/
/* USER CODE BEGIN EF */

/* USER CODE END EF */

void LoRaWAN_Init(void)
{
  /* USER CODE BEGIN LoRaWAN_Init_1 */
  UTIL_TIMER_Create(&TxLedTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTxTimerLedEvent, NULL);
  UTIL_TIMER_Create(&RxLedTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnRxTimerLedEvent, NULL);
  UTIL_TIMER_Create(&JoinLedTimer, 0xFFFFFFFFU, UTIL_TIMER_PERIODIC, OnJoinTimerLedEvent, NULL);
  UTIL_TIMER_SetPeriod(&TxLedTimer, 500);
  UTIL_TIMER_SetPeriod(&RxLedTimer, 500);
  UTIL_TIMER_SetPeriod(&JoinLedTimer, 500);

  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LmHandlerProcess), UTIL_SEQ_RFU, LmHandlerProcess);
  //UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimer), UTIL_SEQ_RFU, SendTxData);

  /* Init Info table used by LmHandler*/
  LoraInfo_Init();

  /* USER CODE END LoRaWAN_Init_1 */

  /* Init the Lora Stack*/
  LmHandlerInit(&LmHandlerCallbacks);

  /* USER CODE BEGIN LoRaWAN_Init_Last */
  LmHandlerConfigure(&LmHandlerParams);

  UTIL_TIMER_Start(&JoinLedTimer);

  LmHandlerJoin(ActivationType);

  /* send every time timer elapses */
  UTIL_TIMER_Create(&TxTimer,  0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTxTimerEvent, NULL);
  UTIL_TIMER_SetPeriod(&TxTimer,  APP_TX_DUTYCYCLE);
  UTIL_TIMER_Start(&TxTimer);

  /* USER CODE END LoRaWAN_Init_Last */
}

/* USER CODE BEGIN PB_Callbacks */
static void SendTxData(void)
{
	  uint32_t i = 0;
	  int16_t temp = 0;
	  sensor_t sensor_data;
	  UTIL_TIMER_Time_t nextTxIn = 0;

	  EnvSensors_Read(&sensor_data);

	  AppData.Port = LORAWAN_USER_APP_PORT;
	}

static void OnTxTimerEvent(void *context)
{
  /* USER CODE BEGIN OnTxTimerEvent_1 */

  /* USER CODE END OnTxTimerEvent_1 */
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimer), CFG_SEQ_Prio_0);

  /*Wait for next tx slot*/
  UTIL_TIMER_Start(&TxTimer);
  /* USER CODE BEGIN OnTxTimerEvent_2 */

  /* USER CODE END OnTxTimerEvent_2 */
}
/* USER CODE END PB_Callbacks */

/* Private functions ---------------------------------------------------------*/
/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */

static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params)
{
  /* USER CODE BEGIN OnRxData_1 */

  /* USER CODE END OnRxData_1 */
}

/* USER CODE BEGIN PrFD_LedEvents */
static void OnTxTimerLedEvent(void *context)
{
  /* USER CODE BEGIN OnTxTimerLedEvent_1 */
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, SET);
  /* USER CODE END OnTxTimerLedEvent_1 */

}

static void OnRxTimerLedEvent(void *context)
{
  /* USER CODE BEGIN OnRxTimerLedEvent_1 */

  /* USER CODE END OnRxTimerLedEvent_1 */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, SET);
  /* USER CODE BEGIN OnRxTimerLedEvent_2 */

  /* USER CODE END OnRxTimerLedEvent_2 */
}

static void OnJoinTimerLedEvent(void *context)
{
  /* USER CODE BEGIN OnJoinTimerLedEvent_1 */

  /* USER CODE END OnJoinTimerLedEvent_1 */
  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
  /* USER CODE BEGIN OnJoinTimerLedEvent_2 */

  /* USER CODE END OnJoinTimerLedEvent_2 */
}
/* USER CODE END PrFD_LedEvents */

static void OnTxData(LmHandlerTxParams_t *params)
{
  /* USER CODE BEGIN OnTxData_1 */
	if ((params != NULL) && (params->IsMcpsConfirm != 0)){
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, RESET);

		UTIL_TIMER_Start(&TxLedTimer);
	}
  /* USER CODE END OnTxData_1 */
}

static void OnJoinRequest(LmHandlerJoinParams_t *joinParams)
{
  /* USER CODE BEGIN OnJoinRequest_1 */
	if (joinParams != NULL)
	  {
	    if (joinParams->Status == LORAMAC_HANDLER_SUCCESS)
	    {
	      UTIL_TIMER_Stop(&JoinLedTimer);

	      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, SET);
	    }
	  }
  /* USER CODE END OnJoinRequest_1 */
}

static void OnMacProcessNotify(void)
{
  /* USER CODE BEGIN OnMacProcessNotify_1 */
	UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LmHandlerProcess), CFG_SEQ_Prio_0);
  /* USER CODE END OnMacProcessNotify_1 */
}
