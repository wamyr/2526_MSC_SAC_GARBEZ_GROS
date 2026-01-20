/*
 * asserv.c
 *
 *  Created on: Nov 11, 2025
 *      Author: nicolas
 */

#include "motor_control/asserv.h"

/**
 * @brief clear pi current variables and init coefficients
 *
 * This fonction init the current's controller. It points on every variables of the pi structure and put it at 0. It points on ji, kp, T coefficients to put it at their values.
 *
 * @param pi the pointer of the structure pi
 */
void Current_PI_Controller_Init(PI_Controller *pi) {

	/* Clear controller variables */
	pi->integrator = 0.0f;
	pi->prevError  = 0.0f;

	pi->prevMeasurement = 0.0f;

	pi->out = 0.0f;

	/* PI coefficient values */
	pi->Kp = 0.0112664025854474 ;
	pi->Ki = 4.7381474597804 ;

	/* Sample Time */
	pi->T = 0.001 ;

}

/**
 * @brief clear pi speed variables and init coefficients
 *
 * This fonction init the speed's controller. It points on every variables of the pi structure and put it at 0. It points on ji, kp, T coefficients to put it at their values.
 *
 * @param pi the pointer of the structure pi
 */
void Speed_PI_Controller_Init(PI_Controller *pi) {

	/* Clear controller variables */
	pi->integrator = 0.0f;
	pi->prevError  = 0.0f;

	pi->prevMeasurement = 0.0f;

	pi->out = 0.0f;

	/* PI coefficient values */
	pi->Kp = 0.0f ;
	pi->Ki = 0.0f ;

	/* Sample Time */
	pi->T = 0.001 ;

}

/**
 * @brief The function which calculate at a period T the output of pi controller.
 *
 * This fonction is called at every period T of each controller to calculate their output.
 *
 * @param pi the pointer of the structure pi
 * @param setpoint the float of the order
 * @param setpoint the float of the measurement
 */
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



