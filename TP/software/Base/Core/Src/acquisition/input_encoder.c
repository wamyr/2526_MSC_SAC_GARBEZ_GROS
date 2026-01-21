/*
 * input_encoder.c
 *
 *  Created on: Nov 11, 2025
 *      Author: nicolas
 */

#include "acquisition/input_encoder.h"

uint16_t ancien_CNT_encodeur = 0;
uint16_t ancien_temps_us = 0; // Pour stocker la valeur du TIM16
float rpm =0;

/**
 *
 * @brief Init shell's function of encoder
 *
 * @details This function add the function encoder_show_speed() in the shell
 *
 * @return 1 if the operation is valid, 0 otherwise.
 */
int input_encoder_init(){
	return shell_add(&hshell1, "EncValue", encoder_show_speed, "Give motor speed");
}

/**
 *
 * @brief print on the shell rpm value
 *
 * @details This function print in the shell the value of rpm calculated by Calcul_Vitesse_Precise(void).
 *
 * @param h_shell The pointer to the shell instance.
 * @param argc The number of command arguments.
 * @param argv The array of command arguments.
 * @return HAL_OK if the operation is valid, HAL_ERROR otherwise.
 */
int encoder_show_speed(h_shell_t* h_shell, int argc, char** argv){
	int size;

	if(argc!=TWO_ARGUMENTS){
		size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "Need 2 arguments : enc_value & recv \r\n");
		h_shell->drv.transmit(h_shell->print_buffer, size);
		return HAL_ERROR;
	}
	size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "motor speed with speed : %.2f tr/min\r\n", rpm);
	size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "motor speed : %.2f tr/min\r\n", rpm);
	h_shell->drv.transmit(h_shell->print_buffer, size);

	return HAL_OK;
}

/**
 *
 * @brief Calculate the speed of the motor at a moment.
 *
 * @details This function read the tick of the timer configured on encoder mode to calculate a delta t. With this, the function can calculate the delta position of the encoder and so the speed of the motor. The function did that without forgotten the different conversions
 *
 */
void Calcul_Vitesse_Precise(void) {
	uint16_t temps_actuel_us = __HAL_TIM_GET_COUNTER(&htim7); // Le TIM16 tourne en boucle (0 à 65535)
	uint16_t dt_us = temps_actuel_us - ancien_temps_us;
	float dt_secondes = (float)dt_us / USECOND_SECOND_CONVERSION; 	// Convertir en secondes pour la physique


	uint16_t cnt_actuel = __HAL_TIM_GET_COUNTER(&htim3); //Lire la position (TIM3)
	int16_t delta_pos = (int16_t)(cnt_actuel - ancien_CNT_encodeur);


	rpm = ((float)delta_pos / STEP_ROTATION_CONVERSION) / dt_secondes *SECOND_MINUTE_CONVERSION*SENS_ROTATION_CORRECTION;

	//sauvegarde pour le prochain tour
	ancien_temps_us = temps_actuel_us;
	ancien_CNT_encodeur = cnt_actuel;
}

