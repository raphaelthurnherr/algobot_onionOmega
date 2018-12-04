/*
 * buggy_descriptor.h
 *
 *  Created on: 25 nov. 2016
 *      Author: raph
 * test Git
 */

#ifndef ALGOIDCOM_BUGGY_DESCRIPTOR_H_
#define ALGOIDCOM_BUGGY_DESCRIPTOR_H_

typedef enum o_type{
	UNKNOWN = -1,
	MOTOR,
        STEPMOTOR,
	PWM,
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
        DIN_2,
        DIN_3,
	NBDIN
}t_din;

typedef enum o_btn{
	BTN_0,
	BTN_1,
	NBBTN
}t_btn;

typedef enum o_colorSens{
	RGBC_SENS_0,
	RGBC_SENS_1,
	NBRGBC
}t_rgbc;

typedef enum o_ain{
	BATT_0,
	NBAIN
}t_ain;

typedef enum o_sonar{
	SONAR_0,
	NBSONAR
}t_sonar;

typedef enum o_motor{
	MOTOR_0,
	MOTOR_1,
	NBMOTOR
}t_motor;

typedef enum o_stepper{
	STEPPER_0,
        NBSTEPPER
}t_stepper;

typedef enum o_servo{
	SERVO_0,
	SERVO_1,
	NBSERVO
}t_servo;

typedef enum o_pwm{
	PWM_0,
	PWM_1,
	PWM_2,
        PWM_3,
        PWM_4,
        PWM_5,
        PWM_6,
        PWM_7,
        PWM_8,
        PWM_9, 
	NBPWM
}t_pwm;

typedef enum o_led{
	LED_0,
	LED_1,
	LED_2,
	NBLED,
}t_led;


// Check TO CONVERT IN INT -1 / 0 / +1   IF NECESSARY !!!!
#define BUGGY_STOP 		0
#define BUGGY_FORWARD           1
#define BUGGY_BACK 		2

#endif /* ALGOIDCOM_BUGGY_DESCRIPTOR_H_ */
