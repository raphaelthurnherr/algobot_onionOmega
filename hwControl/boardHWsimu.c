#ifdef I2CSIMU

#include <stdio.h>
#include "boardHWsimu.h"
#include "../buggy_descriptor.h"


unsigned char buggyBoardInit(void);                             // Initialisation of the board (PWM Driver, GPIO driver, etc..)

unsigned char configPWMdevice(void);                            // Configuration of the PCA9685 for 50Hz operation
unsigned char configGPIOdevice(void);                           // Configuration IO mode of the MCP28003
unsigned char configRGBdevice(void);                            // Configuration mode of the BH1745NUC RGB sensor
unsigned char configStepMotorDriver(void);                           // Configuration du contrôleur de moteur pas à pas

char MCP2308_ReadGPIO(unsigned char input);                     // Get the selected input value on device
int EFM8BB_readSonarDistance(void);				// Get distance in mm from the EFM8BB microcontroller
char EFM8BB_readDigitalInput(unsigned char InputNr);		// Get digital input state in mm from the EFM8BB microcontroller
int EFM8BB_readBatteryVoltage(void);				// Get the battery voltage in mV from EFM8BB microcontroller
int EFM8BB_readFrequency(unsigned char wheelNb);		// Get the wheel frequency
int EFM8BB_readPulseCounter(unsigned char wheelNb);
int EFM8BB_clearWheelDistance(unsigned char wheelNb);
int EFM8BB_getFirmwareVersion(void);                            // Get the MCU firmware version
int EFM8BB_getBoardType(void);                                  // Get the type of the board.

int PCA9629_ReadMotorState(int motorNumber);                             // Lecture du registre de contrôle du moteur
void PCA9685_DCmotorSetSpeed(unsigned char motorAdr, unsigned char dutyCycle);

int PCA9629_StepperMotorControl(int motorNumber, int data);              //Configuration du registre "PAS" du driver moteur
int PCA9629_StepperMotorSetStep(int motorNumber, int stepCount);         //Configuration du registre "PAS" du driver moteur
int PCA9629_StepperMotorMode(int motorNumber, int data);                 // Mode action continue ou unique
int PCA9629_StepperMotorPulseWidth(int motorNumber, int data);           // Définition de la largeur d'impulstion

int BH1745_getRGBvalue(unsigned char sensorNb, int color);                   // Get the value for specified color

int I2C_readDeviceReg(unsigned char deviceAd, unsigned char registerAdr);    // Get the value for selected register on device
int I2C_writeDeviceReg(unsigned char deviceAd, unsigned char registerAdr, unsigned char data);    // Get the value for selected register on device

unsigned char motorDCadr[2]={PCA_DCM0, PCA_DCM1};		// Valeur de la puissance moteur

unsigned int pulses0, pulses1;

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
    unsigned char gpio, result;
    
    gpio = rand() % 20;
    
    if(gpio > 10)
        result = 0;
    else result = 1;
    
    //printf(" #SIMU-> Read MCP GPIO on input: %d\n",input);
    return result;
}


//================================================================================
// DCMOTORSETSPEED
// D�fini le duty cyle � appliquer sur les sorties du chip PWM (0..100%)
// motorAdr: Adresse de sortie du contr�leur PWM sur lequel doit �tre appliqu� le dutyCycle
//================================================================================

void PCA9685_DCmotorSetSpeed(unsigned char motorAdr, unsigned char dutyCycle){
    printf(" #SIMU-> DC motor speed adress: 0x%2x with dutycycle: %d\n", motorAdr, dutyCycle);
}


//================================================================================
// DCMOTORSETROTATION
// D�fini le sense de rottion d'un moteur DC (sens horaire, antihoraire ou stop)
//================================================================================

void MCP2308_DCmotorSetRotation(unsigned char motorAdr, unsigned char direction){
    printf(" #SIMU-> DC motor rotation adress: 0x%2x with direction: %d\n", motorAdr, direction);
}

//================================================================================
// STEPPERMOTORSETSTEP
// Paramètrage du nombre de pas dans les registres du driver moteur pour CW et CCW
//================================================================================

int PCA9629_StepperMotorSetStep(int motorNumber, int stepCount){
   	unsigned char err=0;
	unsigned char motorAddress = 0;
        
        motorAddress = PCA9629 + motorNumber;
        
        printf(" #SIMU-> STEP motor rotation adress: 0x%2x with step: %d\n", motorAddress, stepCount);
	return(err);
}

//================================================================================
// STEPPERMOTORCONTROL
// Registre de commande du driver de moteur
//================================================================================
int PCA9629_StepperMotorControl(int motorNumber, int data){
   	unsigned char err=0;
	unsigned char motorAddress = 0;
        
        motorAddress = PCA9629 + motorNumber;

        printf(" #SIMU-> STEP motor CONTROL adress: 0x%2x with register data: %d\n", motorAddress, data);
        return(err);
}

//================================================================================
// READMOTORSTATE
// Lecture de l'état actuel du moteur (run/stop)
//================================================================================
int PCA9629_ReadMotorState(int motorNumber){
   	unsigned char err=0;
	unsigned char motorAddress = 0;
        
        motorAddress = PCA9629 + motorNumber;

        return(err);
}

//================================================================================
// STEPPERMOTORMODE
// Registre de commande du mode du driver
// Mode action continue ou mode action unique
//================================================================================
int PCA9629_StepperMotorMode(int motorNumber, int data){
   	unsigned char err=0;
	unsigned char motorAddress = 0;
        
        motorAddress = PCA9629 + motorNumber;

        printf(" #SIMU-> STEP motor MODE adress: 0x%2x with register data: %d\n", motorAddress, data);
        
        return(err);
}

//================================================================================
// STEPPERMOTORPULSEWIDTH
// Registre de configuration de la largeur d'impulsion moteur pour les sens CW et CCW
// Comprise entre 2mS (500Hz) et 22.5mS(44Hz) pour bon fonctionnement du mnoteur
//================================================================================
int PCA9629_StepperMotorPulseWidth(int motorNumber, int data){
   	unsigned char err=0;
	unsigned char motorAddress = 0;
        
        motorAddress = PCA9629 + motorNumber;

        printf(" #SIMU-> STEP motor Pulse width adress: 0x%2x with register data: %d\n", motorAddress, data);
        return(err);
}
//================================================================================
// SETSERVOPOS
// D�fini la position a appliquer au servomoteur
// smAddr = adresse pour le port de sortie concern� sur le chip PCA9685
// position= Angle de positionnement en degr� du servomoteur (de 0..100%)
//================================================================================

void PCA9685_setServoPos(unsigned char smAddr, char position){
    printf(" #SIMU-> SERVO motor adress: 0x%2x with position: %d\n", smAddr, position);
}

//================================================================================
// SETLEDPOWER
// D�fini l'intensit� d'�clairage pour led @ 50HZ selon config PCA9685
// smAddr = adresse pour le port de sortie concern� sur le chip PCA9685
// power = Intensit� d'�clairage ( 0..100%)
//================================================================================

void PCA9685_setLedPower(unsigned char smAddr, unsigned char power){
    printf(" #SIMU-> LED/PWM adress: 0x%2x with power: %d\n", smAddr, power);
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

    static unsigned int SonarDistance_mm = 600;
    
    unsigned char sens;
   
    sens = rand() % 50;
    
    if(sens>25)
        SonarDistance_mm += rand() % 50;
    else
        SonarDistance_mm -= rand() % 50;
    
    if (SonarDistance_mm >= 1200)
        SonarDistance_mm=600;
    
    if (SonarDistance_mm <= 20)
        SonarDistance_mm=600;
    
    //printf(" #SIMU-> Read sonar distance: %d\n", SonarDistance_mm);

    return SonarDistance_mm;
}


// -------------------------------------------------------------------
// GETBATTERYVOLTAGE
// Lecture de la tension batterie mesuree en mV
// Retourne une valeures positve correspondant � la tension en mV
// ou -1 si erreur de lecture
// -------------------------------------------------------------------
int EFM8BB_readBatteryVoltage(void){
    static unsigned int batteryVoltage_mV =3830;
    unsigned char sens;
   
    sens = rand() % 20;
    
    if(sens>10)
        batteryVoltage_mV += rand() % 25;
    else
        batteryVoltage_mV -= rand() % 25;
    
    if (batteryVoltage_mV >= 4200)
        batteryVoltage_mV=3830;
    
    if (batteryVoltage_mV <= 3300)
        batteryVoltage_mV=3830;
    //printf(" #SIMU-> Read battery voltage: %d\n", batteryVoltage_mV);

    return batteryVoltage_mV;
}

// -------------------------------------------------------------------
// GETFREQUENCY
// Get frequency measured on EFM8BB
// ou -1 si erreur de lecture
// -------------------------------------------------------------------
int EFM8BB_readFrequency(unsigned char wheelNb){
    int freq=150;
    freq = rand() % 100;
        
    //printf(" #SIMU-> Read frequency on encoder %d: %d\n", wheelNb, freq);
    return freq;
}

// -------------------------------------------------------------------
// GETPULSECOUNTER
// Get pulse counter on EFM8BB
// ou -1 si erreur de lecture
// -------------------------------------------------------------------
int EFM8BB_readPulseCounter(unsigned char wheelNb){

    int result;
    
        
        pulses0 += rand() % 25;
        pulses1 += rand() % 25;
    
        switch(wheelNb){
            case 0 : result=pulses0; break;
            case 1 : result=pulses1; break;
            default: result=0;break;
        }
    
    //printf(" #SIMU-> Read pulses on encoder %d\n", wheelNb);
    return result;
}

// -------------------------------------------------------------------
// CLEARWHEELDISTANCE
// RetourReset to 0 the pulse counter on EFM8BB
// ou -1 si erreur de lecture
// -------------------------------------------------------------------
int EFM8BB_clearWheelDistance(unsigned char wheelNb){
    
    switch(wheelNb){
        case 0 : pulses0=0; break;
        case 1 : pulses1=0; break;
        default: break;
    }
            
    printf(" #SIMU-> Reset distance on encoder %d\n", wheelNb);
    return 0;
}

// -------------------------------------------------------------------
// GETDIGITALINPUT
// Mesure de l'etat des entrees digitale
// Param�tre "InputNr" plus utilis�...
// -------------------------------------------------------------------
char EFM8BB_readDigitalInput(unsigned char InputNr){
        unsigned char din;

        din = rand() % 255;
        
        
    //printf(" #SIMU-> Read DIN on input %d: %d\n", InputNr, result);
    return din;
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
        unsigned int static red=1000, green=2500, blue=11000, clear=1000;
        unsigned char Rsens,Gsens, Bsens, Csens;
                
        Rsens = rand() % 50;
        Gsens = rand() % 100;
        Bsens = rand() % 150;
        Csens = rand() % 200;
        
        //Simulator Only
        if(Rsens>10)
            red += rand() % 100;
        else
            red -= rand() % 100;
        
        if(Gsens>20)
            green += rand() % 100;
        else
            green -= rand() % 100; 
        
        if(Bsens>30)
            blue += rand() % 100;
        else
            blue -= rand() % 100; 

        if(Csens>25)
            clear -= rand() % 100;
        else
            clear -= rand() % 100; 

        // Restore implicite default value 
        if(red >11000)
            red=1000;
        if(green >27000)
            green=2500;
        if(blue >21000)
            blue=11000;
        if(clear >2000)
            clear=1000;
        
        // End of simu
        
        switch(color){
            case RED :   value=red; break;
            case GREEN : value=green; break;
            case BLUE :  value=blue; break;
            case CLEAR : value=clear; break;
            
            default : value=-1; break;
        }

        
        //  Set to positive value
        if (value < 0)
            value*=-1;
        

        if(sensorNb>0)
            if(Rsens>10)
                value += rand() % 250;
            else value -= rand() % 250;
        
       // printf(" #SIMU-> Read color %d: %d on sensor: %d \n",color, value, sensorNb);        
        return value;
}

#endif