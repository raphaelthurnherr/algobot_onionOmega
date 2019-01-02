#define MAX_MQTT_BUFF 4096

#include "type.h"

// DEFINITION DES TYPES DE MESSAGE
typedef enum msgformat{
	EVENT_ACTION_ERROR,
	EVENT_ACTION_END,
	EVENT_ACTION_BEGIN,
	EVENT_ACTION_ABORT,
        EVENT_ACTION_RUN,
	RESP_STD_MESSAGE,
} t_msgformat;

// DEFINITION DES TYPES DE MESSAGE
typedef enum msgtype{
	ERR_TYPE,
	COMMAND,
	REQUEST,
	ACK,
	RESPONSE,
	EVENT,
        DATAFLOW,
} t_msgtype;

// DEFINITION DES PARAMETRES DE TYPE PARAMETRE
typedef enum msgparam{
        ERR_HEADER,     // Not use
	ERR_PARAM,      // Not use
	STOP,           // Not use
	MOVE,           // Not use
	MOTORS,
	DINPUT,
	DISTANCE,
	BATTERY,
	pPWM,
	pLED,
        pSERVO,
	STATUS,
        BUTTON,
        COLORS,
        CONFIG,
        SYSTEM,
        STEPPER,
}t_msgparam;


struct m2wd{
	int motor;
	int speed;
        int velocity;
	int time;
	int cm;
	int accel;
	int decel;
        char invert[25];
};

struct mStepper{
	int motor;
	int velocity;
	int step;
	int rotation;
        int angle;
        int time;
        char invert[25];
};

struct mDin{
	int id;
	char event_state[25];
};

struct mStream{
	char state[25];
        char onEvent[25];
	int time;
};

struct mAppConf{
	char reset[25];
        char save[25];
};

struct mMotConfig{
        int id;
	char inverted[25];
};

struct mWheelConfig{
	int id;
	int diameter;
	int pulsesPerRot;
};

struct mStepperConfig{
        int id;
	char inverted[25];
        int  ratio;
        int  stepsPerRot;
};

struct mLedConfig{
        int id;
	char state[25];
	int power;
        char isServoMode[25];
};

struct mConfig{
        int motValueCnt;
        int ledValueCnt;
        int stepperValueCnt;
        int wheelValueCnt;
	struct mStream stream;
        struct mAppConf config;
        struct mMotConfig motor[10];
        struct mWheelConfig wheel[10];
        struct mStepperConfig stepper[10];
        struct mLedConfig led[10];
};

struct mDistance{
	int id;
	char event_state[50];
	int event_low;
	int event_high;
	int angle;
};

struct mBattery{
	int id;
	char event_state[50];
	int event_low;
	int event_high;
        int capacity;
};

struct mPwm{
	int id;
	char state[50];
	int angle;
};

struct mLed{
	int id;
	char state[50];
	int powerPercent;
        int time;
        int blinkCount;
        char isServoMode;
};


struct mServo{
	int id;
	int posPercent;
	int state;
};

struct mSystem{
	char name[32];
        int startUpTime;
        char firmwareVersion[32];
        char mcuVersion[32];
        char HWrevision[32];
        float battVoltage;
        int battPercent;
        int wan_online;
        int tx_message;
        int rx_message;   
};

struct mSystemCmd{
        char application[32];
        char firmwareUpdate[32];
        char webAppUpdate[32];
};

struct mColor{
        int value;
	int event_low;
	int event_high;
};

struct mRGB{
	int id;
        char event_state[50];
        struct mColor red;
        struct mColor green;
        struct mColor blue;
        struct mColor clear;
};

// Structure d'un message algoid recu
typedef struct JsonCommand{
	char msgTo[32];
	char msgFrom[32];
	int msgID;
	t_msgtype msgType;
	t_msgparam msgParam;
	unsigned char msgValueCnt;

	// UNION ???
	struct m2wd DCmotor[20];
        struct mStepper StepperMotor[20];
	struct mDin DINsens[20];
        struct mDin BTNsens[20];
	struct mDistance DISTsens[20];
	struct mBattery BATTsens[20];
	struct mLed PWMarray[20];
        struct mLed LEDarray[20]; // LED (internal board pwm)
        struct mRGB RGBsens[20];
        struct mConfig Config;
        struct mSystemCmd System;
	// UNION ???
}ALGOID;

// Structure de r�ponse � un message algoid
typedef struct JsonResponse{
	int value;
	int responseType;

	// UNION ???
        struct mSystem SYSresponse;
	struct mDin DINresponse;
	struct mBattery BATTesponse;
	struct mDistance DISTresponse;
        struct mServo SERVOresponse;
	struct m2wd MOTresponse;
        struct mStepper STEPPERresponse;
        struct mLed PWMresponse;
        struct mLed LEDresponse;
        struct mDin BTNresponse;
        struct mRGB RGBresponse; 
        struct mConfig CONFIGresponse;
        struct mSystemCmd SYSCMDresponse;
	// UNION ???
}ALGOID_RESPONSE;

ALGOID AlgoidCommand;    // Utilis� par main.c
ALGOID AlgoidMessageRX;
ALGOID AlgoidMsgRXStack[10];

// Buffer de sortie pour les msgValue[
ALGOID_RESPONSE AlgoidResponse[20];

extern t_sysConfig sysConfig;

extern char GetAlgoidMsg(ALGOID DestReceiveMessage,char *srcDataBuffer);

void ackToJSON(char * buffer, int msgId, char* to, char * from, char * msgType,char * msgParam, unsigned char value, unsigned char count);
