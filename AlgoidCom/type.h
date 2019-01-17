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
        float speed_rpm;
	int velocity;
	int direction;
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

struct m_PIDConfig{
        char PIDstate;
        float PID_Ki;
        float PID_Kp;
        float PID_Kd;
};

struct m_MotConfig{
	char inverted;
        int  minPWM;
        int  minRPM;
        int  maxRPM;
        struct m_PIDConfig rpmRegulator;
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

typedef struct tmap{
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
}t_robotMap;


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


// ICI LES NOUVEAUX TYPE DE VARIABLE

// -----------------------------
// COMMUNES
// -----------------------------
struct s_eventAnalog{
	int enable;
	int low;
	int high;
	int hysteresis;
};

struct s_eventBool{
	int enable;
};

struct s_color{
	int value;
        struct s_eventAnalog event;
};

// -------------------------------------
// DEFINITION DES TYPE DE CAPTEURS
// -------------------------------------
struct s_din{
	int value;
        struct s_eventBool event;
};

struct s_ain{
	int value;
        struct s_eventAnalog event;
};

struct s_rgbc{
        struct s_color red;
        struct s_color green;
        struct s_color blue;
        struct s_color clear;
};

struct s_counter{
	int frequency;
        int counter;
};


// -------------------------------------
// DEFINITION DES TYPE DE SORTIE
// -------------------------------------
/*
struct s_stepperAction{
	int step;
        int rotation;
        int angle;
        int time;
};
*/


struct s_motor_sp{
    int speed;
    int direction;    
};

struct s_stepper_sp{
    int speed;
    int direction;    
    int steps;  
};


struct s_motor_config{
    char inverted;   
    char powerMin;   
};

struct actuator_motor{
    struct s_motor_sp setpoint;
    struct s_motor_config config;
};

struct a_stepper{
    int speed;
    int direction;
    int steps;
    
};
// 
// --------------------------------------
struct t_sensor{
	struct s_din din[NBDIN];
        struct s_ain ain[NBAIN];
	struct s_counter counter[NBMOTOR];
	struct s_rgbc rgbc[NBRGBC];

};

struct t_actuator{
    struct actuator_motor motor[NBMOTOR];
    int stepper;
};

// --------------------------------------
// HIGH LEVEL
// --------------------------------------

typedef struct t_device{
    struct t_sensor sensor;
    struct t_actuator actuator;
}t_device;

// --------------------------------------
// HIGH LEVEL
// --------------------------------------

struct s_pid{
    char  enable;
    float Kp;
    float Ki;
    float Kd;
};

struct s_wheel_meas{
    int  distance;
    int  rpm;
    int  speed_cmS;
    int  speed_percent;
};

// --------------------------------------
struct wheel_settarget{
    int time;
    int distanceCM;
    int angle;
    int rotation;
};

struct dc_wheel_config{
    struct s_pid pidReg;
    int motorID;
    int counterID;
    int diameter;           // Config of wheel diameter in mm
    int pulsesPerRot;       // Config number of pulses per rotation of encoder
    int rpmMax;             // Config max supperted RPM of Wheel (motor)
    int rpmMin;             // Config min supported RPM of Wheel (motor)
};

struct stepper_wheel_config{
    struct s_pid pidReg;
    int motorID;
    int diameter;           // Config of wheel diameter in mm
    int rpmMax;             // Config max supperted RPM of Wheel (motor)
    int rpmMin;             // Config min supported RPM of Wheel (motor)
};

typedef struct robotDCWheel{
    struct s_motor_sp *motor;
    struct wheel_settarget *target;
    struct dc_wheel_config config;
    struct s_wheel_meas measure;
}robot_dcwheel;

typedef struct robotStepperWheel{
    struct wheel_settarget *target;
    struct s_stepper_sp *motor;
    struct stepper_wheel_config config;
    struct s_wheel_meas measure;
}robot_stepperwheel;


// AIN & BATTERY  

struct s_ain_config{
    int  ainID;
};

struct s_battery_meas{
    int  voltage_mV;
    int  capacity;
};

typedef struct robotBattery{
    struct s_ain_config config;
    struct s_battery_meas measure;
    struct s_eventAnalog event;
    
}robot_battery;


// BUTTONS

struct s_din_config{
    int  dinID;
};

struct s_button_meas{
    int  state;
};


typedef struct robotButton{
    struct s_din_config config;
    struct s_button_meas measure;
    struct s_eventBool event;
    
}robot_button;


// COmmunication & MQTT 

struct mqtt_stream{
    int  state;
    int  time_ms;
    int  onEvent;
    int  topic;
};

struct s_mqtt{
    struct mqtt_stream streamm;
};

struct s_udp_bc{
    int  state;
    int  time_ms;
};


struct s_udp{
    struct s_udp_bc broadcast;
};
typedef struct s_communication{
    struct s_mqtt mqtt;
    struct s_udp udp;
}communication;




typedef struct robotKehops{
    robot_battery battery[2];
    robot_button button[NBBTN];
    robot_stepperwheel stepperWheel[NBSTEPPER];
    robot_dcwheel dcWheel[NBMOTOR];
    
}robot_kehops;

#ifdef __cplusplus
}
#endif

#endif /* TYPE_H */

