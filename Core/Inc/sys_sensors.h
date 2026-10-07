/*
 * sys_sensors.h
 *
 *  Created on: Feb 22, 2022
 *      Author: ben.blouin
 */

#ifndef INC_SYS_SENSORS_H_
#define INC_SYS_SENSORS_H_

/**
  * Sensor data parameters
  */
typedef struct
{
  //float pressure;         /*!< in mbar */
  float temperature;      /*!< in degC */
  //float humidity;         /*!< in % */
  //int32_t latitude;       /*!< latitude converted to binary */
  //int32_t longitude ;     /*!< longitude converted to binary */
  //int16_t altitudeGps;    /*!< in m */
  //int16_t altitudeBar ;   /*!< in m * 10 */
  /**more may be added*/
  /* USER CODE BEGIN sensor_t */

  /* USER CODE END sensor_t */
} sensor_t;

/**
  * @brief  initialises the environmental sensor
  */
void  EnvSensors_Init(void);

/**
  * @brief  Environmental sensor  read.
  * @param  sensor_data sensor data
  */
void EnvSensors_Read(sensor_t *sensor_data);


#endif /* INC_SYS_SENSORS_H_ */
