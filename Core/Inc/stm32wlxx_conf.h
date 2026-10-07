/*
 * stm32wlxx_conf.h
 *
 *  Created on: Feb 22, 2022
 *      Author: ben.blouin
 */

#ifndef INC_STM32WLXX_CONF_H_
#define INC_STM32WLXX_CONF_H_

#include "stm32wlxx_hal.h"

/**
  * Radio maximum wakeup time (in ms)
  */
#define RF_WAKEUP_TIME                     10U

/**
  * Indicates whether or not TCXO is supported by the board
  * 0: TCXO not supported
  * 1: TCXO supported
  */
#define IS_TCXO_SUPPORTED                   1U

/**
  * Indicates whether or not DCDC is supported by the board
  * 0: DCDC not supported
  * 1: DCDC supported
  */
#define IS_DCDC_SUPPORTED                   1U


#endif /* INC_STM32WLXX_CONF_H_ */
