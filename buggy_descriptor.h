/*
 * buggy_descriptor.h
 *
 *  Created on: 25 nov. 2016
 *      Author: raph
 */

#ifndef ALGOIDCOM_BUGGY_DESCRIPTOR_H_
#define ALGOIDCOM_BUGGY_DESCRIPTOR_H_

typedef enum o_type{
	UNKNOWN = -1,
	MOTOR,
	SERVOM,
	LED
}t_type;

typedef enum o_encoder{
	MOTOR_ENCODER_LEFT,
	MOTOR_ENCODER_RIGHT,
	NBCOUNTER
}t_encoder;

typedef enum o_din{
	DIN_0,
	DIN_1,
	NBDIN
}t_din;

typedef enum o_ain{
	BATT_0,
	NBAIN
}t_ain;

typedef enum o_pwm{
	SONAR_0,
	NBPWM
}t_pwm;

typedef enum o_motor{
	MOTOR_LEFT,
	MOTOR_RIGHT,
	NBMOTOR
}t_motor;

typedef enum o_servo{
	SERVO_0,
	SERVO_1,
	SERVO_2,
	NBSERVO
}t_servo;

typedef enum o_led{
	LED_0,
	LED_1,
	LED_2,
	NBLED,
}t_led;

#define BUGGY_STOP 		0
#define BUGGY_FORWARD 	1
#define BUGGY_BACK 		2

#endif /* ALGOIDCOM_BUGGY_DESCRIPTOR_H_ */
