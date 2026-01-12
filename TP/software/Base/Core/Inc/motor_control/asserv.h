/*
 * asserv.h
 *
 *  Created on: Nov 11, 2025
 *      Author: nicolas
 */

#ifndef INC_MOTOR_CONTROL_ASSERV_H_
#define INC_MOTOR_CONTROL_ASSERV_H_

typedef struct {

	/* Controller gains */
	float Kp;
	float Ki;

	/* Output limits */
	float limMin;
	float limMax;

	/* Integrator limits */
	float limMinInt;
	float limMaxInt;

	/* Sample time (in seconds) */
	float T;

	/* Controller "memory" */
	float integrator;
	float prevError;			/* Required for integrator */
	float prevMeasurement;		/* Required for differentiator */

	/* Controller output */
	float out;

} PI_Controller;


#define VDC 48 //doublon

void  Current_PI_Controller_Init(PI_Controller *pi);
void Speed_PI_Controller_Init(PI_Controller *pi);
float PI_Controller_Update(PI_Controller *pi, float setpoint, float measurement);

#endif /* INC_MOTOR_CONTROL_ASSERV_H_ */
