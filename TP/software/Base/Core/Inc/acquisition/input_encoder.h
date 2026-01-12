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

#define PI 3.1415

extern uint16_t ancien_temps_us;
extern uint16_t ancien_CNT_encodeur;
extern float rpm;

int input_encoder_init();
int encoder_show_speed(h_shell_t* h_shell, int argc, char** argv);
void Calcul_Vitesse_Precise(void);

#endif /* INC_ACQUISITION_INPUT_ENCODER_H_ */
