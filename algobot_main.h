/*
 * algoid_2wd_buggy.h
 *
 *  Created on: 8 avr. 2016
 *      Author: raph
 */

#ifndef ALGOID_2WD_BUGGY_H_
#define ALGOID_2WD_BUGGY_H_

#define MILLISECOND       0
#define CENTIMETER	  1
#define CMPP		  0.723


struct m_prox{
	int state;
	int event_enable;
	int safetyStop_state;
	int safetyStop_value;
	int em_stop;
};

struct m_dist{
	int value;
	int event_enable;
	int event_low;
	int event_high;
	int event_hysteresis;
	int safetyStop_state;
	int safetyStop_value;
};

struct m_voltage{
	int value;
	int event_enable;
	int event_low;
	int event_high;
	int event_hysteresis;
	int safetyStop_state;
	int safetyStop_value;
};

struct m_counter{
	float startEncoderValue;
	float stopEncoderValue;
};

struct m_motor{
	int distance;
	int speed;
	int direction;
        char accel;
        char decel;
        int cm;
        int time;
};

struct m_led{
	int state;
	int power;
        int blinkCount;
        int blinkTime;
};

typedef struct tsensor{
	struct m_prox proximity[NBDIN];
        struct m_prox button[NBBTN];
	struct m_dist distance[NBPWM];
	struct m_voltage battery[NBAIN];
	struct m_counter encoder[NBMOTOR];
	struct m_motor motor[NBMOTOR];
        struct m_led led[NBLED];
        struct m_led pwm[NBPWM];
}t_sensor;


typedef struct tSystem{
	char name[32];
        int startUpTime;
        char firmwareVersion[32];
        char mcuVersion[32];
        char HWrevision[32];
        float battVoltage;
}t_system;

extern t_sensor body;
extern t_system sysInfo;

#endif /* ALGOID_2WD_BUGGY_H_ */
