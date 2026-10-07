/*
 * sys_sensors.c
 *
 *  Created on: Feb 22, 2022
 *      Author: ben.blouin
 */

#include "stdint.h"
#include "sys_conf.h"
#include "sys_sensors.h"

#define TEMPERATURE_DEFAULT_VAL   18.0f                 /*!< default temperature */

void EnvSensors_Read(sensor_t *sensor_data)
{
	float TEMPERATURE_Value = TEMPERATURE_DEFAULT_VAL;
	sensor_data->temperature = TEMPERATURE_Value;
}
