#ifndef BOARDHWCTRL_H_
#define BOARDHWCTRL_H_

// Device addresses (7 bits, lsb is a don't care)

#define  MCP2308               	0x20	// Device address for GPIO controller
#define  EFM8BB               	0x0A	// Device address for EFM8BB microcontroller
#define  PCA9685               	0x40 	// Device address for PWM controller

#define PCA_PWM_ALL_ADR			0xFC	// PCA9685 all call address

#define PCA_DCM0				0x08	// PCA9685 Output 0 address (Motor 0 speed pwm)
#define PCA_DCM1				0x0C	// PCA9685 Output 1 address (Motor 1 speed pwm)

#define PCA_LED0				0x10	// PCA9685 Output 2 address (Led 0 pwm)
#define PCA_LED1				0x14	// PCA9685 Output 3 address (Led 1 pwm)
#define PCA_LED2				0x18	// PCA9685 Output 4 address (Led 2 pwm)

#define PCA_SRM0				0x3C	// PCA9685 Output 13 address (Servomotor 0 pwm)
#define PCA_SRM1				0x40	// PCA9685 Output 14 address (Servomotor 1 pwm)
#define PCA_SRM2				0x44	// PCA9685 Output 15 address (Servomotor 2 pwm)

#define MCW 			0
#define MCCW			1
#define MSTOP 			2

extern unsigned char buggyBoardInit(void);
extern unsigned char motorDCadr[2];			// Valeur de la puissance moteur
extern void MCP2308_DCmotorState(unsigned char state);
extern void MCP2308_DCmotorSetRotation(unsigned char motorAdr, unsigned char direction);
extern void PCA9685_DCmotorSetSpeed(unsigned char motorAdr, unsigned char dutyCycle);
extern void PCA9685_setServoPos(unsigned char smAddr, unsigned char position);
extern void PCA9685_setLedPower(unsigned char smAddr, unsigned char power);
//extern void setDCmotorPower(unsigned char motorAdr, unsigned char power);

extern int EFM8BB_readSonarDistance(void);							// Get distance in mm from the EFM8BB microcontroller
extern char EFM8BB_readDigitalInput(unsigned char InputNr);			// Get digital input state in mm from the EFM8BB microcontroller
extern int EFM8BB_readBatteryVoltage(void);							// Get the battery voltage in mV from EFM8BB microcontroller
extern int EFM8BB_readFrequency(unsigned char wheelNb);				// Get frequency measured on EFM8BB
extern int EFM8BB_readPulseCounter(unsigned char wheelNb);			// Get pulse counter on EFM8BB
extern int EFM8BB_clearWheelDistance(unsigned char wheelNb);		// Reset to 0 the pulse counter on EFM8BB

#endif
