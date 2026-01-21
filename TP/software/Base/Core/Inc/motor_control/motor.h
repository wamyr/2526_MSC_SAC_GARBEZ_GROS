/*
 * motor.h
 *
 *  Created on: Nov 11, 2025
 *      Author: nicolas
 */

#ifndef INC_MOTOR_CONTROL_MOTOR_H_
#define INC_MOTOR_CONTROL_MOTOR_H_

#define ARR_VALUE 8499
#define PSC_VALUE 0
#define COMMAND_MAX_VALUE 8500
#define COMMAND_HALF_MAX_VALUE 4250
#define TWO_ARGUMENTS 2
#define DELAY_RAMP_CRR_VALUE 50000 // less than 1 us
#define INCREMENT 200
#define PERCENT_TO_MAX_CRR_VALUE_CONVERSION 85 // conversion to max CRR Value
#define DUTY_CYCLE_MAX_VALUE 100
#define MOTOR_SPEED_MAX_VALUE 3000
#define PI 3.1415

#include <stdint.h>
#include "tim.h"
#include "user_interface/shell.h"
#include "./app.h"

extern int motor_command;

int motor_init();

#endif /* INC_MOTOR_CONTROL_MOTOR_H_ */
