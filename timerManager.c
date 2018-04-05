/*
 * timerManager.c
 *
 *  Created on: 8 avr. 2016
 *      Author: raph
 */

// Defninition des emplacement dans les variables timer avec callback

#define ACTIONID 0					// Slot action concerné
#define NAME	 1					// Slot Data pour No de roue
#define PTRFUNC  2					// Slot, Fonction callback
#define STOPTIME 3					// Slot, Stoptime pour callback
#define TYPE	 4					// Slot Data pour No de LED

#include "pthread.h"
#include <unistd.h>
#include <stdio.h>
#include "timerManager.h"

// Thread Messager
pthread_t th_timers;

int myTimer[10][5];					// Données des timer callback
int timerDataBackup[5];

int timeNow = 0;					// Variable de comptage de temp actuel pour les timers avec callback
unsigned char checkMotorPowerFlag;
unsigned char t100msFlag;
unsigned char t10secFlag;


// ------------------------------------------
// Programme principale TIMER
// ------------------------------------------
void *TimerTask (void * arg){
	int i;
	unsigned int cyclicTimer50ms;	// Compteur du timer cyclique 50mS
	unsigned int cyclicTimer100ms;	// Compteur du timer cyclique 100mS
	unsigned int cyclicTimer10sec;	// Compteur du timer cyclique 10Secondes
	unsigned int endTimerValues[5]; // Memorisation des data du timer
	while(1){

		// Controle successivement les timers pour la gestion du temps de
		// fonctionnement des roues. Une fois le timeout atteind, appelle la fonction call-back
		// de l'action à effectuer
		for(i=0;i<10;i++){
			if(myTimer[i][STOPTIME]!=0){						// Timer Actif (!=0), Ne provoque pas d'action si timer inactif
				if(timeNow >= myTimer[i][STOPTIME]){			// Fin du timer ?

					// Memorise les data timer pour appelle de fonction callback
					timerDataBackup[ACTIONID]=myTimer[i][ACTIONID];
					timerDataBackup[NAME]=myTimer[i][NAME];
                                        timerDataBackup[TYPE]=myTimer[i][TYPE];
					timerDataBackup[PTRFUNC]=myTimer[i][PTRFUNC];

					// Libère l'espace de l'action terminée si pas de "reload"
					myTimer[i][STOPTIME] = myTimer[i][PTRFUNC] = myTimer[i][ACTIONID] = 0;
					myTimer[i][NAME]=-1;

					onTimeOut(timerDataBackup[PTRFUNC], timerDataBackup[ACTIONID],timerDataBackup[NAME]);	// Appelle la fonction callback à la fin du timer
				}
			}
		}

		// Controle le time out de 50ms
		if(cyclicTimer50ms>=50){
			checkMotorPowerFlag=1;
			cyclicTimer50ms=0;				// Reset le compteur 50ms
		}

		// Controle le time out de 100ms
		if(cyclicTimer100ms>=100){
			t100msFlag=1;
			cyclicTimer100ms=0;				// Reset le compteur 100ms
		}

		// Controle le time out de 10 secondes
		if(cyclicTimer10sec>=10000){
			t10secFlag=1;
			cyclicTimer10sec=0;				// Reset le compteur 10secondes
		}

		cyclicTimer50ms++;				// Reset le compteur 10secondes
		cyclicTimer100ms++;				// Reset le compteur 10secondes
		cyclicTimer10sec++;				// Reset le compteur 10secondes
		timeNow++;
		usleep(1000);
	}
	pthread_exit (0);
}

// ------------------------------------------------------------------------------------
// TIMERMANAGER: Initialisation du gestionnaire de timer
// - Démarre le thread
// ------------------------------------------------------------------------------------
int InitTimerManager(void){
	// CREATION DU THREAD DE TIMER
	  if (pthread_create (&th_timers, NULL, TimerTask, NULL)!=0) {
		return (1);
	  }else return (0);
}

// ------------------------------------------------------------------------------------
// CLOSETIMER: Fermeture du gestionnaire de timers
// - Stop le thread timers
// ------------------------------------------------------------------------------------

int CloseTimerManager(void){
	int result;
	// TERMINE LE THREAD DE MESSAGERIE
	pthread_cancel(th_timers);
	// Attends la terminaison du thread de messagerie
	result=pthread_join(th_timers, NULL);
	return (result);
}

// ------------------------------------------------------------------------------------
// SETTIMER: Paramètrage d'un timer pour le fonctionnement d' une roue selon en temps donné
// time_ms: Durée de fonctionnement
// *callback: Fonction callback à appeler à la fin du timer
// actionNumber: Numéro d'action à attribuer
// wheelName: No de la roue concernée par le fonctionnement
// ------------------------------------------------------------------------------------
int setTimer(int time_ms, int (*callback)(int, int),int actionNumber, int name, int type){

	int i;
	int timerIsSet=0;
	int setTimerResult=0;

	// Recherche un emplacement libre pour inserer les données du timer
	// Ecrase le timer si nouvelle consigne pour la roue
	//|| (wheelName == myTimer[i][WHEEL])

	i=0;

	while((i<10) && (!timerIsSet)){
                           
		if(name == myTimer[i][NAME]){
                    if(type == myTimer[i][TYPE]){
			printf("Annulation du timer en cours pour l'action %d : %d\n", myTimer[i][ACTIONID], name);
			setTimerResult=myTimer[i][ACTIONID];						// Retourne le numéro d'action ecrassé
			myTimer[i][ACTIONID]=0;								// Libère l'emplacement memoire du timer ecrasé
                    }
		}

                                
		if(myTimer[i][ACTIONID]<=0){
			myTimer[i][STOPTIME] = timeNow + time_ms;					// Ajoute le temps donné au compteur actuel
			myTimer[i][PTRFUNC]=callback;							// memorisation de la fonction callback de fin de timer
			myTimer[i][NAME]=name;								// memorisation de la donnée concernée par l'action(roue)
			myTimer[i][TYPE]=type;								// memorisation de la donnée concernée par l'action(roue)
                        myTimer[i][ACTIONID]=actionNumber;						// Memorise le no d'action
			if(!setTimerResult)setTimerResult=1;						// Retourne OK, si pas d'action écrasé
			timerIsSet=1;
		}

		i++;
	}

	return setTimerResult;
}



// ------------------------------------------------------------------------------------
// ONTIMEOUT: Fcontion appelee en fin de timer
// appelle une fonction callback prédéfinie par *ptrFunc
// ------------------------------------------------------------------------------------
void onTimeOut(void (*ptrFunc)(int, int),int actionNumber, int name){
	(*ptrFunc)(actionNumber, name);		// Appelle de la fonction call back prédéfinie par *ptrFonc avec les paramètre recus
}



