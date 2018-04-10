// DEFINITION DES TYPES DE MESSAGE
typedef enum msgtype{
	ERR_TYPE,
	COMMAND,
	REQUEST,
	ACK,
	RESPONSE,
	EVENT,
	NEGOC,
	WARNING
} t_msgtype;

// DEFINITION DES PARAMETRES DE TYPE PARAMETRE
typedef enum msgparam{
        ERR_HEADER,
	ERR_PARAM,
	STOP,
	MOVE,
	MOTORS,
	DINPUT,
	DISTANCE,
	BATTERY,
	pPWM,
	pLED,
        pSERVO,
	STATUS,
}t_msgparam;


struct m2wd{
	int motor;
	int velocity;
	int time;
	int cm;
	int accel;
	int decel;
};

struct mDin{
	int id;
	char event_state[25];
	char safetyStop_state[25];
	int safetyStop_value;
};

struct mDistance{
	int id;
	char event_state[50];
	int event_low;
	int event_high;
	int angle;
	char safetyStop_state[25];
	int safetyStop_value;
};

struct mBattery{
	int id;
	char event_state[50];
	int event_low;
	int event_high;
	char safetyStop_state[25];
	int safetyStop_value;
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
};


struct mServo{
	int id;
	int angle;
	int state;
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
        struct mServo SERVOmotor[20];
	struct mDin DINsens[20];
	struct mDistance DISTsens[20];
	struct mBattery BATTsens[20];
	struct mPwm PWMout[20]; // ---> SERVO
	struct mLed PWMarray[20];
        struct mLed LEDarray[20]; // LED (internal board pwm)
	// UNION ???

}ALGOID;

// Structure de r�ponse � un message algoid
typedef struct JsonResponse{
	int value;
	int responseType;

	// UNION ???
	struct mDin DINresponse;
	struct mBattery BATTesponse;
	struct mDistance DISTresponse;
        struct mServo SERVOresponse;
	struct m2wd MOTresponse;
        struct mLed PWMresponse;
	// UNION ???
}ALGOID_RESPONSE;

ALGOID AlgoidCommand;    // Utilis� par main.c
ALGOID AlgoidMessageRX;
ALGOID AlgoidMsgRXStack[10];

// Buffer de sortie pour les msgValue[
ALGOID_RESPONSE AlgoidResponse[20];


extern char GetAlgoidMsg(ALGOID DestReceiveMessage,char *srcDataBuffer);
void ackToJSON(char * buffer, int msgId, char* to, char * from, char * msgType,char * msgParam, unsigned char value, unsigned char count);
