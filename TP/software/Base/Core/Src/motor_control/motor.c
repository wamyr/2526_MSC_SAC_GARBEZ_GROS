/*
 * motor.c
 *
 *  Created on: Nov 11, 2025
 *      Author: nicolas
 */

#include "motor_control/motor.h"
#include "acquisition/input_encoder.h"
#include "stdlib.h"

//we can initiate those functions and variables here because we will only use them INSIDE this file

int ccr_value = 0 ;
int motor_command = 0 ;

int motor_start(h_shell_t* h_shell, int argc, char** argv);
int motor_speed(h_shell_t* h_shell, int argc, char** argv);
void motor_ramp_update(h_shell_t* h_shell, int speed_order);
static void delay_periods(h_shell_t* h_shell, uint16_t us);

/**
 *
 * @brief Init shell's function of motor command
 *
 * This function add the function motor_start() and motor_speed() in the shell
 *
 * @return 1 if the operation is valid, 0 otherwise.
 */
int motor_init(){
	shell_add(&hshell1, "motor", motor_start, "Control motor");
	return shell_add(&hshell1, "speed", motor_speed, "Set speed motor");
}

/**
 *
 * @brief Start the PWMs for the command of the DC motor.
 *
 * This function allow to start or stop the command of the chopper bridge arm. If we want to start the motor, the CCR register is set to this half value to avoid the start of the motor without this will.
 *
 * @param h_shell The pointer to the shell instance.
 * @param argc The number of command arguments.
 * @param argv The array of command arguments.
 * @return HAL_OK if the operation is valid, HAL_ERROR otherwise.
 */
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

/**
 *
 * @brief Set duty cycle of the PWMs.
 *
 * This function verify if the value of the duty cycle is between the valid values. If it's the case, we make an operation to convert the duty cycle into CCR Value.
 *
 * @param h_shell The pointer to the shell instance.
 * @param argc The number of command arguments.
 * @param argv The array of command arguments.
 * @return HAL_OK if the operation is valid, HAL_ERROR otherwise.
 */
int motor_speed(h_shell_t* h_shell, int argc, char** argv){
	int size;

	if(argc!=TWO_ARGUMENTS){
		size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "Need 2 arguments : motor duty cycle in pourcent\r\n");
		h_shell->drv.transmit(h_shell->print_buffer, size);
		return HAL_ERROR;
	}

	//int speed_order = atoi(argv[1])*PERCENT_TO_MAX_CRR_VALUE_CONVERSION;
	int user_command = atoi(argv[1]);
	if((user_command > 0) && (user_command < DUTY_CYCLE_MAX_VALUE)){


		//motor_ramp_update(h_shell, user_command*PERCENT_TO_MAX_CRR_VALUE_CONVERSION);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, user_command*PERCENT_TO_MAX_CRR_VALUE_CONVERSION); // to make sure in the end we have the right value
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, COMMAND_MAX_VALUE-user_command*PERCENT_TO_MAX_CRR_VALUE_CONVERSION);

		size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "motor speed : %d. Don't forget to start the motor before\r\n", user_command);
		h_shell->drv.transmit(h_shell->print_buffer, size);

/*
		start_asserv_flag = 1 ;
		motor_command = user_command ; // for now

		size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "Controller working. Don't forget to start the motor before\r\n");
		h_shell->drv.transmit(h_shell->print_buffer, size);
*/
		return HAL_OK;
	}

	size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "Duty cycle value does not match the require value\r\n");
	h_shell->drv.transmit(h_shell->print_buffer, size);
	return HAL_ERROR;

}

/**
 *
 * @brief Make a ramp to pass to a value of order to another one.
 *
 * This function get the value of the current value of CCR register and change that one by an increment at each period.
 *
 * @param h_shell The pointer to the shell instance.
 * @param speed_order The int of the speed order
 *
 */
void motor_ramp_update(h_shell_t* h_shell, int speed_order) {

	int speed = __HAL_TIM_GET_COMPARE(&htim1, TIM_CHANNEL_1);
	int increment = (speed_order > speed) ? INCREMENT : -INCREMENT; // increment in the other sens if we turn in counterclockwise
	while(abs(speed_order - speed) > INCREMENT) {
		speed += increment;
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, speed);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, COMMAND_MAX_VALUE - speed);
		delay_periods(h_shell, DELAY_RAMP_CRR_VALUE);
	}
}

/**
 *
 * @brief Function to replace HAL_Delay().
 *
 * This function allow to wait until the timer reach a value calculated with the frequency of the timer and coherent value to wait.
 *
 * @param h_shell The pointer to the shell instance.
 * @param crr_to_reach_value The value of the CRR register to reach
 *
 */
static void delay_periods(h_shell_t* h_shell, uint16_t crr_to_reach_value)
{
	TIM16->CNT = 0 ;
	while (TIM16->CNT < crr_to_reach_value){
		/*
		int size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "wait ramp on going\r\n");
		h_shell->drv.transmit(h_shell->print_buffer, size);
		*/
		//less precise with the print option
	}
}

