/*
 * hwManager.h
 *
 *  Created on: 26 nov. 2016
 *      Author: raph
 */

#ifndef HWMANAGER_H_
#define HWMANAGER_H_

// Definition des couleurs pour le capteur RGB
    #define RED                                 0
    #define GREEN                               1
    #define BLUE                                2
    #define CLEAR                               3

int InitHwManager(void);						// Ouverture du thread Timer
int CloseHwManager(void);						// Fermeture du thread Timer

int set_i2c_command_queue(int (*callback)(char, int),char adr, int cmd); // Ajout la commande I2C dans la file

int getMotorFrequency(unsigned char motorNb);	// Retourne la fr�quence actuelle mesuree sur l'encodeur
int getMotorPulses(unsigned char motorNb);		// Retourne le nombre d'impulsion d'encodeur moteur depuis le d�marrage
char getDigitalInput(unsigned char inputNb);	// Retourne l'�tat de l'entr�e num�rique sp�cifi�e
int getSonarDistance(void);						// Retourne la distance en cm
int getBatteryVoltage(void);					// Retourne la tension battery en mV
char getButtonInput(unsigned char buttonNumber);
int getColorValue(unsigned char sensorID, unsigned char color);      // Retourne la valeur de la couleur d�finie sur le capteur d�fini

extern int setMotorSpeed(int motorName, int ratio);
extern int setMotorDirection(int motorName, int direction);

extern void setServoPosition(unsigned char smName, char position);
extern void setLedPower(unsigned char ledID, unsigned char power);
extern void setPwmPower(unsigned char ID, unsigned char power);

extern int setStepperStepAction(int motorNumber, int direction, int stepCount);      // Effectue une action sur le moteur pas à pas (direction, nombre de pas)
extern int setStepperSpeed(int motorNumber, int speed);     // Configuration de la vitesse du moteur pas à pas

extern int resetHardware(t_sysConfig * Config);                         // Reset les peripherique hardware (Moteur, LED, PWM, etc...)

//extern char getOrganNumber(int organName);		// Retourne le num�ro du moteur 0..xx selon le nom d'organe sp�cifi�

extern int getHWInfo(char* MCUversion, char* HWtype); // Get the hardware/software version
#endif /* HWMANAGER_H_ */
