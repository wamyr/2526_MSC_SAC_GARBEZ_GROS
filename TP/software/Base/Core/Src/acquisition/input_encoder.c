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

int input_encoder_init(){
	return shell_add(&hshell1, "EncValue", encoder_show_speed, "Give motor speed");
}

int encoder_show_speed(h_shell_t* h_shell, int argc, char** argv){
	int size;

	if(argc!=TWO_ARGUMENTS){
		size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "Need 2 arguments : enc_value & recv \r\n");
		h_shell->drv.transmit(h_shell->print_buffer, size);
		return HAL_ERROR;
	}
	size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "motor speed : %.2f tr/min\r\n", rpm);
	h_shell->drv.transmit(h_shell->print_buffer, size);

	return HAL_OK;
}

void Calcul_Vitesse_Precise(void) {
	// 1. Lire le temps actuel (en microsecondes)
	// Le TIM16 tourne en boucle (0 à 65535)
	uint16_t temps_actuel_us = __HAL_TIM_GET_COUNTER(&htim7);

	// 2. Calculer le temps écoulé (Delta T)
	// L'arithmétique uint16_t gère automatiquement le débordement (ex: 10 - 65500 donne le bon écart)
	uint16_t dt_us = temps_actuel_us - ancien_temps_us;

	// Convertir en secondes pour la physique (ex: 1000 us = 0.001 s)
	float dt_secondes = (float)dt_us / 1000000.0f;

	// 3. Lire la position (TIM3)
	uint16_t cnt_actuel = __HAL_TIM_GET_COUNTER(&htim3);
	int16_t delta_pos = (int16_t)(cnt_actuel - ancien_CNT_encodeur);

	// 4. Calcul final (tr/min)
	// 4096 pas = 1 tour
	rpm = ((float)delta_pos / 4096.0f) / dt_secondes * 60.0f;
	rpm = rpm * 2* PI / 60 ;
	// 5. Mise à jour pour le prochain tour
	ancien_temps_us = temps_actuel_us;
	ancien_CNT_encodeur = cnt_actuel;
}
