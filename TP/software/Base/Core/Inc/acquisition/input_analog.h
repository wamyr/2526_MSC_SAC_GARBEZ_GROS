/*
 * input_analog.h
 *
 *  Created on: Nov 11, 2025
 *      Author: nicolas
 */

#ifndef INC_INPUT_ANALOG_H_
#define INC_INPUT_ANALOG_H_

#include "main.h"
#include "user_interface/shell.h"
#include <stdio.h>
#include <string.h>

extern ADC_HandleTypeDef hadc1;

//define pour la mesure du courant
#define ADC_RESOLUTION      4096.0f
#define ADC_VREF            3.3f
#define SENSOR_SENSITIVITY  0.025f   // GO-SME 10 (0.025 mV/A)
#define V_OFFSET_THEO       (ADC_VREF / 2.0f)

int input_analog_init();
float Calculer_Courant_Moyen();

#endif /* INC_INPUT_ANALOG_H_ */
