/*
 * timerManager.h
 *
 *  Created on: 8 avr. 2016
 *      Author: raph
 */

#ifndef TIMERMANAGER_H_
#define TIMERMANAGER_H_

int InitTimerManager(void);						// Ouverture du thread Timer
int CloseTimerManager(void);					// Fermeture du thread Timer
int setTimerWheel(int time_ms, int (*callback)(int, int), int actionNumber, int wheelName);		// Param�trage d'un timer pour
																								// le temps de fonctionnement d'une roue

extern unsigned char checkMotorPowerFlag;		// A MODIFIER PAR UN TEMPS
extern unsigned char t100msFlag;				// Flag 100mS scrut� par le programme principal
extern unsigned char t10secFlag;				// Flag 10mS scrut� par le programme principal
#endif /* TIMERMANAGER_H_ */
