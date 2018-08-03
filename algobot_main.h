/*
 * algoid_2wd_buggy.h
 *
 *  Created on: 8 avr. 2016
 *      Author: raph
 */

#ifndef ALGOID_2WD_BUGGY_H_
#define ALGOID_2WD_BUGGY_H_


//
#define LED_OFF           0
#define LED_ON            1
#define LED_BLINK         2

#define INFINITE          2
#define ON                1
#define OFF               0

#define MILLISECOND       0
#define CENTIMETER	  1


#define CMPP		  0.248             // For 57 pulses per rotation and wheel diameter 45mm


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
        int capacity;
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

struct m_dataStream{
	int state;
        int onEvent;
	int time_ms;
};

struct m_AppConf{
	int reset;
};

struct m_color{
        int value;
	int event_low;
	int event_high;
        int event_hysteresis;
};

struct m_RGB{
	int id;
        int event_enable;
        struct m_color red;
        struct m_color green;
        struct m_color blue;
        struct m_color clear;
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
        struct m_RGB rgb[NBRGBC];
}t_sensor;


typedef struct tSystem{
	char name[32];
        long startUpTime;
        char firmwareVersion[32];
        char mcuVersion[32];
        char HWrevision[32];
        float battVoltage;
        int wan_online;
}t_system;

typedef struct tConfig{
	struct m_dataStream dataStream;   
        struct m_AppConf config;  
}t_sysConfig;

extern t_sensor body;
extern t_system sysInfo;
extern t_sysConfig sysConfig;

#endif /* ALGOID_2WD_BUGGY_H_ */
