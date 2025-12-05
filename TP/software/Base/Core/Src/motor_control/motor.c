/*
 * motor.c
 *
 *  Created on: Nov 11, 2025
 *      Author: nicolas
 */

#include "motor_control/motor.h"
#include "stdlib.h"

int ccr_value = 0 ;

int motor_start(h_shell_t* h_shell, int argc, char** argv); //we can initiate those functions here because we will only use them INSIDE this file
int motor_speed(h_shell_t* h_shell, int argc, char** argv);

int motor_init(){
	shell_add(&hshell1, "motor", motor_start, "Control motor");
	return shell_add(&hshell1, "speed", motor_speed, "Set speed motor");
}

int motor_start(h_shell_t* h_shell, int argc, char** argv){
	int size;

	if(argc!=TWO_ARGUMENTS){
		size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "Need 2 arguments : motor start/stop\r\n");
		h_shell->drv.transmit(h_shell->print_buffer, size);
		return HAL_ERROR;
	}
	if(strcmp(argv[1],"start")==0){
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, COMMAND_HALF_MAX_VALUE);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, COMMAND_HALF_MAX_VALUE);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1); //We try to start the PWM if a nicest way but it didn't work so we start them on by one.
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2); // dead time : 100ns so 17 tick
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
		size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "motor started\r\n");
		h_shell->drv.transmit(h_shell->print_buffer, size);
		return HAL_OK;
	}
	else if(strcmp(argv[1],"stop")==0){
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
		size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "motor stopped \r\n");
		h_shell->drv.transmit(h_shell->print_buffer, size);
		return HAL_OK;
	}

	size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "Need 2 arguments : motor start/stop\r\n");
	h_shell->drv.transmit(h_shell->print_buffer, size);
	return HAL_ERROR;
}


int motor_speed(h_shell_t* h_shell, int argc, char** argv){
	int size;
	int speed = atoi(argv[1]);
	if(argc!=TWO_ARGUMENTS){
		size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "Need 2 arguments : motor start/stop\r\n");
		h_shell->drv.transmit(h_shell->print_buffer, size);
		return HAL_ERROR;
	}

	if((speed > 0) && (speed < COMMAND_MAX_VALUE)){
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, speed);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, COMMAND_MAX_VALUE-speed);
		size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "motor speed : %d. Don't forget to start the motor before\r\n", speed);
		h_shell->drv.transmit(h_shell->print_buffer, size);
		return HAL_OK;
	}

	size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "Speed value does not match the require value\r\n");
	h_shell->drv.transmit(h_shell->print_buffer, size);
	return HAL_ERROR;

}



