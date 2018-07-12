/*
 * hwMagager.c
 *
 *  Created on: 26 nov. 2016
 *      Author: raph
 */

#define  CALLBACK 0
#define  ADR	  1
#define	 CMD      2

#include "pthread.h"
#include <unistd.h>
#include <stdio.h>
#include "hwManager.h"
#include "boardHWctrl.h"
#include "../buggy_descriptor.h"

// Thread Messager
pthread_t th_hwManager;

struct s_encoder{
	int pulseFromStartup;
	int frequency;
};

struct s_colorSensor{
	int red;
	int green;
        int blue;
        int clear;
};

typedef struct tmeasures{
	unsigned char din[NBDIN];
	int ain[NBAIN];
	struct s_encoder counter[NBCOUNTER];
	int pwm[NBPWM];
        unsigned char btn[NBBTN];
        struct s_colorSensor RGBC[NBRGBC];
}t_measure;

typedef struct tHWversion{
        int mcuVersion;
        int HWrevision;
}t_HWversion;


t_measure sensor;
t_HWversion BoardInfo;

int i2c_command_queuing[50][3];

int timeCount_ms=0;
unsigned char motorDCactualPower[2];				// Valeur de la puissance moteur
unsigned char motorDCtargetPower[2]; 				// Valuer de consigne pour la puissance moteur
unsigned char motorDCaccelValue[2]={25,25};			// Valeur d'acceleration des moteurs
unsigned char motorDCdecelValue[2]={25,25};			// Valeur d'acceleration des moteurs


int getMotorFrequency(unsigned char motorNb);	// Retourne la fréquence actuelle mesuree sur l'encodeur
int getMotorPulses(unsigned char motorName);		// Retourne le nombre d'impulsion d'encodeur moteur depuis le démarrage
char getDigitalInput(unsigned char inputNumber);	// Retourne l'état de l'entrée numérique spécifiée
char getButtonInput(unsigned char buttonNumber);        // Retourne l'état du bouton
int getSonarDistance(void);						// Retourne la distance en cm
int getBatteryVoltage(void);					// Retourne la tension battery en mV
int getColorValue(unsigned char sensorID, unsigned char color);      // Retourne la valeur de la couleur définie sur le capteur défini
//char getOrganNumber(int organName);		// Retourne le numéro du moteur 0..xx selon le nom d'organe spécifié
unsigned char getOrganI2Cregister(char organType, unsigned char organName); // Retourne l'adresse du registre correspondant au nom de l'organe

extern int setMotorSpeed(int motorName, int ratio);
void setMotorAccelDecel(unsigned char motorNo, char accelPercent, char decelPercent);		// Défini l'accéleration/deceleration d'un moteur
int setMotorDirection(int motorName, int direction);
void checkDCmotorPower(void);				// Fonction temporaire pour rampe d'acceleration
unsigned char getMotorPower(unsigned char motorNr);											// Retourne la velocité actuelle d'un moteur

void setServoPosition(unsigned char smName, unsigned char angle);
void setLedPower(unsigned char ledID, unsigned char power);
void setPwmPower(unsigned char ID, unsigned char power);

void processCommandQueue(void);
void execCommand(void (*ptrFunc)(char, int), char adr, int cmd);
int set_i2c_command_queue(int (*callback)(char, int),char adr, int cmd);		//

int getHWversion(void);                                                 // Get the hardware
int getMcuFirmware(void);                                              // Get the hardware
// ------------------------------------------
// Programme principale TIMER
// ------------------------------------------
void *hwTask (void * arg){
	int i;
        char dinState=0;

	if(buggyBoardInit()){
                
		printf("\n#[HW MANAGER] Initialisation carte HW: OK\n");
		sendMqttReport(0,"#[HW MANAGER] Initialisation carte HW: OK\n");
	}
	else{
		printf("#[HW MANAGER] Initialisation carte HW: ERREUR \n");
		sendMqttReport(0,"#[HW MANAGER] Initialisation carte HW: ERREUR\n");
	}

	// Reset la distance de la carte EFM8BB
	EFM8BB_clearWheelDistance(MOTOR_ENCODER_LEFT);
	EFM8BB_clearWheelDistance(MOTOR_ENCODER_RIGHT);
        
        BoardInfo.mcuVersion=EFM8BB_getFirmwareVersion();      
        BoardInfo.HWrevision=EFM8BB_getBoardType();
        
        
        
	while(1){
		// Sequencage des messages sur bus I2C à interval régulier
		switch(timeCount_ms){
			case 5	: sensor.counter[MOTOR_ENCODER_LEFT].pulseFromStartup = EFM8BB_readPulseCounter(MOTOR_ENCODER_LEFT);
					  sensor.counter[MOTOR_ENCODER_LEFT].frequency = EFM8BB_readFrequency(MOTOR_ENCODER_LEFT); break;
			case 10	: sensor.counter[MOTOR_ENCODER_RIGHT].pulseFromStartup = EFM8BB_readPulseCounter(MOTOR_ENCODER_RIGHT);
					  sensor.counter[MOTOR_ENCODER_RIGHT].frequency = EFM8BB_readFrequency(MOTOR_ENCODER_RIGHT); break;
			case 15	:   dinState = EFM8BB_readDigitalInput(0);              // Paramètre transmis non utilisé par la fonction...
                                    if(dinState & 0x01) sensor.din[DIN_0] = 1;
                                    else sensor.din[DIN_0]=0;
                        
                                    if(dinState & 0x02) sensor.din[DIN_1] = 1;
                                    else sensor.din[DIN_1]=0;
                        
                                    if(dinState & 0x04) sensor.din[DIN_2] = 1;
                                    else sensor.din[DIN_2]=0;
                        
                                    if(dinState & 0x08) sensor.din[DIN_3] = 1;
                                    else sensor.din[DIN_3]=0;
                                    break;
			case 20	: sensor.pwm[SONAR_0] = EFM8BB_readSonarDistance()/10; break;       // Conversion de distance mm en cm
			case 25	: sensor.ain[BATT_0] = EFM8BB_readBatteryVoltage(); break;
                        
                        case 30	: sensor.btn[BTN_0] = MCP2308_ReadGPIO(BTN_0) ;
                                  sensor.btn[BTN_1] = MCP2308_ReadGPIO(BTN_1) ; break;

                        case 35	: sensor.RGBC[RGBC_SENS_0].red = BH1745_getRGBvalue(RGBC_SENS_0, RED) ;
                                  sensor.RGBC[RGBC_SENS_0].green = BH1745_getRGBvalue(RGBC_SENS_0, GREEN) ;
                                  sensor.RGBC[RGBC_SENS_0].blue = BH1745_getRGBvalue(RGBC_SENS_0,BLUE) ;
                                  sensor.RGBC[RGBC_SENS_0].clear = BH1745_getRGBvalue(RGBC_SENS_0,CLEAR) ; break;
 
                        case 36	: sensor.RGBC[RGBC_SENS_1].red = BH1745_getRGBvalue(RGBC_SENS_1, RED) ;
                        sensor.RGBC[RGBC_SENS_1].green = BH1745_getRGBvalue(RGBC_SENS_1, GREEN) ;
                        sensor.RGBC[RGBC_SENS_1].blue = BH1745_getRGBvalue(RGBC_SENS_1, BLUE) ;
                        sensor.RGBC[RGBC_SENS_1].clear = BH1745_getRGBvalue(RGBC_SENS_1, CLEAR) ; break;
                        
			default:
                            if(i2c_command_queuing[0][CALLBACK]!=0)processCommandQueue(); break;
		}
                
//                printf("\n LEFT: %d   RIGHT: %d", sensor.counter[MOTOR_ENCODER_LEFT].pulseFromStartup, sensor.counter[MOTOR_ENCODER_RIGHT].pulseFromStartup);


		// Reset le compteur au bout de 50mS
		if(timeCount_ms<50)
			timeCount_ms++;
		else timeCount_ms=0;

		usleep(1000);
	}
	pthread_exit (0);
}

// ------------------------------------------------------------------------------------
// TIMERMANAGER: Initialisation du gestionnaire de timer
// - Démarre le thread
// ------------------------------------------------------------------------------------
int InitHwManager(void){
	// CREATION DU THREAD DE TIMER
	  if (pthread_create (&th_hwManager, NULL, hwTask, NULL)!= 0) {
		return (1);
	  }else return (0);
}

// ------------------------------------------------------------------------------------
// CLOSEHWMANAGER: Fermeture du gestionnaire hardware
// - Stop le thread hardware
// ------------------------------------------------------------------------------------

int CloseHwManager(void){
	int result;
	// TERMINE LE THREAD DE MESSAGERIE
	pthread_cancel(th_hwManager);
	// Attends la terminaison du thread de messagerie
	result=pthread_join(th_hwManager, NULL);
	return (result);
}

// ------------------------------------------------------------------------------------
// GETPULSEMOTOR: lecture de l'encodeur optique du moteur spécifié
// Entrée: Numéro de l'encodeur
// Sortie:
// ------------------------------------------------------------------------------------
int getMotorPulses(unsigned char motorName){
	int pulses;

	pulses = sensor.counter[motorName].pulseFromStartup;
	/*
	switch(motorName){
		case MOTOR_0: pulses = buggySensor.left_encoder.pulseFromStartup; break;
		case MOTOR_1: pulses = buggySensor.right_encoder.pulseFromStartup; break;
		default: pulses= -1; break;
	}
	*/
	return pulses;
}

char getDigitalInput(unsigned char inputNumber){
	char inputState=0;

	inputState = sensor.din[inputNumber];
//        printf("DIG %d: %d\n",inputNumber, inputState);
	return inputState;
}

char getButtonInput(unsigned char buttonNumber){
	char inputState=0;

	inputState = sensor.btn[buttonNumber];
	return inputState;
}

int getMotorFrequency(unsigned char motorNb){
	char freq;

	freq = sensor.counter[motorNb].frequency;
	/*
	switch(motorNb){
		case 0: freq = buggySensor.left_encoder.frequency; break;
		case 1: freq = buggySensor.right_encoder.frequency; break;
		default: freq=-1; break;
	}
	*/

	return freq;
	return 0;
}


int getSonarDistance(void){
	int sonarCm=0;

	sonarCm = sensor.pwm[SONAR_0];
	//sonarCm= buggySensor.usonic;
	return sonarCm;
}

int getBatteryVoltage(void){
	int voltage=0;
	voltage = sensor.ain[BATT_0];
	//voltage = buggySensor.battery;
	return voltage;
}

int getColorValue(unsigned char sensorID, unsigned char color){
    int colorValue;
    
    switch(color){
        case RED : colorValue= sensor.RGBC[sensorID].red; break;
        case GREEN : colorValue=sensor.RGBC[sensorID].green; break;
        case BLUE : colorValue=sensor.RGBC[sensorID].blue; break;
        case CLEAR : colorValue=sensor.RGBC[sensorID].clear; break;
        default : colorValue = -1; break;
    }
/*    
    printf("RGBC# %d Values: RED: %d, GREEN: %d, BLUE: %d, CLEAR: %d VALUE: %d\n", sensorID,
                        sensor.RGBC[RGBC_SENS_0].red,
                        sensor.RGBC[RGBC_SENS_0].green,
                        sensor.RGBC[RGBC_SENS_0].blue,
                        sensor.RGBC[RGBC_SENS_0].clear,colorValue);
  */      
	return colorValue;
}

// ---------------------------------------------------------------------------
// SETMOTORDIRECTION
// !!!!!!!!!!!!! FONCTION A RETRAVAILLER !!!!!!!!!!!!!!!!!!!
// ---------------------------------------------------------------------------
int setMotorDirection(int motorName, int direction){
	unsigned char motorAdress;

	// Conversion No de moteur en adresse du registre du PWM controleur
	motorAdress=getOrganI2Cregister(MOTOR, motorName);

	switch(direction){
		case BUGGY_FORWARD :	set_i2c_command_queue(&MCP2308_DCmotorSetRotation, motorAdress, MCW); break;
		case BUGGY_BACK :       set_i2c_command_queue(&MCP2308_DCmotorSetRotation, motorAdress, MCCW); break;

		case BUGGY_STOP : 		break;
		default :		     	break;
	}
	return(1);
}

// ---------------------------------------------------------------------------
// GETMCUHWVERSION
// Get the hardware/software version
// ---------------------------------------------------------------------------
int getMcuHWversion(void){
	return(BoardInfo.HWrevision);
}

// ---------------------------------------------------------------------------
// GETMCUFIRMWARE
// Get the hardware/software version
// ---------------------------------------------------------------------------
int getMcuFirmware(void){
	return(BoardInfo.mcuVersion);
}


// ------------------------------------------------------------------------------------
// CHECKMOTORPOWER:
// Fonction appelée periodiquement pour la gestion de l'acceleration
// Décelération du moteur.
// Elle va augmenter ou diminuer la velocite du moteur jusqu'a atteindre la consigne
// ------------------------------------------------------------------------------------
void checkDCmotorPower(void){
	unsigned char i;
	//unsigned char PowerToSet;

	// Contrôle successivement la puissance sur chaque moteur et effectue une rampe d'accélération ou décéleration
	for(i=0;i<2;i++){
		//printf("Motor Nb: %d Adr: %2x ActualPower: %d   TargetPower: %d  \n",i, motorDCadr[i], motorDCactualPower[i], motorDCtargetPower[i]);
		if(motorDCactualPower[i] < motorDCtargetPower[i]){
			//PowerToSet=motorDCactualPower[i] + ((motorDCtargetPower[i]-motorDCactualPower[i])/100)*motorDCaccelValue[i];
			//printf("Power to set: %d %",PowerToSet);

			if(motorDCactualPower[i]+motorDCaccelValue[i]<=motorDCtargetPower[i])		// Contrôle que puissance après acceleration ne dépasse pas la consigne
				motorDCactualPower[i]+=motorDCaccelValue[i];						// Augmente la puissance moteur
			else motorDCactualPower[i]=motorDCtargetPower[i];						// Attribue la puissance de consigne

			set_i2c_command_queue(&PCA9685_DCmotorSetSpeed, motorDCadr[i], motorDCactualPower[i]);
			//PCA9685_DCmotorSetSpeed(motorDCadr[i], motorDCactualPower[i]);
		}

		if(motorDCactualPower[i]>motorDCtargetPower[i]){
			if(motorDCactualPower[i]-motorDCdecelValue[i]>=motorDCtargetPower[i])		// Contrôle que puissance après acceleration ne dépasse pas la consigne
				motorDCactualPower[i]-=motorDCdecelValue[i];						// Diminue la puissance moteur
			else motorDCactualPower[i]=motorDCtargetPower[i];						// Attribue la puissance de consigne


			set_i2c_command_queue(&PCA9685_DCmotorSetSpeed, motorDCadr[i], motorDCactualPower[i]);
			//PCA9685_DCmotorSetSpeed(motorDCadr[i], motorDCactualPower[i]);

			// Ouvre le pont en h de commande moteur
			if(motorDCactualPower[i]==0)
				setMotorDirection(i,BUGGY_STOP);
		}
	}
}

// -------------------------------------------------------------------
// GETMOTORPOWER
// Retourne l'état actuelle de la puissance du moteur selectionné
// -------------------------------------------------------------------

unsigned char getMotorPower(unsigned char motorNr){
	return motorDCactualPower[motorNr];
}


// -------------------------------------------------------------------
// setMotorAccelDecel
// Défini les valeurs d'acceleration et decelaration du moteur
// Valeur donnée en % de ce qu'il reste pour atteindre la consigne
// -------------------------------------------------------------------
void setMotorAccelDecel(unsigned char motorNo, char accelPercent, char decelPercent){

	//unsigned char motorSlot;
	//motorSlot = getOrganNumber(motorNo);

	// Récupération de la valeur absolue de l'acceleration
	if(accelPercent<0) accelPercent*=-1;
	// Défini un maximum de 100% d'acceleration
	if(accelPercent>100)
		accelPercent=100;

	// Récupération de la valeur absolue de la deceleration
	if(decelPercent<0) decelPercent*=-1;
	// Défini un maximum de 100% de deceleration
	if(decelPercent>100)
		decelPercent=100;

	// Ne modifie les valeurs d'acceleration et deceleration uniquement si "valable" (=>0)
	if(accelPercent>0)
		motorDCaccelValue[motorNo] = accelPercent;
	if(decelPercent>0)
		motorDCdecelValue[motorNo] = decelPercent;
}


// ---------------------------------------------------------------------------
// SETMOTORSPEED
// Applique la consigne de vélocité pour un moteur donné
// Cette consigne n'est pas appliquée directement sur les moteur mais sera progressivement
// approchée par le gestionnaire d'acceleration.
// ---------------------------------------------------------------------------
int setMotorSpeed(int motorName, int ratio){
	//char motorSlot;
	//motorSlot = getOrganNumber(motorName);

	// Vérification ratio max et min comprise entre 0..100%
	if(ratio > 100)
		ratio = 100;
	if (ratio<0)
		ratio = 0;

	if(motorName >= 0)
		motorDCtargetPower[motorName]=ratio;
	else
		printf("\n function [setMotorSpeed] : undefine motor #%d", motorName);

	return(1);
}


// ------------------------------------------------------------------------------------
// SET_I2C_COMMAND_QUEUE: Mise en file d'attente de l'appelle d'une fonction I2C
// ------------------------------------------------------------------------------------
int set_i2c_command_queue(int (*callback)(char, int),char adr, int cmd){
	unsigned char freeIndex, i;

	// Recherche d'un emplacement libre dans la file d'attente
	for(freeIndex=0;(freeIndex<50) && (i2c_command_queuing[freeIndex][CALLBACK]>0);freeIndex++);

	if(freeIndex>=49) printf("\n[hwManager]->File de commande pleine !\n");
	else
	{
		i2c_command_queuing[freeIndex][CALLBACK] =  callback;
		i2c_command_queuing[freeIndex][ADR] =  adr;
		i2c_command_queuing[freeIndex][CMD] =  cmd;
	}


	return freeIndex;
}


void setServoPosition(unsigned char smName, unsigned char angle){
	char smAddr;

	smAddr=getOrganI2Cregister(PWM, smName);
	set_i2c_command_queue(&PCA9685_setServoPos, smAddr, angle);
}

void setLedPower(unsigned char ledID, unsigned char power){
	unsigned char ledAdress;
	ledAdress=getOrganI2Cregister(LED, ledID);
	set_i2c_command_queue(&PCA9685_setLedPower, ledAdress, power);
}

void setPwmPower(unsigned char ID, unsigned char power){
	unsigned char pwmAdress;
	pwmAdress=getOrganI2Cregister(PWM, ID);
	set_i2c_command_queue(&PCA9685_setLedPower, pwmAdress, power);
}

// ------------------------------------------------------------------------------------
// ONTIMEOUT: Fcontion appelee en fin de timer
// appelle une fonction callback prédéfinie par *ptrFunc
// ------------------------------------------------------------------------------------
void execCommand(void (*ptrFunc)(char, int), char adr, int cmd){
	(*ptrFunc)(adr, cmd);		// Appelle de la fonction call back prédéfinie par *ptrFonc avec les paramètre recus
}


void processCommandQueue(void){
	int i;
	// ENVOIE DE LA COMMANDE I2C

	//printf("#%d  callback: %d, adr: %d cmd: %d\n",0 ,i2c_command_queuing[0][CALLBACK],i2c_command_queuing[0][ADR],i2c_command_queuing[0][CMD]);
	execCommand(i2c_command_queuing[0][CALLBACK], i2c_command_queuing[0][ADR], i2c_command_queuing[0][CMD]);
	// DECALAGE DE LA PILE

	for(i=0;i<50;i++){
		i2c_command_queuing[i][CALLBACK] = i2c_command_queuing[i+1][CALLBACK];
		i2c_command_queuing[i][ADR] = i2c_command_queuing[i+1][ADR];
		i2c_command_queuing[i][CMD] = i2c_command_queuing[i+1][CMD];
	}
	i2c_command_queuing[49][CALLBACK]=i2c_command_queuing[49][ADR]=i2c_command_queuing[49][CMD]=0;
}


// ------------------------------------------------------------------------------------
// getOrganAdress: Conversion du non d'organe en adresse I2C
// Retourne l'adresse du registre correspondant au nom de l'organe
// ------------------------------------------------------------------------------------
unsigned char getOrganI2Cregister(char organType, unsigned char organName){
	unsigned char organAdr;

	if(organType == MOTOR){
		switch(organName){
			case MOTOR_0 : organAdr = PCA_DCM0; break;
			case MOTOR_1 : organAdr = PCA_DCM1; break;
			default :	organAdr = UNKNOWN; break;
		}
	}

	if(organType == PWM){
		switch(organName){
			case PWM_0 : organAdr = PCA_CN1_4; break;
			case PWM_1 : organAdr = PCA_CN2_4; break;
			case PWM_2 : organAdr = PCA_CN3_7; break;
                        case PWM_3 : organAdr = PCA_CN4_4; break;
                        case PWM_4 : organAdr = PCA_CN7_3; break;
                        case PWM_5 : organAdr = PCA_CN8_3; break;
                        case PWM_6 : organAdr = PCA_CN9_7; break;
                        case PWM_7 : organAdr = PCA_CN10_4; break;
                        case PWM_8 : organAdr = PCA_CN11_3; break;
                        case PWM_9 : organAdr = PCA_CN12_3; break;
                        
			default :	organAdr = UNKNOWN; break;
		}
	}

	if(organType == LED){
		switch(organName){
			case LED_0 : organAdr = PCA_LED0; break;
			case LED_1 : organAdr = PCA_LED1; break;
			case LED_2 : organAdr = PCA_LED2; break;
			default :	organAdr = UNKNOWN; break;
		}
	}

	return organAdr;
}
