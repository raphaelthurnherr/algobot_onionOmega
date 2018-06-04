/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../buggy_descriptor.h"
#include "../timerManager.h"
#include "linux_json.h"
#include "../algobot_main.h"
#include "asyncTools.h"

char reportBuffer[256];

int setAsyncMotorAction(int actionNumber, int motorNb, int veloc, char unit, int value);
int endWheelAction(int actionNumber, int motorNb);
int checkMotorEncoder(int actionNumber, int encoderName);
int dummyMotorAction(int actionNumber, int encoderName);
// -------------------------------------------------------------------
// SETASYNCMOTORLACTION
// Effectue l'action sur une roue spécifiée
// - Démarrage du timer avec definition de fonction call-back, et no d'action
// - Démarrage du mouvement de la roue spécifiée
// - Vélocité entre -100 et +100 qui défini le sens de rotation du moteur
// -------------------------------------------------------------------

int setAsyncMotorAction(int actionNumber, int motorNb, int veloc, char unit, int value){
	int myDirection;
	int setTimerResult;
	int endOfTask;

	// Conversion de la vélocité de -100...+100 en direction AVANCE ou RECULE
	if(veloc > 0){
		myDirection=BUGGY_FORWARD;
		body.motor[motorNb].direction = 1;
	}
	if(veloc == 0){
		myDirection=BUGGY_STOP;
		body.motor[motorNb].direction = 0;
	}
	if(veloc < 0){
		myDirection=BUGGY_BACK;
		body.motor[motorNb].direction = -1;
		veloc *=-1;					// Convertion en valeur positive
	}

	// Démarre de timer d'action sur la roue et spécifie la fonction call back à appeler en time-out
	// Valeur en retour >0 signifie que l'action "en retour" à été écrasée
	switch(unit){
		case  MILLISECOND:  setTimerResult=setTimer(value, &endWheelAction, actionNumber, motorNb, MOTOR); break;
		case  CENTIMETER:   //motorNb = getOrganNumber(motorNb);
                                    body.encoder[motorNb].startEncoderValue=getMotorPulses(motorNb)*CMPP;
                                    body.encoder[motorNb].stopEncoderValue = body.encoder[motorNb].startEncoderValue+ value;
                                    setTimerResult=setTimer(50, &checkMotorEncoder, actionNumber, motorNb, MOTOR); break;// Démarre un timer pour contrôle de distance chaque 35mS
                                   
                case  INFINITE:     setTimerResult=setTimer(100, &dummyMotorAction, actionNumber, motorNb, MOTOR); break;
		default: printf("\n!!! ERROR Function [setAsyncMotorAction] -> unknown mode");break;
	}

	if(setTimerResult!=0){						// Timer pret, action effectuée ()
		if(setTimerResult>1){					// Le timer à été écrasé par la nouvelle action en retour car sur la même roue
			endOfTask=removeBuggyTask(setTimerResult);	// Supprime l'ancienne tâche qui à été écrasée par la nouvelle action
			if(endOfTask){
				sprintf(reportBuffer, "Annulation des actions moteur pour la tache #%d\n", endOfTask);

				// Récupère l'expediteur original du message ayant provoqué
				// l'évenement
				char msgTo[32];
				int ptr=getSenderFromMsgId(endOfTask);
				strcpy(msgTo, msgEventHeader[ptr].msgFrom);
				// Libère la memorisation de l'expediteur
				removeSenderOfMsgId(endOfTask);

				AlgoidResponse[0].responseType=2;
				sendResponse(endOfTask, AlgoidCommand.msgFrom, EVENT, MOTORS, 1);			// Envoie un message ALGOID de fin de tâche pour l'action écrasé
				printf(reportBuffer);									// Affichage du message dans le shell
				sendMqttReport(endOfTask, reportBuffer);                                                // Envoie le message sur le canal MQTT "Report"
			}

		}

		// Défini le "nouveau" sens de rotation à applique au moteur ainsi que la consigne de vitesse
		if(setMotorDirection(motorNb, myDirection)){							// Sens de rotation
			setMotorSpeed(motorNb, veloc);									// Vitesse

			// Envoie de message ALGOID et SHELL
			sprintf(reportBuffer, "Start wheel %d with velocity %d for time %d\n", motorNb, veloc, value);
			printf(reportBuffer);
			sendMqttReport(actionNumber, reportBuffer);
		}
		else{
			sprintf(reportBuffer, "Error, impossible to start wheel %d\n",motorNb);
			printf(reportBuffer);
			sendMqttReport(actionNumber, reportBuffer);
		}

	}
	else printf("Error, Impossible to set timer wheel\n");
	return 0;
}

// -------------------------------------------------------------------
// END2WDACTION
// Fin de l'action sur une roue
// Fonction appelée après le timout défini par l'utilisateur, Stop le moteur spécifié
// -------------------------------------------------------------------
int endWheelAction(int actionNumber, int motorNb){
	int endOfTask;
	//printf("Action number: %d - End of timer for wheel No: %d\n",actionNumber , motorNb);

	// Stop le moteur
	setMotorSpeed(motorNb, 0);

	// Retire l'action de la table et vérification si toute les actions sont effectuées
	// Pour la tâche en cours donnée par le message ALGOID

	endOfTask = removeBuggyTask(actionNumber);

	// Contrôle que toutes les actions ont été effectuée pour la commande recue dans le message ALGOID
	if(endOfTask){
		// Récupère l'expediteur original du message ayant provoqué
		// l'évenement
		char msgTo[32];
		int ptr=getSenderFromMsgId(endOfTask);
		strcpy(msgTo, msgEventHeader[ptr].msgFrom);
		// Libère la memorisation de l'expediteur
		removeSenderOfMsgId(endOfTask);

		// Récupère la distance et la vitesse actuelle du moteur
		// Pour retour event a l'expediteur originel
/*		int i;
		for(i=0;i<NBMOTOR;i++){
			AlgoidResponse[i].MOTresponse.id=i;
			AlgoidResponse[i].MOTresponse.speed=body.motor[i].speed;
			AlgoidResponse[i].MOTresponse.distance=body.motor[i].distance;
			AlgoidResponse[i].MOTresponse.time=9999;
		}
*/
		AlgoidResponse[0].responseType=0;

		sendResponse(endOfTask, msgTo, EVENT, MOTORS, 1);
		sprintf(reportBuffer, "FIN DES ACTIONS \"WHEEL\" pour la tache #%d\n", endOfTask);
		printf(reportBuffer);
		sendMqttReport(endOfTask, reportBuffer);
	}

	return 0;
}


// ----------------------------------------------------------------------
// CHECKMOTORENCODER
// Contrôle la distance parcourue et stop la roue si destination atteinte
// Fonction appelée après le timout défini par l'utilisateur.
// -----------------------------------------------------------------------

int checkMotorEncoder(int actionNumber, int encoderName){
	float distance;					// Variable de distance parcourue depuis le start
	//unsigned char encoderNumber;

	//encoderNumber = getOrganNumber(encoderName);

	distance = getMotorPulses(encoderName);

	if(distance >=0){
		distance = (distance*CMPP);
    	usleep(2200);
	}else  printf("\n ERROR: I2CBUS READ\n");
	//printf("\n Encodeur #%d -> START %.2f cm  STOP %.2f cm", encoderNumber, startEncoderValue[encoderNumber], stopEncoderValue[encoderNumber]);

	if(distance >= body.encoder[encoderName].stopEncoderValue)
		endWheelAction(actionNumber, encoderName);
	else
		setTimer(50, &checkMotorEncoder, actionNumber, encoderName, MOTOR);

	return 0;
}

int dummyMotorAction(int actionNumber, int encoderName){
        setTimer(0, &dummyMotorAction, actionNumber, encoderName, MOTOR);
    return 0;
}