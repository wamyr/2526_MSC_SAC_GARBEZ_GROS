/*
 * asserv.c
 *
 *  Created on: Nov 11, 2025
 *      Author: nicolas
 */

#include "motor_control/asserv.h"

void Current_PI_Controller_Init(PI_Controller *pi) {

	/* Clear controller variables */
	pi->integrator = 0.0f;
	pi->prevError  = 0.0f;

	pi->prevMeasurement = 0.0f;

	pi->out = 0.0f;

	/* Limit values */
	pi->limMaxInt= 2.5;
	pi->limMinInt= -2.5;

	pi->limMax = 12 ;
	pi->limMin = -12 ;

	/* PI coefficient values */
	pi->Kp = 0.0112 ;
	pi->Ki = 4.738 ;

	/* Sample Time */
	pi->T = 0.000025 ;

}

void Speed_PI_Controller_Init(PI_Controller *pi) {

	/* Clear controller variables */
	pi->integrator = 0.0f;
	pi->prevError  = 0.0f;

	pi->prevMeasurement = 0.0f;

	pi->out = 0.0f;

	/* Limit values */
	pi->limMaxInt= 200;
	pi->limMinInt= -200;

	pi->limMax = 12 ;
	pi->limMin = -12 ;

	/* PI coefficient values */
	pi->Kp = 0.15 ;
	pi->Ki = 0.06 ;

	/* Sample Time */
	pi->T = 0.05 ;

}

float PI_Controller_Update(PI_Controller *pi, float setpoint, float measurement) {

	/*
	 * Error signal
	 */
	float error = setpoint - measurement;

	/*
	 * Proportional
	 */
	float proportional = pi->Kp * error;

	/*
	 * Integral
	 */
	pi->integrator = pi->integrator + 0.5f * pi->Ki * pi->T * (error + pi->prevError);

	/* Anti-wind-up via integrator clamping */
	if (pi->integrator > pi->limMaxInt) {

		pi->integrator = pi->limMaxInt;

	} else if (pi->integrator < pi->limMinInt) {

		pi->integrator = pi->limMinInt;

	}

	/*
	 * Compute output and apply limits
	 */
	pi->out = proportional + pi->integrator ;

	if (pi->out > pi->limMax) {

		pi->out = pi->limMax;

	} else if (pi->out < pi->limMin) {

		pi->out = pi->limMin;

	}

	/* Store error and measurement for later use */
	pi->prevError       = error;
	pi->prevMeasurement = measurement;

	/* Return controller output */
	return pi->out;
}



