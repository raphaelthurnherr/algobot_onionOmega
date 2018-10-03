#ifdef I2CSIMU

#include <stdio.h>
#include "boardHWsimu.h"
#include "../buggy_descriptor.h"


unsigned char configPWMdevice(void);                            // Configuration of the PCA9685 for 50Hz operation
unsigned char configGPIOdevice(void);                           // Configuration IO mode of the MCP28003
unsigned char configRGBdevice(void);                            // Configuration mode of the BH1745NUC RGB sensor

char MCP2308_ReadGPIO(unsigned char input);                     // Get the selected input value on device
int EFM8BB_readSonarDistance(void);				// Get distance in mm from the EFM8BB microcontroller
char EFM8BB_readDigitalInput(unsigned char InputNr);		// Get digital input state in mm from the EFM8BB microcontroller
int EFM8BB_readBatteryVoltage(void);				// Get the battery voltage in mV from EFM8BB microcontroller
int EFM8BB_readFrequency(unsigned char wheelNb);		// Get the wheel frequency
int EFM8BB_readPulseCounter(unsigned char wheelNb);
int EFM8BB_clearWheelDistance(unsigned char wheelNb);
char MCP2308_ReadGPIO(unsigned char input);
int EFM8BB_getFirmwareVersion(void);                            // Get the MCU firmware version
int EFM8BB_getBoardType(void);                                  // Get the type of the board.
int BH1745_getRGBvalue(unsigned char sensorNb, int color);                              // Get the value for specified color

unsigned char motorDCadr[2]={PCA_DCM0, PCA_DCM1};		// Valeur de la puissance moteur

//================================================================================
// BUGGYBOARDINIT
// Initialisation of the board (PWM Driver, GPIO driver, etc..)
//================================================================================

unsigned char buggyBoardInit(void){
	unsigned char err;

	err+=configPWMdevice();					// Configuration du Chip PWM pour gestion de la v�locit� des DC moteur et angle servomoteur
	err+=configGPIOdevice();				// Confguration du chip d'entr�es/sortie pour la gestion du sens de rotation des moteur DC
        err+=configRGBdevice();                                 // Configuration du capteur de couleur RGBC
        
	MCP2308_DCmotorState(1);				// Set the HDRIVER ON
	if(err)
		return 0;					// Erreur
	else return 1;
}


//================================================================================
// DCMOTORSTATE
// Defini l'etat general de tout les moteurs DC (Driver pont en H)
//
//================================================================================

void MCP2308_DCmotorState(unsigned char state){
    printf(" #SIMU-> DC motor state: %d\n",state);
}

//================================================================================
// MCP2308_ReadGPIO
// Lecture d'un GPIO sur le peripherique 
//
//================================================================================

char MCP2308_ReadGPIO(unsigned char input){
    printf(" #SIMU-> Read MCP GPIO on input: %d\n",input);
    return 1;
}


//================================================================================
// DCMOTORSETSPEED
// D�fini le duty cyle � appliquer sur les sorties du chip PWM (0..100%)
// motorAdr: Adresse de sortie du contr�leur PWM sur lequel doit �tre appliqu� le dutyCycle
//================================================================================

void PCA9685_DCmotorSetSpeed(unsigned char motorAdr, unsigned char dutyCycle){
    printf(" #SIMU-> DC motor adress: %d with dutycycle: %d\n", motorAdr, dutyCycle);
}


//================================================================================
// DCMOTORSETROTATION
// D�fini le sense de rottion d'un moteur DC (sens horaire, antihoraire ou stop)
//================================================================================

void MCP2308_DCmotorSetRotation(unsigned char motorAdr, unsigned char direction){
    printf(" #SIMU-> DC motor adress: %d with direction: %d\n", motorAdr, direction);
}


//================================================================================
// SETSERVOPOS
// D�fini la position a appliquer au servomoteur
// smAddr = adresse pour le port de sortie concern� sur le chip PCA9685
// position= Angle de positionnement en degr� du servomoteur (de 0..100%)
//================================================================================

void PCA9685_setServoPos(unsigned char smAddr, unsigned char position){
    printf(" #SIMU-> SERVO motor adress: %d with position: %d\n", smAddr, position);
}

//================================================================================
// SETLEDPOWER
// D�fini l'intensit� d'�clairage pour led @ 50HZ selon config PCA9685
// smAddr = adresse pour le port de sortie concern� sur le chip PCA9685
// power = Intensit� d'�clairage ( 0..100%)
//================================================================================

void PCA9685_setLedPower(unsigned char smAddr, unsigned char power){
    printf(" #SIMU-> LED adress: %d with power: %d\n", smAddr, power);
}



//================================================================================
// CONFIGPWMDEVICE
// Configuration initial pour le controleur PWM PCA9685
//	- S�l�ction horloge interne 25MHz
//	- Mode op�ration � 50Hz (Principalement pour la commande de servomoteurs)
//	- Sorties non invers�es
//	- Ps d'auto incrementation
//================================================================================
unsigned char configPWMdevice(void){
    printf(" #SIMU-> Configuring PWM device\n");
    return 0;
}


//================================================================================
// CONFIGGPIODEVICE
// Configuration initiale pour le GPIO Controleur MCP2308
//	- Non auto-incrementation
//	- Pull-up activ�e
//	- Pin en sortie
//================================================================================
unsigned char configGPIOdevice(void){
    printf(" #SIMU-> Configuring GPIO device\n");
    return 0;
}


//================================================================================
// CONFIGRGBDEVICE
// Configuration initiale pour le capteur RGB BH1745NUC
//	- Registre de contr�le

//================================================================================
unsigned char configRGBdevice(void){
   printf(" #SIMU-> Configuring RGB device\n");
   return 0;
}






// -------------------------------------------------------------------
// GETSONARDISTANCE
// Lecture de la distance mesuree au sonar [mm]
// Retourn une valeures positve correspondant � la distance en mm
// ou -1 si erreur de lecture
// -------------------------------------------------------------------
int EFM8BB_readSonarDistance(void){

    unsigned int SonarDistance_mm =555;
    printf(" #SIMU-> Read sonar distance\n");

    return SonarDistance_mm;
}


// -------------------------------------------------------------------
// GETBATTERYVOLTAGE
// Lecture de la tension batterie mesuree en mV
// Retourne une valeures positve correspondant � la tension en mV
// ou -1 si erreur de lecture
// -------------------------------------------------------------------
int EFM8BB_readBatteryVoltage(void){
    unsigned int batteryVoltage_mV =3830;
    printf(" #SIMU-> Read battery voltage\n");

    return batteryVoltage_mV;
}

// -------------------------------------------------------------------
// GETFREQUENCY
// Get frequency measured on EFM8BB
// ou -1 si erreur de lecture
// -------------------------------------------------------------------
int EFM8BB_readFrequency(unsigned char wheelNb){
    int freq=101;
    printf(" #SIMU-> Read frequency on encoder %d\n", wheelNb);
    return freq;
}

// -------------------------------------------------------------------
// GETPULSECOUNTER
// Get pulse counter on EFM8BB
// ou -1 si erreur de lecture
// -------------------------------------------------------------------
int EFM8BB_readPulseCounter(unsigned char wheelNb){
    int pulses=1245;
    printf(" #SIMU-> Read pulses on encoder %d\n", wheelNb);
    return pulses;
}

// -------------------------------------------------------------------
// CLEARWHEELDISTANCE
// RetourReset to 0 the pulse counter on EFM8BB
// ou -1 si erreur de lecture
// -------------------------------------------------------------------
int EFM8BB_clearWheelDistance(unsigned char wheelNb){
    printf(" #SIMU-> Reset distance on encoder %d\n", wheelNb);
    return 0;
}

// -------------------------------------------------------------------
// GETDIGITALINPUT
// Mesure de l'etat des entrees digitale
// Param�tre "InputNr" plus utilis�...
// -------------------------------------------------------------------
char EFM8BB_readDigitalInput(unsigned char InputNr){
    printf(" #SIMU-> Read DIN on input %d\n", InputNr);
    return 0;
}


// -------------------------------------------------------------------
// GETFIRMWAREVERSION
// RECUPERE LA VERSION FIRMWARE DU MCU
// -------------------------------------------------------------------
int EFM8BB_getFirmwareVersion(void){
    printf(" #SIMU-> Read MCU version\n");
    return 99;
}

// -------------------------------------------------------------------
// GETBOARDTYPE
// Recupere le type de montage de la carte
// -------------------------------------------------------------------
int EFM8BB_getBoardType(void){
    printf(" #SIMU-> Read board type\n");
    return 0xAA;
}

// -------------------------------------------------------------------
// BH1445GETRGBVALUE
// RECUPERE LA VALEUR DU REGISTRE POUR LA COULEUR:
// RED =0, GREEN=1, BLUE=2, CLEAR=3
// -------------------------------------------------------------------
int BH1745_getRGBvalue(unsigned char sensorNb, int color){
        int value=-1;  // Registre CLEAR LSB par defaut 
        
        switch(color){
            case RED :   value=0xBB; break;
            case GREEN : value=0xCC; break;
            case BLUE :  value=0xDD; break;
            case CLEAR : value=0xEE; break;
            
            default : value=-1; break;
        }

        printf(" #SIMU-> Read color: %d on sensor: %d \n", value, sensorNb);        
        return value;
}

#endif