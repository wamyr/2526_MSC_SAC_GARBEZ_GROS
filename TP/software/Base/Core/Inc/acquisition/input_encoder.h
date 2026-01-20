/*
 * input_encoder.h
 *
 *  Created on: Nov 11, 2025
 *      Author: nicolas
 */

#ifndef INC_ACQUISITION_INPUT_ENCODER_H_
#define INC_ACQUISITION_INPUT_ENCODER_H_

#include <stdint.h>
#include "tim.h"
#include "user_interface/shell.h"
#include "./app.h"

#define SENS_ROTATION_CORRECTION  (-1.0f) //l'encodeur est inversé par rapport au moteur
#define USECOND_SECOND_CONVERSION 1000000.0f
#define SECOND_MINUTE_CONVERSION 60.0f
#define STEP_ROTATION_CONVERSION 4096.0f // 4096 pas = 1 tour with this encoder mode

extern uint16_t ancien_temps_us;
extern uint16_t ancien_CNT_encodeur;
extern float rpm;

int input_encoder_init();
int encoder_show_speed(h_shell_t* h_shell, int argc, char** argv);
void Calcul_Vitesse_Precise(void);

#endif /* INC_ACQUISITION_INPUT_ENCODER_H_ */
