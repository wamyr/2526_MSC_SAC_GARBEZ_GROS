/*
 * motor.h
 *
 *  Created on: Nov 11, 2025
 *      Author: nicolas
 */

#ifndef INC_MOTOR_CONTROL_MOTOR_H_
#define INC_MOTOR_CONTROL_MOTOR_H_

#define ARR_VALUE 8499 //résolution de plus de 10 bits
#define PSC_VALUE 0

#include "tim.h"
#include "user_interface/shell.h"

int motor_init();

#endif /* INC_MOTOR_CONTROL_MOTOR_H_ */
