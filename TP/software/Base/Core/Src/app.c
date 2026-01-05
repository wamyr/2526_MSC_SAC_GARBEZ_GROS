/*
 * app.c
 *
 *  Created on: Nov 11, 2025
 *      Author: nicolas
 */

#include "app.h"

#include "user_interface/shell.h"

PI_Controller Current_PI_Controller ;
int start_asserv_flag = 0 ;

static char shell_uart2_received_char;

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
	//	input_analog_init();
	// ENCODER INPUT
	//	input_encoder_init();
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

void loop(){

	if(start_asserv_flag != 0)
	{
		float corrected_motor_command = PI_Controller_Update(&Current_PI_Controller, motor_command, Calculer_Courant_Moyen());
		int duty_cycle_command = (int)((corrected_motor_command/VDC + 1)*0.5 + 0.5 ); //valeur magique à changer
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, duty_cycle_command*PERCENT_TO_MAX_CRR_VALUE_CONVERSION); // to make sure in the end we have the right value
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, COMMAND_MAX_VALUE-duty_cycle_command*PERCENT_TO_MAX_CRR_VALUE_CONVERSION);
/*
		int size = snprintf((&hshell1)->print_buffer, SHELL_PRINT_BUFFER_SIZE, "Current duty cycle : %d.\r\n", duty_cycle_command);
		(&hshell1)->drv.transmit((&hshell1)->print_buffer, size);
*/
	}

}
