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
#include "hwManager.h"
#include "boardHWctrl.h"
#include "../buggy_descriptor.h"

// Thread Messager
pthread_t th_hwManager;

struct s_encoder{
	int pulseFromStartup;
	int frequency;
};

typedef struct tmeasures{
	unsigned char din[NBDIN];
	int ain[NBAIN];
	struct s_encoder counter[NBCOUNTER];
	int pwm[NBPWM];
}t_measure;

t_measure sensor;

int i2c_command_queuing[50][3];

int timeCount_ms=0;
unsigned char motorDCactualPower[2];				// Valeur de la puissance moteur
unsigned char motorDCtargetPower[2]; 				// Valuer de consigne pour la puissance moteur
unsigned char motorDCaccelValue[2]={25,25};			// Valeur d'acceleration des moteurs
unsigned char motorDCdecelValue[2]={25,25};			// Valeur d'acceleration des moteurs


int getMotorFrequency(unsigned char motorNb);	// Retourne la fréquence actuelle mesuree sur l'encodeur
int getMotorPulses(unsigned char motorName);		// Retourne le nombre d'impulsion d'encodeur moteur depuis le démarrage
char getDigitalInput(unsigned char inputNumber);	// Retourne l'état de l'entrée numérique spécifiée
int getSonarDistance(void);						// Retourne la distance en cm
int getBatteryVoltage(void);					// Retourne la tension battery en mV

//char getOrganNumber(int organName);		// Retourne le numéro du moteur 0..xx selon le nom d'organe spécifié
unsigned char getOrganI2Cregister(char organType, unsigned char organName); // Retourne l'adresse du registre correspondant au nom de l'organe

extern int setMotorSpeed(int motorName, int ratio);
void setMotorAccelDecel(unsigned char motorNo, char accelPercent, char decelPercent);		// Défini l'accéleration/deceleration d'un moteur
int setMotorDirection(int motorName, int direction);
void checkDCmotorPower(void);				// Fonction temporaire pour rampe d'acceleration
unsigned char getMotorPower(unsigned char motorNr);											// Retourne la velocité actuelle d'un moteur

void setServoPosition(unsigned char smName, unsigned char angle);
void setLedPower(unsigned char ledID, unsigned char power);

void processCommandQueue(void);
void execCommand(void (*ptrFunc)(char, int), char adr, int cmd);
int set_i2c_command_queue(int (*callback)(char, int),char adr, int cmd);		//
// ------------------------------------------
// Programme principale TIMER
// ------------------------------------------
void *hwTask (void * arg){
	int i;

	if(buggyBoardInit()){
		printf("\n#[HW MANAGER] Initialisation carte HW: OK\n");
		sendMqttReport(0,"#[HW MANAGER] Initialisation carte HW: OK\n");
		setLedPower(LED_0,5);
                setLedPower(LED_1,40);
                setLedPower(LED_2,100);
	}
	else{
		printf("#[HW MANAGER] Initialisation carte HW: ERREUR \n");
		sendMqttReport(0,"#[HW MANAGER] Initialisation carte HW: ERREUR\n");
	}

	// Reset la distance de la carte EFM8BB
	EFM8BB_clearWheelDistance(MOTOR_ENCODER_LEFT);
	EFM8BB_clearWheelDistance(MOTOR_ENCODER_RIGHT);

	while(1){
		// Sequencage des messages sur bus I2C à interval régulier
		switch(timeCount_ms){
			case 5	: sensor.counter[MOTOR_ENCODER_LEFT].pulseFromStartup = EFM8BB_readPulseCounter(MOTOR_ENCODER_LEFT);
					  sensor.counter[MOTOR_ENCODER_LEFT].frequency = EFM8BB_readFrequency(MOTOR_ENCODER_LEFT); break;
			case 10	: sensor.counter[MOTOR_ENCODER_RIGHT].pulseFromStartup = EFM8BB_readPulseCounter(MOTOR_ENCODER_RIGHT);
					  sensor.counter[MOTOR_ENCODER_RIGHT].frequency = EFM8BB_readFrequency(MOTOR_ENCODER_RIGHT); break;
			case 15	: sensor.din[DIN_0] = EFM8BB_readDigitalInput(DIN_0); break;
			case 20	: sensor.din[DIN_1] = EFM8BB_readDigitalInput(DIN_1); break;
			case 25	: sensor.pwm[SONAR_0] = EFM8BB_readSonarDistance()/10; break;
			case 30	: sensor.ain[BATT_0] = EFM8BB_readBatteryVoltage() ;break;

			default: if(i2c_command_queuing[0][CALLBACK]!=0)processCommandQueue(); break;
		}

		// Reset le compteur au bout de 100mS
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
		case MOTOR_LEFT: pulses = buggySensor.left_encoder.pulseFromStartup; break;
		case MOTOR_RIGHT: pulses = buggySensor.right_encoder.pulseFromStartup; break;
		default: pulses= -1; break;
	}
	*/
	return pulses;
}

char getDigitalInput(unsigned char inputNumber){
	char inputState;

	inputState = sensor.din[inputNumber];
							/*;
	switch(inputNumber){
		case 0: inputState = buggySensor.din0; break;
		case 1: inputState = buggySensor.din1; break;
		default: inputState=-1; break;
	}
	*/
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


// ---------------------------------------------------------------------------
// SETMOTORDIRECTION
// !!!!!!!!!!!!! FONCTION A RETRAVAILLER !!!!!!!!!!!!!!!!!!!
// ---------------------------------------------------------------------------
int setMotorDirection(int motorName, int direction){

	unsigned char motorAdress;

	// Conversion No de moteur en adresse du registre du PWM controleur
	motorAdress=getOrganI2Cregister(MOTOR, motorName);
	/*
	switch(motorName){
		case MOTOR_LEFT: 	motorAdress = DCM0;	break;
		case MOTOR_RIGHT :  motorAdress = DCM1;	break;
		default : return(0);
	}
	 */

	switch(direction){
		case BUGGY_FORWARD :	set_i2c_command_queue(&MCP2308_DCmotorSetRotation, motorAdress, MCW); break;
								//MCP2308_DCmotorSetRotation(motorAdress, MCW); break;
		case BUGGY_BACK :
								set_i2c_command_queue(&MCP2308_DCmotorSetRotation, motorAdress, MCCW); break;
								//MCP2308_DCmotorSetRotation(motorAdress, MCCW); break;
		case BUGGY_STOP : 		break;
		default :		     	break;
	}


	return(1);
}

// ------------------------------------------------------------------------------------
// CHECKMOTORPOWER:
// Fonction appelée periodiquement pour la gestion de l'acceleration
// Décelération du moteur.
// Elle va augmenté ou diminuer la velocite du moteur jusqu'a atteindre la consigne
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

	smAddr=getOrganI2Cregister(SERVOM, smName);
	set_i2c_command_queue(&PCA9685_setServoPos, smAddr, angle);
}

void setLedPower(unsigned char ledID, unsigned char power){
	unsigned char ledAdress;
	ledAdress=getOrganI2Cregister(LED, ledID);
	set_i2c_command_queue(&PCA9685_setLedPower, ledAdress, power);
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
			case MOTOR_LEFT : organAdr = PCA_DCM0; break;
			case MOTOR_RIGHT : organAdr = PCA_DCM1; break;
			default :	organAdr = UNKNOWN; break;
		}
	}

	if(organType == SERVOM){
		switch(organName){
			case SERVO_0 : organAdr = PCA_CN13_4; break;
			case SERVO_1 : organAdr = PCA_CN11_4; break;
			case SERVO_2 : organAdr = PCA_CN14_7; break;
                        case SERVO_3 : organAdr = PCA_CN15_4; break;
                        case SERVO_4 : organAdr = PCA_CN8_3; break;
                        case SERVO_5 : organAdr = PCA_CN7_3; break;
                        case SERVO_6 : organAdr = PCA_CN5_7; break;
                        case SERVO_7 : organAdr = PCA_CN4_4; break;
                        case SERVO_8 : organAdr = PCA_CN9_3; break;
                        case SERVO_9 : organAdr = PCA_CN10_3; break;
                        
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
