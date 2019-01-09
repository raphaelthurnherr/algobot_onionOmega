
#ifndef I2CSIMU

#include "boardHWctrl.h"
#include <onion-i2c.h>
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

void PCA9685_DCmotorSetSpeed(unsigned char motorAdr, unsigned char dutyCycle);
void PCA9685_setServoPos(unsigned char smAddr, char position);

int PCA9629_StepperMotorControl(int motorNumber, int data);              //Configuration du registre "PAS" du driver moteur
int PCA9629_StepperMotorSetStep(int motorNumber, int stepCount);         //Configuration du registre "PAS" du driver moteur
int PCA9629_StepperMotorMode(int motorNumber, int data);                 // Mode action continue ou unique
int PCA9629_StepperMotorPulseWidth(int motorNumber, int data);           // Définition de la largeur d'impulstion
int PCA9629_ReadMotorState(int motorNumber);                             // Lecture du registre de contrôle du moteur
int BH1745_getRGBvalue(unsigned char sensorNb, int color);               // Get the value for specified color

int I2C_readDeviceReg(unsigned char deviceAd, unsigned char registerAdr);    // Get the value for selected register on device
int I2C_writeDeviceReg(unsigned char deviceAd, unsigned char registerAdr, unsigned char data);    // Get the value for selected register on device

//unsigned char motorDCadr[2]={PCA_DCM0, PCA_DCM1};		// Valeur de la puissance moteur

//================================================================================
// BUGGYBOARDINIT
// Initialisation of the board (PWM Driver, GPIO driver, etc..)
//================================================================================

unsigned char buggyBoardInit(void){
	unsigned char err;
        
	err+=configPWMdevice();					// Configuration du Chip PWM pour gestion de la v�locit� des DC moteur et angle servomoteur
	err+=configGPIOdevice();				// Confguration du chip d'entr�es/sortie pour la gestion du sens de rotation des moteur DC
        err+=configRGBdevice();                                 // Configuration du capteur de couleur RGBC
        err+=configStepMotorDriver();                           // Configuration du contrôleur de moteur pas à pas
        
        // Reset la distance de la carte EFM8BB
	EFM8BB_clearWheelDistance(MOTOR_ENCODER_LEFT);
	EFM8BB_clearWheelDistance(MOTOR_ENCODER_RIGHT);
        
	MCP2308_DCmotorState(1);				// Set the HDRIVER ON
	if(err){
            printf("Kehops I2C devices initialization with %d error\n", err);
            return 0;   // Erreur
        }
	else
            return 1;
}


//================================================================================
// DCMOTORSTATE
// Defini l'etat general de tout les moteurs DC (Driver pont en H)
//
//================================================================================

void MCP2308_DCmotorState(unsigned char state){
	int MCP2308_GPIO_STATE;
        char err;
        
        err=i2c_readByte(0, MCP2308, 0x09, &MCP2308_GPIO_STATE);
        
	if(state) MCP2308_GPIO_STATE |= 0x10;                                   // Activation du driver pont en H
	else MCP2308_GPIO_STATE &= 0xEF;					// d�sactivation du driver pont en H

        i2c_write(0, MCP2308, 0x0A, MCP2308_GPIO_STATE);
}

//================================================================================
// MCP2308_ReadGPIO
// Lecture d'un GPIO sur le peripherique 
//
//================================================================================

char MCP2308_ReadGPIO(unsigned char input){
	int MCP2308_GPIO_STATE;
        unsigned char value=0;
        unsigned char err;
        
        err=i2c_readByte(0, MCP2308, 0x09, &MCP2308_GPIO_STATE);
        
        switch(input){
            case BTN_0 : if(MCP2308_GPIO_STATE & 0x40) value = 0;
                         else value = 1; break;
            case BTN_1 : if(MCP2308_GPIO_STATE & 0x20) value = 0;
                         else value = 1; break;
            default : value = -1; break;
        }
        
	if(!err){
		return value;
	}else return -1;
       
}


//================================================================================
// DCMOTORSETSPEED
// D�fini le duty cyle � appliquer sur les sorties du chip PWM (0..100%)
// motorAdr: Adresse de sortie du contr�leur PWM sur lequel doit �tre appliqu� le dutyCycle
//================================================================================

void PCA9685_DCmotorSetSpeed(unsigned char motorAdr, unsigned char dutyCycle){
	unsigned int power;
	unsigned char PowerLow;
	unsigned char PowerHigh;

	// Conversion du dutyclycle en valeur � appliquer au contr�leur PWM
	power = (4095*dutyCycle)/100;
	PowerLow = power&0x00FF;;
	PowerHigh = (power&0x0F00) >>8;

	i2c_write(0, PCA9685, motorAdr, PowerLow);
        i2c_write(0, PCA9685, motorAdr+1, PowerHigh);
}


//================================================================================
// DCMOTORSETROTATION
// D�fini le sense de rottion d'un moteur DC (sens horaire, antihoraire ou stop)
//================================================================================

void MCP2308_DCmotorSetRotation(unsigned char motorAdr, unsigned char direction){
	int MCP2308_GPIO_STATE;

	// Sélection du chip d'entrée/sortie qui pilote le pont en H
	i2c_readByte(0, MCP2308, 0x09, &MCP2308_GPIO_STATE);
        
        // le bit n�n�ssaire
	//	SELECTION DU MOTEUR No 0
	if(motorAdr==PCA_DCM0){
		// Désactive la commande du moteur
		// avant de changer de sens de rotation
		MCP2308_GPIO_STATE &= 0xF9;

                i2c_write(0, MCP2308, 0x0A, MCP2308_GPIO_STATE);
                
		// S�l�ction du sens de rotation du moteur ou OFF
		switch(direction){
			case MCW 	 :  MCP2308_GPIO_STATE |= 0x02; break;
			case MCCW 	 :  MCP2308_GPIO_STATE |= 0x04; break;
			case MSTOP 	 :  MCP2308_GPIO_STATE |= 0x00; break;
			default		 : ;break;
		}

                i2c_write(0, MCP2308, 0x0A, MCP2308_GPIO_STATE);
        }

//	SELECTION DU MOTEUR No 1
	if(motorAdr==PCA_DCM1){

		// D�sactive la commande du moteur
		// avant de changer de sens de rotation
		MCP2308_GPIO_STATE &= 0xF6;										// Force H-Bridge Off for motor 1

                i2c_write(0, MCP2308, 0x0A, MCP2308_GPIO_STATE);
		// S�l�ction du sens de rotation du moteur ou OFF
		switch(direction){
			case MCW 	 :  MCP2308_GPIO_STATE |= 0x01; break;
			case MCCW 	 :  MCP2308_GPIO_STATE |= 0x08; break;
			case MSTOP 	 :  MCP2308_GPIO_STATE |= 0x00; break;
			default		 : ;break;
		}

                i2c_write(0, MCP2308, 0x0A, MCP2308_GPIO_STATE);
        }

}

//================================================================================
// STEPPERMOTORSETSTEP
// Paramètrage du nombre de pas dans les registres du driver moteur pour CW et CCW
//================================================================================

int PCA9629_StepperMotorSetStep(int motorNumber, int stepCount){
   	unsigned char err=0;
	unsigned char motorAddress = 0;
        
        motorAddress = PCA9629 + motorNumber;

        // Configuration du registre de nombre de pas dans le sens horaire
        err += i2c_write(0, motorAddress, 0x12, stepCount&0x00FF);           // Défini le nombre de pas dans le registre LOW
        err += i2c_write(0, motorAddress, 0x13, (stepCount&0xFF00)>>8);    // Défini le nombre de pas dans le registre HIGH

        // Configuration du registre de nombre de pas dans le sens anti-horaire
        err += i2c_write(0, motorAddress, 0x14, stepCount&0x00FF);           // Défini le nombre de pas dans le registre LOW
        err += i2c_write(0, motorAddress, 0x15, (stepCount&0xFF00)>>8);    // Défini le nombre de pas dans le registre HIGH        

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

        // Configuration du registre dans le sens horaire
        err += i2c_write(0, motorAddress, 0x1A, data & 0x00FF);           // Défini le nombre de rotation dans le registre LOW    
        return(err);
}

//================================================================================
// READMOTORSTATE
// Lecture de l'état actuel du moteur (run/stop)
//================================================================================
int PCA9629_ReadMotorState(int motorNumber){
   	unsigned char err=0;
	unsigned char motorAddress = 0;
        int regState = 0;
        
        motorAddress = PCA9629 + motorNumber;

        // LEcture du registre de controle du driver moteur
        //err += i2c_write(0, motorAddress, 0x1A, data & 0x00FF);           // Défini le nombre de rotation dans le registre LOW    
        err += i2c_readByte(0, motorAddress, 0x1A, &regState);
        //printf("\nRegister motor State: %d\n", regState);
        
        if(!err){    
            return regState;
	}else{
            printf("PCA9629_ReadMotorState() -> Read error\n");
            return -1;
        }
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

        // Configuration du registre dans le sens horaire
        err += i2c_write(0, motorAddress, 0x0F, data & 0x00FF);           // Défini le nombre de rotation dans le registre LOW    
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

        
        // Configuration du registre dans le sens horaire
        err+= i2c_write(0, PCA9629, 0x16, data & 0x00FF);         // CWPWL - Vitesse / Largeur d'impulsion pour CW
        err+= i2c_write(0, PCA9629, 0x17, (data & 0xFF00)>>8);    // CWPWH
        
        err+= i2c_write(0, PCA9629, 0x18, data & 0x00FF);         // CCWPWL - Vitesse / Largeur d'impulsion pour CCW
        err+= i2c_write(0, PCA9629, 0x19, (data & 0xFF00)>>8);    // CCWPWH
        return(err);
}

//================================================================================
// SETSERVOPOS
// D�fini la position a appliquer au servomoteur
// smAddr = adresse pour le port de sortie concern� sur le chip PCA9685
// position= Angle de positionnement en degr� du servomoteur (de 0..100%)
//================================================================================

void PCA9685_setServoPos(unsigned char smAddr, char position){
	unsigned int dutyCycleValue;
	unsigned char dCLow;
	unsigned char dCHigh;

        // V�rifie que le positionnement d�fini soit entre 0 et 100%
	if(position>100)
            position=100;
        
        if(position >= 0)
            dutyCycleValue = 156+(position*2.72);
        else dutyCycleValue = 0;                    // Turn off the servomotor (no refresh)   

	dCLow = dutyCycleValue&0x00FF;;
	dCHigh = (dutyCycleValue&0x0F00) >>8;

//	Applique les nouvelles valeures

        i2c_write(0, PCA9685, smAddr, dCLow);
        i2c_write(0, PCA9685, smAddr+1, dCHigh);
}

//================================================================================
// SETLEDPOWER
// D�fini l'intensit� d'�clairage pour led @ 50HZ selon config PCA9685
// smAddr = adresse pour le port de sortie concern� sur le chip PCA9685
// power = Intensit� d'�clairage ( 0..100%)
//================================================================================

void PCA9685_setLedPower(unsigned char smAddr, unsigned char power){
	unsigned int dutyCycleValue;
	unsigned char dCLow;
	unsigned char dCHigh;

//	i2cSelectSlave(PCA9685);								// S�l�ction du chip PWM

	// V�rifie que la puissance d�finie soit entre 0 et 100%
	if(power>100)
		power=100;

	// Conversion de la puissance 0..100% en valeur de timer pour PCA9685
	dutyCycleValue = (4096/100)*power;
	dCLow = dutyCycleValue&0x00FF;;
	dCHigh = (dutyCycleValue&0x0F00) >>8;

//	Applique les nouvelles valeures

        i2c_write(0, PCA9685, smAddr, dCLow);
        i2c_write(0, PCA9685, smAddr+1, dCHigh);
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
	unsigned char err=0;
//	err=i2cSelectSlave(PCA9685);

	// Registre MODE1, sleep before config, horloge interne � 25MHz

        err+= i2c_write(0, PCA9685, 0x00, 0x10);
        
	// Prescaler pour op�ration 50Hz

        err+= i2c_write(0, PCA9685, 0xFE, 0x81);
        //err+= i2c_write(0, PCA9685, 0xFE, 0x00);

	// Registre MODE 2, sorties non invers�es

        err+= i2c_write(0, PCA9685, 0x01, 0x04);
        
	// TOUTES LED ON au clock 0

        err+= i2c_write(0, PCA9685, 0xFA, 0x00);
       
        err+= i2c_write(0, PCA9685, 0xFB, 0x00);

	// MODE 1, Syst�me pr�t,

        err+= i2c_write(0, PCA9685, 0x00, 0x81);

        if(err)
            printf("Kehops I2C PWM device initialization with %d error\n", err);
        
	return err;
}


//================================================================================
// CONFIGGPIODEVICE
// Configuration initiale pour le GPIO Controleur MCP2308
//	- Non auto-incrementation
//	- Pull-up activ�e
//	- Pin en sortie
//================================================================================
unsigned char configGPIOdevice(void){
	unsigned char err=0;

//	err=i2cSelectSlave(MCP2308);

	// Pas de auto-incrementation
        err+= i2c_write(0, MCP2308, 0x05, 0x20);
        
        // GPIO 0..4 en sorties, GPIO 5..6 en entree pour boutons
        if(!err) 
            err+= i2c_write(0, MCP2308, 0x00, 0x60);

        // Pull up activee         
        if(!err) 
            err+= i2c_write(0, MCP2308, 0x06, 0xFF);

        if(err)
            printf("Kehops I2C GPIO device initialization with %d error\n", err);
        
	return err;
}


//================================================================================
// CONFIGRGBDEVICE
// Configuration initiale pour le capteur RGB BH1745NUC
//	- Registre de contr�le

//================================================================================
unsigned char configRGBdevice(void){
    unsigned char err=0;
    // --- CONFIGURATION DU CAPTEUR 1
    
    // Configuration du registre de contr�le du capteur 1
    // b7:Initial reset, b6, INT inactive
    err+= i2c_write(0, BH1745_0, 0x40, 0xC0);   
    // Configuration du registre de contr�le mode1 (Measurement time=640mS)
    err+= i2c_write(0, BH1745_0, 0x41, 0x02);   
    // Configuration du registre de contr�le mode2 (Mesure RGBC active, Gain=1)
    err+= i2c_write(0, BH1745_0, 0x42, 0x10);   
    // Configuration du registre d'interruption (Interruption d�sactiv�es, pin d�sactiv�e
    err+= i2c_write(0, BH1745_0, 0x60, 0x00);
    // Configuration du registre de persistance (Interruption apr�s chaque mesure)
    err+= i2c_write(0, BH1745_0, 0x61, 0x00);  
    
    // --- CONFIGURATION DU CAPTEUR 2
    
    // Configuration du registre de contr�le du capteur 1
    // b7:Initial reset, b6, INT inactive
    err+= i2c_write(0, BH1745_1, 0x40, 0xC0);   
    // Configuration du registre de contr�le mode1 (Measurement time=640mS)
    err+= i2c_write(0, BH1745_1, 0x41, 0x02);   
    // Configuration du registre de contr�le mode2 (Mesure RGBC active, Gain=1)
    err+= i2c_write(0, BH1745_1, 0x42, 0x10);   
    // Configuration du registre d'interruption (Interruption d�sactiv�es, pin d�sactiv�e
    err+= i2c_write(0, BH1745_1, 0x60, 0x00);
    // Configuration du registre de persistance (Interruption apr�s chaque mesure)
    err+= i2c_write(0, BH1745_1, 0x61, 0x00); 
    
    if(err)
        printf("Kehops I2C RGB device initialization with %d error\n", err);
    
    return err;    
}


//================================================================================
// configStepMotorDriver
// Configuration initiale du contrôleur de moteur pas à pas
//	- Registre de contr�le

//================================================================================
unsigned char configStepMotorDriver(void){
    unsigned char err=0;
    
    // CONFIGURATION DU CIRCUIT DRIVER MOTEUR PAS A PAS
    // bit 6 et 7 non utilisés dans les registres
    
    
    err+= i2c_write(0, PCA9629, 0x00, 0x20);    // MODE - Configuration du registre MODE (pin INT désactivée, Allcall Adr. désactivé)
    err+= i2c_write(0, PCA9629, 0x01, 0xFF);    // WDTOI
    err+= i2c_write(0, PCA9629, 0x02, 0x00);    // WDCNTL
    err+= i2c_write(0, PCA9629, 0x03, 0x0F);    // IO_CFG
    err+= i2c_write(0, PCA9629, 0x04, 0x10);    // INTMODE
    err+= i2c_write(0, PCA9629, 0x05, 0x1F);    // MSK
    err+= i2c_write(0, PCA9629, 0x06, 0x00);    // INTSTAT
    //err+= i2c_write(0, PCA9629, 0x07, 0x);      // IP
    err+= i2c_write(0, PCA9629, 0x08, 0x00);    // INT_MTR_ACT
    err+= i2c_write(0, PCA9629, 0x09, 0x00);    // EXTRASTEPS0
    err+= i2c_write(0, PCA9629, 0x0A, 0x00);    // EXTRASTEPS1
    err+= i2c_write(0, PCA9629, 0x0B, 0x10);    // OP_CFG_PHS
    err+= i2c_write(0, PCA9629, 0x0C, 0x00);    // OP_STAT_TO
    err+= i2c_write(0, PCA9629, 0x0D, 0x00);    // RUCNTL
    err+= i2c_write(0, PCA9629, 0x0E, 0x00);    // RDCNTL
    err+= i2c_write(0, PCA9629, 0x0F, 0x01);    // PMA - 0x01 Action unique, 0x00 action continue
    err+= i2c_write(0, PCA9629, 0x10, 0x05);    // LOOPDLY_CW - Pour un delais de 20ms d'inversion de sens
    err+= i2c_write(0, PCA9629, 0x11, 0x05);    // LOOPDLY_CCW Pour un delais de 20ms d'inversion de sens
    err+= i2c_write(0, PCA9629, 0x12, 0xFF);    // CWSCOUNTL - Nombre de pas CW
    err+= i2c_write(0, PCA9629, 0x13, 0xFF);    // CWSCOUNTH
    err+= i2c_write(0, PCA9629, 0x14, 0xFF);    // CCWSCOUNTL - Nombre de pas CCW
    err+= i2c_write(0, PCA9629, 0x15, 0xFF);    // CCWSCOUNTH
    err+= i2c_write(0, PCA9629, 0x16, 0x9A);    // CWPWL - Vitesse / Largeur d'impulsion pour CW
    err+= i2c_write(0, PCA9629, 0x17, 0x02);    // CWPWH
    err+= i2c_write(0, PCA9629, 0x18, 0x9A);    // CCWPWL - Vitesse / Largeur d'impulsion pour CCW
    err+= i2c_write(0, PCA9629, 0x19, 0x02);    // CCWPWH
    err+= i2c_write(0, PCA9629, 0x1A, 0x00);    // MCNTL - Registre contrôle moteur
    err+= i2c_write(0, PCA9629, 0x1B, 0xE2);    // SUBA1
    err+= i2c_write(0, PCA9629, 0x1C, 0xE4);    // SUBA2
    err+= i2c_write(0, PCA9629, 0x1D, 0xE8);    // SUBA3
    err+= i2c_write(0, PCA9629, 0x1E, 0xE0);    // ALLCALLA
    //err+= i2c_write(0, PCA9629, 0x1F, 0x00);    // STEPCOUNT0
    //err+= i2c_write(0, PCA9629, 0x20, 0x00);    // STEPCOUNT1
    //err+= i2c_write(0, PCA9629, 0x21, 0x00);    // STEPCOUNT2
    //err+= i2c_write(0, PCA9629, 0x22, 0x00);    // STEPCOUNT3
   
    if(err)
        printf("Kehops I2C Step motor driver device initialization with %d error\n", err);
    
    return err;    
}




// -------------------------------------------------------------------
// GETSONARDISTANCE
// Lecture de la distance mesuree au sonar [mm]
// Retourn une valeures positve correspondant � la distance en mm
// ou -1 si erreur de lecture
// -------------------------------------------------------------------
int EFM8BB_readSonarDistance(void){
	unsigned char err;
	unsigned int SonarDistance_mm;
        unsigned int mmMSB;
        unsigned int mmLSB;
        
//	err=i2cSelectSlave(EFM8BB);						

	SonarDistance_mm=0;							// RAZ de la variable distance
        
        err=i2c_readByte(0, EFM8BB, SON0, &mmLSB);
        err+=i2c_readByte(0, EFM8BB, SON0+1, &mmMSB);
        
	if(!err){              
                SonarDistance_mm=((mmMSB<<8) & 0xFF00) + mmLSB;
                return SonarDistance_mm;
	}else{
            printf("EFM8BB_readSonarDistance() -> Read error\n");
            return -1;
        }
}


// -------------------------------------------------------------------
// GETBATTERYVOLTAGE
// Lecture de la tension batterie mesuree en mV
// Retourne une valeures positve correspondant � la tension en mV
// ou -1 si erreur de lecture
// -------------------------------------------------------------------
int EFM8BB_readBatteryVoltage(void){
	unsigned char err =0;
	unsigned int batteryVoltage_mV;
        unsigned int mVMSB;
        unsigned int mVLSB;

	batteryVoltage_mV=0;							// RAZ de la variable

        err=i2c_readByte(0, EFM8BB, VOLT0, &mVLSB);
        err+=i2c_readByte(0, EFM8BB, VOLT0+1, &mVMSB);
	if(!err){
                batteryVoltage_mV=((mVMSB<<8) & 0xFF00) + mVLSB;
		return batteryVoltage_mV;
	}else{
            printf("EFM8BB_readBatteryVoltage() -> Read error\n");
            return -1;
        }
}

// -------------------------------------------------------------------
// GETFREQUENCY
// Get frequency measured on EFM8BB
// ou -1 si erreur de lecture
// -------------------------------------------------------------------
int EFM8BB_readFrequency(unsigned char wheelNb){
	unsigned char err = 0, regAddr = 0;
	int freqLSB=0;
        int freqMSB=0;
        int frequency = 0;

	if(wheelNb == MOTOR_ENCODER_LEFT) regAddr = ENC_FREQ0;
	else regAddr = ENC_FREQ1;

	// RAZ de la variable
        err += i2c_readByte(0, EFM8BB, regAddr, &freqLSB);
        //err += i2c_readByte(0, EFM8BB, regAddr+1, &freqMSB);
        
        //frequency = ((freqMSB<<8) && 0xFF00 )+ freqLSB;
        
        frequency = freqLSB;
        
	if(!err){    
            //printf("EFM8BB_readFrequency() -> %d\n", frequency);
		return frequency;
	}else{
            printf("EFM8BB_readFrequency() -> Read error\n");
            return -1;
        }
}

// -------------------------------------------------------------------
// GETPULSECOUNTER
// Get pulse counter on EFM8BB
// ou -1 si erreur de lecture
// -------------------------------------------------------------------
int EFM8BB_readPulseCounter(unsigned char wheelNb){
	unsigned char err=0, regAddr=0;
	int pulseCount;
        int pcMSB=0;
        int pcLSB=0;

	if(wheelNb==0) {
		regAddr = ENC_CNT0;
	}
	else {
		regAddr = ENC_CNT1;
	}

	pulseCount=0;							// RAZ de la variable

        err=i2c_readByte(0, EFM8BB, regAddr, &pcLSB);
        usleep(1000);
        err+=i2c_readByte(0, EFM8BB, regAddr+1, &pcMSB);
        
        pulseCount = ((pcMSB<<8) & 0xFF00) + pcLSB;
                
	if(!err){
		return pulseCount;
	}else{
            printf("EFM8BB_readPulseCounter() -> Read error\n");
            return -1;
        }
}

// -------------------------------------------------------------------
// CLEARWHEELDISTANCE
// RetourReset to 0 the pulse counter on EFM8BB
// ou -1 si erreur de lecture
// -------------------------------------------------------------------
int EFM8BB_clearWheelDistance(unsigned char wheelNb){
	unsigned char err, regAddr;
	int pulseCount;

	if(wheelNb==0) {
		regAddr = ENC_CNT0_RESET;
	}
	else {
		regAddr = ENC_CNT1_RESET;
	}

	pulseCount=0;							// RAZ de la variable
        err +=i2c_readByte(0, EFM8BB, regAddr, &pulseCount);
	if(!err){
		return pulseCount;
	}else{
            printf("EFM8BB_clearWheelDistance() -> Read error\n");
            return -1;
        }
}

// -------------------------------------------------------------------
// GETDIGITALINPUT
// Mesure de l'etat des entrees digitale
// Param�tre "InputNr" plus utilis�...
// -------------------------------------------------------------------
char EFM8BB_readDigitalInput(unsigned char InputNr){
	unsigned char err;
	char inputState=0;
        
        err = i2c_readByte(0, EFM8BB, DIN_REG, &inputState);

	if(!err){
		return inputState;
	}else{
            printf("EFM8BB_readDigitalInput() -> Read error\n");
            return -1;
        }
}


// -------------------------------------------------------------------
// GETFIRMWAREVERSION
// RECUPERE LA VERSION FIRMWARE DU MCU
// -------------------------------------------------------------------
int EFM8BB_getFirmwareVersion(void){
	unsigned char err;
	int value=-1;
        
        err = i2c_readByte(0, EFM8BB, FIRMWARE_REG, &value);

	if(!err){
		return value;
	}else{
            printf("EFM8BB_getFirmwareVersion() -> Read error\n");
            return -1;
        }
}

// -------------------------------------------------------------------
// GETBOARDTYPE
// Recupere le type de montage de la carte
// -------------------------------------------------------------------
int EFM8BB_getBoardType(void){
	unsigned char err;
	int value=-1;
        
        err = i2c_readByte(0, EFM8BB, BOARDTYPE_REG, &value);
	if(!err){
		return value;
	}else{
            printf("EFM8BB_getBoardType() -> Read error\n");
            return -1;
        }
}

// -------------------------------------------------------------------
// BH1445GETRGBVALUE
// RECUPERE LA VALEUR DU REGISTRE POUR LA COULEUR:
// RED =0, GREEN=1, BLUE=2, CLEAR=3
// -------------------------------------------------------------------
int BH1745_getRGBvalue(unsigned char sensorNb, int color){
	unsigned char err;
	int value=-1;
        int RGBregAdr = 0x56;  // Registre CLEAR LSB par defaut 
        unsigned char SensorAdr=BH1745_0;
        
        unsigned int pcMSB=0;
        unsigned int pcLSB=0;
        
        switch(sensorNb){
            case RGBC_SENS_0: SensorAdr=BH1745_0;break;
            case RGBC_SENS_1: SensorAdr=BH1745_1;break;
            default: SensorAdr=BH1745_0;break;
        }
        
        switch(color){
            case RED :   RGBregAdr=0x50; break;
            case GREEN : RGBregAdr=0x52; break;
            case BLUE :  RGBregAdr=0x54; break;
            case CLEAR : RGBregAdr=0x56; break;
            
            default : value=-1; break;
        }
        
        err = i2c_readByte(0, SensorAdr, RGBregAdr, &pcLSB);
        err+= i2c_readByte(0, SensorAdr, RGBregAdr+1, &pcMSB);
                
	if(!err){
            value = pcLSB + (pcMSB<<8);
		return value;
	}else{
            printf("BH1745_getRGBvalue() -> Read error\n");
            return -1;
        }
}

// Get the value for selected register on device
int I2C_readDeviceReg(unsigned char deviceAd, unsigned char registerAdr){
    unsigned char err;
    int value=-1;

    err = i2c_readByte(0, deviceAd, registerAdr, &value);
    
    //printf("READ device: %d Register: %d    value: %d   ERROR: %d\n", deviceAd, registerAdr, value, err);
    
    if(!err){
            return value;
    }else{
        printf("I2C_readDeviceReg() -> Read error\n");
        return -1;
    }
}

// Set the value for selected register on device
int I2C_writeDeviceReg(unsigned char deviceAd, unsigned char registerAdr, unsigned char data){
    unsigned char err=0;
    err+=i2c_write(0, deviceAd, registerAdr, data);
    
    if(err){
        printf("I2C_writeDeviceReg() -> Write error\n");
    }
    printf("WRITE device: %d Register: %d    value: %d\n", deviceAd, registerAdr, data);
}
#endif