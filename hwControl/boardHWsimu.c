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
int EFM8BB_getFirmwareVersion(void);                            // Get the MCU firmware version
int EFM8BB_getBoardType(void);                                  // Get the type of the board.
int BH1745_getRGBvalue(unsigned char sensorNb, int color);                              // Get the value for specified color

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

    static unsigned int SonarDistance_mm = 2500;
    
    unsigned char sens;
   
    sens = rand() % 50;
    
    if(sens>25)
        SonarDistance_mm += rand() % 50;
    else
        SonarDistance_mm -= rand() % 50;
    
    if (SonarDistance_mm >= 4500)
        SonarDistance_mm=2500;
    
    if (SonarDistance_mm <= 20)
        SonarDistance_mm=2500;
    
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
    int freq=101;
    //printf(" #SIMU-> Read frequency on encoder %d\n", wheelNb);
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
        unsigned int static red, green, blue, clear;
        unsigned char Rsens,Gsens, Bsens, Csens;
                
        Rsens = rand() % 20;
        Gsens = rand() % 40;
        Bsens = rand() % 60;
        Csens = rand() % 50;
        
        //Simulator Only
        if(Rsens>10)
            red += rand() % 25;
        else
            red -= rand() % 25;
        
        if(Gsens>20)
            green += rand() % 25;
        else
            green -= rand() % 25; 
        
        if(Bsens>30)
            blue += rand() % 25;
        else
            blue -= rand() % 25; 

        if(Csens>25)
            clear -= rand() % 25;
        else
            clear -= rand() % 25; 

        // Restore implicite default value 
        if(red >20000)
            red=3000;
        if(green >20000)
            green=3000;
        if(blue >20000)
            blue=3000;
        if(clear >20000)
            clear=3000;
        
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
        

        
        
       // printf(" #SIMU-> Read color %d: %d on sensor: %d \n",color, value, sensorNb);        
        return value;
}

#endif