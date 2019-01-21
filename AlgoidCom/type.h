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
};

// -------------------------------------
// DEFINITION DES TYPE DE CAPTEURS
// -------------------------------------
struct s_din{
	int value;
};

struct s_ain{
	int value;
};

struct s_rgb_config{ 
    int rgbID;   
};

struct s_rgbc{
        struct s_color measure;
        struct s_eventAnalog event;
};

struct s_counter{
	int frequency;
        int counter;
};


// -------------------------------------
// DEFINITION DES TYPE DE SORTIE
// -------------------------------------


// --------------------------------------
// MOTEURS
// --------------------------------------
struct s_motor_sp{
    int speed;
    int direction;    
};

struct s_motor_config{
    char inverted;   
    int powerMin;   
};

struct actuator_motor{
    struct s_motor_sp setpoint;
    struct s_motor_config config;
};

// --------------------------------------
// STEP MOTOR
// --------------------------------------

struct s_stepper_sp{
    int speed;
    int direction;    
    int steps;  
};

struct s_stepper_config{
    char inverted;   
    int steps;   
    int ratio;
};

struct actuator_stepper{
    struct s_stepper_sp setpoint;
    struct s_stepper_config config;
};

// --------------------------------------
// DOUT
// --------------------------------------

struct s_dout_sp{
    char enable;
    int  power;  
};

struct s_dout_config{   
    char isServo;   
};

struct actuator_dout{
    struct s_dout_sp setpoint;
    struct s_dout_config config;
};


struct t_actuator{
    struct actuator_motor motor[NBMOTOR];
    struct actuator_stepper stepperMotor[NBSTEPPER];
    struct actuator_dout digitalOutput[NBPWM+NBLED+NBSERVO];
};

struct t_sensor{
	struct s_din din[NBDIN];
        struct s_ain ain[NBAIN];
	struct s_counter counter[NBMOTOR];
	struct s_rgbc rgbc[NBRGBC];

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

struct dc_wheel_data{
    float _MMPP;
    float _MAXSPEED_CMSEC;
    float _STARTENCODERVALUE;
    float _STOPENCODERVALUE;
};

// --------------------------------------
struct stepwheel_settarget{
    int steps;
    int time;
    int distanceCM;
    int angle;
    int rotation;
};

struct dcwheel_settarget{
    int time;
    int distanceCM;
    int angle;
    int rotation;
};

struct dc_wheel_config{
    struct s_pid pidReg;
    struct s_motor_config *motor;
    int motorID;
    int counterID;
    int diameter;           // Config of wheel diameter in mm
    int pulsesPerRot;       // Config number of pulses per rotation of encoder
    int rpmMax;             // Config max supperted RPM of Wheel (motor)
    int rpmMin;             // Config min supported RPM of Wheel (motor)
};

struct stepper_wheel_config{
    struct s_pid pidReg;
    struct s_stepper_config *motor;
    int motorID;
    int diameter;           // Config of wheel diameter in mm
    int rpmMax;             // Config max supperted RPM of Wheel (motor)
    int rpmMin;             // Config min supported RPM of Wheel (motor)
};

typedef struct robotDCWheel{
    struct s_motor_sp *motor;
    struct dcwheel_settarget *target;
    struct dc_wheel_config config;
    struct s_wheel_meas measure;
    struct dc_wheel_data data;
}robot_dcwheel;

typedef struct robotStepperWheel{
    struct stepwheel_settarget *target;
    struct s_stepper_sp *motor;
    struct stepper_wheel_config config;
    struct s_wheel_meas measure;
}robot_stepperwheel;



// AIN & BATTERY  

struct s_son_config{
    int  sonarID;
};

struct s_batt_config{
    int  ainID;
};

struct s_battery_meas{
    int  voltage_mV;
    int  capacity;
};

struct s_sonar_meas{
    int  distance_cm;
};

typedef struct robotBattery{
    struct s_batt_config config;
    struct s_battery_meas measure;
    struct s_eventAnalog event;
    
}robot_battery;

typedef struct robotSonar{
    struct s_son_config config;
    struct s_sonar_meas measure;
    struct s_eventAnalog event;
}robot_sonar;


// BUTTONS

struct s_button_config{
    int  dinID;
};

struct s_button_meas{
    int  state;
};

struct s_rgb_meas{
    struct s_rgbc red;
    struct s_rgbc green;
    struct s_rgbc blue;
    struct s_rgbc clear;
};

struct s_prox_meas{
    int  state;
};

struct s_prox_config{
    int  dinID;
};

typedef struct robotButton{
    struct s_button_config config;
    struct s_button_meas measure;
    struct s_eventBool event;
    
}robot_button;

typedef struct robotProx{
    struct s_prox_config config;
    struct s_prox_meas measure;
    struct s_eventBool event;
    
}robot_prox;

// COmmunication & MQTT 

struct mqtt_stream{
    int  state;
    int  time_ms;
    int  onEvent;
    int  topic;
};

struct s_mqtt{
    struct mqtt_stream stream;
};

struct s_udp_bc{
    int  state;
    int  time_ms;
};


struct s_udp{
    struct s_udp_bc broadcast;
};
struct app_comm{
    struct s_mqtt mqtt;
    struct s_udp udp;
};


// DOUT, PWM and LEDS

struct s_pwm_action{
    int  blinkCount;
    int  blinkTime;
};

struct s_led_config{
    int  doutID;
    int  defaultPower;
    int  defaultState;
    int  defaultmode;
};


typedef struct robotLed{
    struct s_dout_sp *led;
    struct s_led_config config;
    struct s_pwm_action action;
}robot_led;

typedef struct robotColor{
    struct s_eventBool event;
    struct s_rgb_meas color;
    struct s_rgb_config config;
}robot_color;




typedef struct robotKehops{
    robot_battery battery[2];
    robot_button button[NBBTN];
    robot_stepperwheel stepperWheel[NBSTEPPER];
    robot_dcwheel dcWheel[NBMOTOR];
    robot_led led[NBLED];
    robot_led pwm[NBPWM];
    robot_led servo[NBSERVO];
    robot_prox proximity[NBDIN];
    robot_sonar sonar[NBSONAR];
    robot_color rgb[NBRGBC];
}robot_kehops;


struct tInfosys{
	char name[32];
        long startUpTime;
        char firmwareVersion[32];
        char mcuVersion[32];
        char HWrevision[32];
        float battVoltage;
        int wan_online;
};

struct app_kehops{
    char resetConfig;
};

struct app_device{
    struct s_motor_config *motor[NBMOTOR];
    struct s_stepper_config *stepper[NBSTEPPER];
};

typedef struct systemApp{
    struct app_comm communication;
    struct app_kehops kehops;
    struct app_device device;
    struct tInfosys info;
}t_sysApp;

#ifdef __cplusplus
}
#endif

#endif /* TYPE_H */

