/*
 * app.c
 *
 *  Created on: Nov 11, 2025
 *      Author: nicolas
 */

#include "app.h"
#include "acquisition/input_encoder.h"
#include "acquisition/input_analog.h"
#include "user_interface/shell.h"

//We can declare this variable and structure here because we use them only in this file
PI_Controller Current_PI_Controller ;
int start_asserv_flag = 0 ;
static char shell_uart2_received_char;

/**
 * @brief Init the different shell's functions
 *
 * This function define wich uart we'll use for the shell and init the different functions in this TP.
 *
 */
void init_device(void){
	// Initialisation user interface
	// SHELL
	hshell1.drv.transmit = shell_uart2_transmit;
	hshell1.drv.receive = shell_uart2_receive;
	shell_init(&hshell1);
	HAL_UART_Receive_IT(&huart2, (uint8_t *)&shell_uart2_received_char, 1);

	// LED
	led_init();

	// BUTTON
	//	button_init();
	//
	// Initialisation motor control
	// MOTOR
	motor_init();
	// ASSERV (PID)
	Current_PI_Controller_Init(&Current_PI_Controller);
	//
	// Initialisation data acquistion
	// ANALOG INPUT
	input_analog_init();
	// ENCODER INPUT
	input_encoder_init();
}

uint8_t shell_uart2_transmit(const char *pData, uint16_t size)
{
	HAL_UART_Transmit(&huart2, (uint8_t *)pData, size, HAL_MAX_DELAY);
	return size;
}

uint8_t shell_uart2_receive(char *pData, uint16_t size)
{
	*pData = shell_uart2_received_char;
	return 1;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {
		//		HAL_UART_Transmit(&huart2, (uint8_t *)&shell_uart2_received_char, 1, HAL_MAX_DELAY);
		HAL_UART_Receive_IT(&huart2, (uint8_t *)&shell_uart2_received_char, 1);
		shell_run(&hshell1);
	}
}

/**
 * @brief loop function
 *
 * This function is used to update speed and current controller. At every period of each controller, their respectively flag is turn on 1 and allow to update their output.
 *
 */
void loop(){

	float current_command = 0 ;

	if(start_asserv_flag != 0)
	{
		if(speed_asserv)
		{
			//current_command = PI_Controller_Update(&Speed_PI_Controller, motor_command,rpm);
			current_command = 3 ;
			speed_asserv = 0 ;
			/*
			int size = snprintf((&hshell1)->print_buffer, SHELL_PRINT_BUFFER_SIZE, "current : %.2f.\r\n", current_command);
			(&hshell1)->drv.transmit((&hshell1)->print_buffer, size);
			*/
		}
		if(current_asserv)
		{
			float measure_current = Calculer_Courant_Moyen();
			float corrected_motor_command = PI_Controller_Update(&Current_PI_Controller, current_command, measure_current); // motor command est 100x trop grand !!
			int crr_value_command = (int)((corrected_motor_command/VDC + 1)*0.5*100*PERCENT_TO_MAX_CRR_VALUE_CONVERSION + 0.5 ); //valeur magique à changer
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, crr_value_command); // to make sure in the end we have the right value
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, COMMAND_MAX_VALUE-crr_value_command);
			current_asserv = 0 ;

			int size = snprintf((&hshell1)->print_buffer, SHELL_PRINT_BUFFER_SIZE, "voltage : %.2f measure : %.2f \r\n", corrected_motor_command, measure_current);
			(&hshell1)->drv.transmit((&hshell1)->print_buffer, size);

		}

	}

}
