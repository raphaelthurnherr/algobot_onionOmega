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
int setTimer(int time_ms, int (*callback)(int, int), int actionNumber, int name, int type);		// Param�trage d'un timer pour																						// le temps de fonctionnement d'une roue
extern unsigned char checkMotorPowerFlag;		// A MODIFIER PAR UN TEMPS
extern unsigned char t100msFlag;				// Flag 100mS scrut� par le programme principal
extern unsigned char t10secFlag;				// Flag 10S scrut� par le programme principal
extern unsigned char t60secFlag;				// Flag 60Sec scrut� par le programme principal

#endif /* TIMERMANAGER_H_ */