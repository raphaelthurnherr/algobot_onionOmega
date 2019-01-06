/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   type.h
 * Author: raph-pnp
 *
 * Created on 18. octobre 2018, 12:01
 */

#ifndef TYPE_H
#define TYPE_H

#ifdef __cplusplus
extern "C" {
#endif
#include"../buggy_descriptor.h"
    
struct m_prox{
	int state;
	int event_enable;
};

struct m_dist{
	int value;
	int event_enable;
	int event_low;
	int event_high;
	int event_hysteresis;
};

struct m_voltage{
	int value;
        int capacity;
	int event_enable;
	int event_low;
	int event_high;
	int event_hysteresis;
};

struct m_counter{
	float startEncoderValue;
	float stopEncoderValue;
};

struct m_motor{
	int distance_cm;
        int speed_cmS;
	int velocity;
	int direction;
        int accel;
        int decel;
        int cm;
        int time;
};

struct m_stepper{
    	int speed;
	int direction;
	int step;
        int rotation;
        int angle;
        int time;
};

struct m_led{
	int state;
	int power;
        int blinkCount;
        int blinkTime;
        char isServoMode;
};

struct m_dataStream{
	int state;
        int onEvent;
	int time_ms;
};

struct m_AppConf{
	int reset;
};

struct m_MotConfig{
	char inverted;
        int  minPower;
        int  maxRPM;
};

struct m_StepperConfig{
	char inverted;
        int  ratio;
        int  stepPerRot;
};

struct m_LedConfig{
	int state;
	int power;
        char isServoMode;
};

struct m_WheelConfig{
        int  pulsePerRot;
        int  diameter;
        float _MMPP;
        float _MAXSPEED_CMSEC;
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
        struct m_stepper stepper[NBSTEPPER];
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
        struct m_MotConfig motor[NBMOTOR];
        struct m_WheelConfig wheel[NBMOTOR];
        struct m_LedConfig led[NBLED];
        struct m_StepperConfig stepper[NBSTEPPER];
}t_sysConfig;

#ifdef __cplusplus
}
#endif

#endif /* TYPE_H */

