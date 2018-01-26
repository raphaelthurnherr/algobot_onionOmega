/*
 * hwManager.h
 *
 *  Created on: 26 nov. 2016
 *      Author: raph
 */

#ifndef HWMANAGER_H_
#define HWMANAGER_H_

int InitHwManager(void);						// Ouverture du thread Timer
int CloseHwManager(void);						// Fermeture du thread Timer

int getMotorFrequency(unsigned char motorNb);	// Retourne la fréquence actuelle mesuree sur l'encodeur
int getMotorPulses(unsigned char motorNb);		// Retourne le nombre d'impulsion d'encodeur moteur depuis le démarrage
char getDigitalInput(unsigned char inputNb);	// Retourne l'état de l'entrée numérique spécifiée
int getSonarDistance(void);						// Retourne la distance en cm
int getBatteryVoltage(void);					// Retourne la tension battery en mV

extern int setMotorSpeed(int motorName, int ratio);
extern void setMotorAccelDecel(unsigned char motorNo, char accelPercent, char decelPercent);
extern int setMotorDirection(int motorName, int direction);
extern void checkDCmotorPower(void);				// Fonction temporaire pour rampe d'acceleration
extern unsigned char getMotorPower(unsigned char motorNr);			// Get the actual power of selected motor
extern void setServoPosition(unsigned char smName, unsigned char angle);
extern void setLedPower(unsigned char ledID, unsigned char power);
//extern char getOrganNumber(int organName);		// Retourne le numéro du moteur 0..xx selon le nom d'organe spécifié

int set_i2c_command_queue(int (*callback)(char, int),char adr, int cmd);		//
#endif /* HWMANAGER_H_ */
