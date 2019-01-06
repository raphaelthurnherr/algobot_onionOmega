#ifndef BOARDHWCTRL_H_
#define BOARDHWCTRL_H_

#define BOARD_REV2

// Device addresses (7 bits, lsb is a don't care)
#ifdef BOARD_REV2
    #define  PCA9629               	0x20	// Device address for Step motor Driver
    #define  MCP2308               	0x21	// Device address for GPIO controller
    #define  EFM8BB               	0x0A	// Device address for EFM8BB microcontroller
    #define  BH1745_0               	0x38	// Device address for RGB SENSOR
    #define  BH1745_1               	0x39	// Device address for RGB SENSOR
    #define  PCA9685               	0x40 	// Device address for PWM controller

    // REGISTER DEFINITION FOR PWM DRIVER

    #define PCA_PWM_ALL_ADR			0xFC	// PCA9685 all call address

    #define PCA_DCM0				0x08	// PCA9685 Output 0 address (Motor 0 speed pwm)
    #define PCA_DCM1				0x0C	// PCA9685 Output 1 address (Motor 1 speed pwm)

    #define PCA_LED0				0x2C	//  (Led 0 pwm)
    #define PCA_LED1				0x38	//  (Led 1 pwm)
    #define PCA_LED2				0x40	//  (Led 2 pwm)
  
    #define PCA_CN1_4				0x10	//  (PWM on CN1 connector)
    #define PCA_CN2_4				0x14	//  (PWM on CN2 connector)
    #define PCA_CN3_7				0x1C	//  (PWM on CN14 connector)
    #define PCA_CN4_4                           0x20    //  (PWM on CN15 connector)
    #define PCA_CN7_3				0x44	//  (PWM on CN8 connector)
    #define PCA_CN8_3				0x28	//  (PWM on CN7 connector)
    #define PCA_CN9_7				0x24	//  (PWM on CN5 connector)
    #define PCA_CN10_4				0x3C	//  (PWM on CN4 connector)
    #define PCA_CN11_3				0x30	//  (PWM on CN9 connector)
    #define PCA_CN12_3				0x34	//  (PWM on CN10 connector)

    // REGISTER DEFINITION FOR EFM8 MICROCONTROLLER
    
    #define FIRMWARE_REG                        0x01
    #define BOARDTYPE_REG                       0x00
    #define DIN_REG                             0x04
    #define SON0                                0x08
    #define VOLT0                               0x0E
    #define ENC_FREQ0                           0x13
    #define ENC_CNT1_RESET                      0x24
    #define ENC_CNT1                            0x14    
    #define ENC_FREQ1                           0x17
    #define ENC_CNT0_RESET                      0x28
    #define ENC_CNT0                            0x18

    #define MCW                                 0
    #define MCCW                                1
    #define MSTOP                               2

// Definition des couleurs pour le capteur RGB
    #define RED                                 0
    #define GREEN                               1
    #define BLUE                                2
    #define CLEAR                               3

#endif

extern unsigned char buggyBoardInit(void);
extern unsigned char motorDCadr[2];                                             // Valeur de la puissance moteur

extern void MCP2308_DCmotorState(unsigned char state);
extern void MCP2308_DCmotorSetRotation(unsigned char motorAdr, unsigned char direction);
extern char MCP2308_ReadGPIO(unsigned char input);   // Lecture d'un GPIO sur le peripherique 
extern void PCA9685_DCmotorSetSpeed(unsigned char motorAdr, unsigned char dutyCycle);
extern void PCA9685_setServoPos(unsigned char smAddr, char position);
extern void PCA9685_setLedPower(unsigned char smAddr, unsigned char power);

extern int PCA9629_StepperMotorControl(int motorNumber, int data);              //Configuration du registre "PAS" du driver moteur
extern int PCA9629_StepperMotorSetStep(int motorNumber, int stepCount);         //Configuration du registre "PAS" du driver moteur
extern int PCA9629_StepperMotorMode(int motorNumber, int data);                 // Mode action continue ou unique
extern int PCA9629_StepperMotorPulseWidth(int motorNumber, int data);           // Registre de configuration de la largeur d'impulsion moteur pour les sens CW et CCW
extern int PCA9629_ReadMotorState(int motorNumber);                             // Lecture du registre de contrôle du moteur
//extern void setDCmotorPower(unsigned char motorAdr, unsigned char power);

extern int EFM8BB_readSonarDistance(void);					// Get distance in mm from the EFM8BB microcontroller
extern char EFM8BB_readDigitalInput(unsigned char InputNr);			// Get digital input state in mm from the EFM8BB microcontroller
extern int EFM8BB_readBatteryVoltage(void);					// Get the battery voltage in mV from EFM8BB microcontroller
extern int EFM8BB_readFrequency(unsigned char wheelNb);				// Get frequency measured on EFM8BB
extern int EFM8BB_readPulseCounter(unsigned char wheelNb);			// Get pulse counter on EFM8BB
extern int EFM8BB_clearWheelDistance(unsigned char wheelNb);                    // Reset to 0 the pulse counter on EFM8BB
extern int EFM8BB_getFirmwareVersion(void);                                     // Get the MCU firmware version
extern int EFM8BB_getBoardType(void);                                           // Get the type of the board.
extern int BH1745_getRGBvalue(unsigned char sensorNb, int color);                              // Get the value for specified color
extern int I2C_readDeviceReg(unsigned char deviceAd, unsigned char registerAdr);    // Get the value for selected register on device
extern int I2C_writeDeviceReg(unsigned char deviceAd, unsigned char registerAdr, unsigned char data);    // Get the value for selected register on device
#endif
