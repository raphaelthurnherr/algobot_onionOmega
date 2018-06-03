#include "boardHWctrl.h"
#include <onion-i2c.h>
#include "../buggy_descriptor.h"


unsigned char configPWMdevice(void);                            // Configuration of the PCA9685 for 50Hz operation
unsigned char configGPIOdevice(void);                           // Configuration IO mode of the MCP28003

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
unsigned char motorDCadr[2]={PCA_DCM0, PCA_DCM1};		// Valeur de la puissance moteur

//================================================================================
// BUGGYBOARDINIT
// Initialisation of the board (PWM Driver, GPIO driver, etc..)
//================================================================================

unsigned char buggyBoardInit(void){
	unsigned char err;

	err+=configPWMdevice();					// Configuration du Chip PWM pour gestion de la vï¿½locitï¿½ des DC moteur et angle servomoteur
	err+=configGPIOdevice();				// Confguration du chip d'entrï¿½es/sortie pour la gestion du sens de rotation des moteur DC
	MCP2308_DCmotorState(1);				// Set the HDRIVER ON
	if(err)
		return 0;							// Erreur
	else return 1;
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
	else MCP2308_GPIO_STATE &= 0xEF;					// dï¿½sactivation du driver pont en H

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
// Dï¿½fini le duty cyle ï¿½ appliquer sur les sorties du chip PWM (0..100%)
// motorAdr: Adresse de sortie du contrï¿½leur PWM sur lequel doit ï¿½tre appliquï¿½ le dutyCycle
//================================================================================

void PCA9685_DCmotorSetSpeed(unsigned char motorAdr, unsigned char dutyCycle){
	unsigned int power;
	unsigned char PowerLow;
	unsigned char PowerHigh;

	// Dï¿½fini un dutycylce de maximum 100%
	if(dutyCycle>100)
		dutyCycle=100;

	// Conversion du dutyclycle en valeur ï¿½ appliquer au contrï¿½leur PWM
	power = ((409500/100)*dutyCycle)/100;
	PowerLow = power&0x00FF;;
	PowerHigh = (power&0x0F00) >>8;

	i2c_write(0, PCA9685, motorAdr, PowerLow);
        i2c_write(0, PCA9685, motorAdr+1, PowerHigh);
}


//================================================================================
// DCMOTORSETROTATION
// Dï¿½fini le sense de rottion d'un moteur DC (sens horaire, antihoraire ou stop)
//================================================================================

void MCP2308_DCmotorSetRotation(unsigned char motorAdr, unsigned char direction){
	int MCP2308_GPIO_STATE;

	// Sï¿½lection du chip d'entrï¿½e/sortie qui pilote le pont en H

	i2c_readByte(0, MCP2308, 0x09, &MCP2308_GPIO_STATE);
        
        
        
        // le bit nï¿½nï¿½ssaire
	//	SELECTION DU MOTEUR No 0
	if(motorAdr==PCA_DCM0){
		// Dï¿½sactive la commande du moteur
		// avant de changer de sens de rotation
		MCP2308_GPIO_STATE &= 0xF9;

                i2c_write(0, MCP2308, 0x0A, MCP2308_GPIO_STATE);
		// Sï¿½lï¿½ction du sens de rotation du moteur ou OFF
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

		// Dï¿½sactive la commande du moteur
		// avant de changer de sens de rotation
		MCP2308_GPIO_STATE &= 0xF6;										// Force H-Bridge Off for motor 1

                i2c_write(0, MCP2308, 0x0A, MCP2308_GPIO_STATE);
		// Sï¿½lï¿½ction du sens de rotation du moteur ou OFF
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
// SETSERVOPOS
// Dï¿½fini la position a appliquer au servomoteur
// smAddr = adresse pour le port de sortie concernï¿½ sur le chip PCA9685
// position= Angle de positionnement en degrï¿½ du servomoteur (de 0..100%)
//================================================================================

void PCA9685_setServoPos(unsigned char smAddr, unsigned char position){
	unsigned int dutyCycleValue;
	unsigned char dCLow;
	unsigned char dCHigh;

	// Vï¿½rifie que le positionnement dï¿½fini soit entre 0 et 100%
	if(position>100)
		position=100;

	// Conversion de la position 0..100% selon le fonctionnement du servo moteur
	// Durï¿½e de pulse de minium 0.5mS et maximum 2.5mS
	dutyCycleValue = 205+(position*2.04);
	dCLow = dutyCycleValue&0x00FF;;
	dCHigh = (dutyCycleValue&0x0F00) >>8;

//	Applique les nouvelles valeures

         i2c_write(0, PCA9685, smAddr, dCLow);

        i2c_write(0, PCA9685, smAddr+1, dCHigh);
}

//================================================================================
// SETLEDPOWER
// Dï¿½fini l'intensitï¿½ d'ï¿½clairage pour led @ 50HZ selon config PCA9685
// smAddr = adresse pour le port de sortie concernï¿½ sur le chip PCA9685
// power = Intensitï¿½ d'ï¿½clairage ( 0..100%)
//================================================================================

void PCA9685_setLedPower(unsigned char smAddr, unsigned char power){
	unsigned int dutyCycleValue;
	unsigned char dCLow;
	unsigned char dCHigh;

//	i2cSelectSlave(PCA9685);								// Sï¿½lï¿½ction du chip PWM

	// Vï¿½rifie que la puissance dï¿½finie soit entre 0 et 100%
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
//	- Sï¿½lï¿½ction horloge interne 25MHz
//	- Mode opï¿½ration ï¿½ 50Hz (Principalement pour la commande de servomoteurs)
//	- Sorties non inversï¿½es
//	- Ps d'auto incrementation
//================================================================================
unsigned char configPWMdevice(void){
	unsigned char err;
//	err=i2cSelectSlave(PCA9685);

	// Registre MODE1, sleep before config, horloge interne ï¿½ 25MHz

        i2c_write(0, PCA9685, 0x00, 0x10);
        
	// Prescaler pour opï¿½ration 50Hz

        i2c_write(0, PCA9685, 0xFE, 0x81);

	// Registre MODE 2, sorties non inversï¿½es

        i2c_write(0, PCA9685, 0x01, 0x04);
        
	// TOUTES LED ON au clock 0

        i2c_write(0, PCA9685, 0xFA, 0x00);
       
        i2c_write(0, PCA9685, 0xFB, 0x00);

	// MODE 1, Systï¿½me prï¿½t,

        i2c_write(0, PCA9685, 0x00, 0x81);


	return err;
}


//================================================================================
// CONFIGGPIODEVICE
// Configuration initiale pour le GPIO Controleur MCP2308
//	- Non auto-incrementation
//	- Pull-up activï¿½e
//	- Pin en sortie
//================================================================================
unsigned char configGPIOdevice(void){
	unsigned char err;

//	err=i2cSelectSlave(MCP2308);

	// Pas de auto-incrementation
        i2c_write(0, MCP2308, 0x05, 0x20);
	// Pull up activee
        i2c_write(0, MCP2308, 0x06, 0xFF);
	// GPIO 0..4 en sorties, GPIO 5..6 en entree pour boutons
        i2c_write(0, MCP2308, 0x00, 0x60);
	return err;
}






// -------------------------------------------------------------------
// GETSONARDISTANCE
// Lecture de la distance mesuree au sonar [mm]
// Retourn une valeures positve correspondant ï¿½ la distance en mm
// ou -1 si erreur de lecture
// -------------------------------------------------------------------
int EFM8BB_readSonarDistance(void){
	unsigned char err;
	unsigned int SonarDistance_mm;
        unsigned int mmMSB;
        unsigned int mmLSB;
        
//	err=i2cSelectSlave(EFM8BB);						

	SonarDistance_mm=0;							// RAZ de la variable distance

	if(!err){
//		SonarDistance_mm=i2cReadByte(20);
                i2c_readByte(0, EFM8BB, SON0, &mmLSB);
//		SonarDistance_mm+=(i2cReadByte(21)<<8);
                i2c_readByte(0, EFM8BB, SON0+1, &mmMSB);
                SonarDistance_mm=mmLSB + (mmMSB<<8);
		//printf("MSB: %2x %2x", );
                return SonarDistance_mm;
	}else return -1;
}


// -------------------------------------------------------------------
// GETBATTERYVOLTAGE
// Lecture de la tension batterie mesuree en mV
// Retourne une valeures positve correspondant ï¿½ la tension en mV
// ou -1 si erreur de lecture
// -------------------------------------------------------------------
int EFM8BB_readBatteryVoltage(void){
	unsigned char err;
	unsigned int batteryVoltage_mV;
        unsigned int mVMSB;
        unsigned int mVLSB;

	batteryVoltage_mV=0;							// RAZ de la variable

	if(!err){
                i2c_readByte(0, EFM8BB, VOLT0, &mVLSB);
                i2c_readByte(0, EFM8BB, VOLT0+1, &mVMSB);
                batteryVoltage_mV=mVLSB + (mVMSB<<8);
                
//                printf("VOLTAGE: %d mV\n", batteryVoltage_mV);
		return batteryVoltage_mV;
	}else return -1;
}

// -------------------------------------------------------------------
// GETFREQUENCY
// Get frequency measured on EFM8BB
// ou -1 si erreur de lecture
// -------------------------------------------------------------------
int EFM8BB_readFrequency(unsigned char wheelNb){
	unsigned char err, regAddr;
	unsigned int freq;

	if(wheelNb==0) regAddr = ENC_FREQ0;
	else regAddr = ENC_FREQ1;

	freq=0;							// RAZ de la variable

	if(!err){
                i2c_readByte(0, EFM8BB, regAddr, &freq);
		return freq;
	}else return -1;
}

// -------------------------------------------------------------------
// GETPULSECOUNTER
// Get pulse counter on EFM8BB
// ou -1 si erreur de lecture
// -------------------------------------------------------------------
int EFM8BB_readPulseCounter(unsigned char wheelNb){
	unsigned char err=0, regAddr=0;
	unsigned int pulseCount;
        unsigned int pcMSB=0;
        unsigned int pcLSB=0;

	if(wheelNb==0) {
		regAddr = ENC_CNT0;
	}
	else {
		regAddr = ENC_CNT1;
	}

	pulseCount=0;							// RAZ de la variable

        err+=i2c_readByte(0, EFM8BB, regAddr, &pcLSB);
        err+=i2c_readByte(0, EFM8BB, regAddr+1, &pcMSB);
        
        pulseCount=pcLSB + (pcMSB<<8);
                
	if(!err){
		return pulseCount;
	}else return -1;
}

// -------------------------------------------------------------------
// CLEARWHEELDISTANCE
// RetourReset to 0 the pulse counter on EFM8BB
// ou -1 si erreur de lecture
// -------------------------------------------------------------------
int EFM8BB_clearWheelDistance(unsigned char wheelNb){
	unsigned char err, regAddr;
	unsigned int pulseCount;

	if(wheelNb==0) {
		regAddr = ENC_CNT0_RESET;
	}
	else {
		regAddr = ENC_CNT1_RESET;
	}

	pulseCount=0;							// RAZ de la variable

	if(!err){
		//pulseCount=(i2cReadByte(regAddr));
                i2c_readByte(0, EFM8BB, regAddr, &pulseCount);
		return pulseCount;
	}else return -1;
}

// -------------------------------------------------------------------
// GETDIGITALINPUT
// Mesure de l'etat des entrees digitale
// Paramètre "InputNr" plus utilisé...
// -------------------------------------------------------------------
char EFM8BB_readDigitalInput(unsigned char InputNr){
	unsigned char err;
	char inputState=0;
        
        err = i2c_readByte(0, EFM8BB, DIN_REG, &inputState);

	if(!err){
		return inputState;
	}else return -1;
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
	}else return -1;
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
	}else return -1;
}


