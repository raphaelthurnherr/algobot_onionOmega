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

int setAsyncStepperAction(int actionNumber, int motorNb, int veloc, char unit, int value);
int endStepperAction(int actionNumber, int motorNb);
int checkStepperStatus(int actionNumber, int encoderName);
int dummyStepperAction(int actionNumber, int encoderName);
// -------------------------------------------------------------------
// SETASYNCSTEPPERACTION
// Effectue l'action sur le moteur pas à pas spécifié
// - Démarrage du timer avec definition de fonction call-back, et no d'action
// - Démarrage du mouvement de la roue spécifiée
// - Vélocité entre -100 et +100 qui défini le sens de rotation du moteur
// -------------------------------------------------------------------

int setAsyncStepperAction(int actionNumber, int motorNb, int veloc, char unit, int value){
	int myDirection;
	int setTimerResult;
	int endOfTask;           
        
	if(veloc == 0){
		myDirection=BUGGY_STOP;
		body.stepper[motorNb].direction = 0;
	}else
        {
            if(veloc < 0){
                // Check if motor inversion requiered and modify if necessary
                if(!sysConfig.stepper[motorNb].inverted) myDirection=BUGGY_FORWARD;
                else myDirection=BUGGY_BACK;
                body.stepper[motorNb].direction = -1;
                veloc *=-1;					// Convertion en valeur positive
            }else{      
                // Check if motor inversion requiered and modify if necessary
                if(!sysConfig.stepper[motorNb].inverted) myDirection=BUGGY_BACK;
                else myDirection=BUGGY_FORWARD;
            body.stepper[motorNb].direction = 1;
            }            
        }


	// D�marre de timer d'action sur la roue et sp�cifie la fonction call back � appeler en time-out
	// Valeur en retour >0 signifie que l'action "en retour" � �t� �cras�e
	switch(unit){
		case  MILLISECOND:  setTimerResult=setTimer(value, &endStepperAction, actionNumber, motorNb, STEPMOTOR); break;
		case  STEP:  setTimerResult=setTimer(50, &checkStepperStatus, actionNumber, motorNb, STEPMOTOR); break;         // Contrôle periodique du status du moteur pas à pas
//                case  ANGLE:  setTimerResult=setTimer(value, &endStepperAction, actionNumber, motorNb, STEPMOTOR); break;
//		case  ROTATION:  setTimerResult=setTimer(value, &endStepperAction, actionNumber, motorNb, STEPMOTOR); break;                
//		case  CENTIMETER:   //motorNb = getOrganNumber(motorNb);
//                                    body.encoder[motorNb].startEncoderValue=getMotorPulses(motorNb)*CMPP;
//                                    body.encoder[motorNb].stopEncoderValue = body.encoder[motorNb].startEncoderValue+ value;
//                                    setTimerResult=setTimer(50, &checkMotorEncoder, actionNumber, motorNb, STEPMOTOR); break;// D�marre un timer pour contr�le de distance chaque 35mS
                                   
                case  INFINITE:     setTimerResult=setTimer(500, &dummyStepperAction, actionNumber, motorNb, STEPMOTOR); break;
		default: printf("\n!!! ERROR Function [setAsyncStepperAction] -> unknown mode");break;
	}

	if(setTimerResult!=0){						// Timer pret, action effectuée ()
		if(setTimerResult>1){					// Le timer est écrasé par la nouvelle action en retour car sur la même roue
			endOfTask=removeBuggyTask(setTimerResult);	// Supprime l'ancienne tâche qui est écrasée par la nouvelle action
			if(endOfTask){
				sprintf(reportBuffer, "Annulation des actions moteur pas à pas pour la tache #%d\n", endOfTask);

				// Récupère l'expediteur original du message ayant provoqu�
				// l'évenement
				char msgTo[32];
				int ptr=getSenderFromMsgId(endOfTask);
				strcpy(msgTo, msgEventHeader[ptr].msgFrom);
				// Libère la memorisation de l'expediteur
				removeSenderOfMsgId(endOfTask);

				AlgoidResponse[0].responseType=EVENT_ACTION_ABORT;
				sendResponse(endOfTask, AlgoidCommand.msgFrom, EVENT, STEPPER, 1);			// Envoie un message ALGOID de fin de t�che pour l'action �cras�
				printf(reportBuffer);									// Affichage du message dans le shell
				sendMqttReport(endOfTask, reportBuffer);                                                // Envoie le message sur le canal MQTT "Report"
			}
		}
                
		// Défini le "nouveau" sens de rotation, applique au moteur ainsi que la consigne de vitesse
		if(setStepperSpeed(motorNb, veloc)){
//			setMotorSpeed(motorNb, veloc);									// Vitesse
                        setStepperStepAction(motorNb, myDirection, value);
                                
			// Envoie de message ALGOID et SHELL
			sprintf(reportBuffer, "Start step motor %d with velocity %d for time %d\n", motorNb, veloc, value);
			printf(reportBuffer);
			sendMqttReport(actionNumber, reportBuffer);
		}
		else{
			sprintf(reportBuffer, "Error, impossible to start stepper motor %d\n",motorNb);
			printf(reportBuffer);
			sendMqttReport(actionNumber, reportBuffer);
		}

	}
	else printf("Error, Impossible to set timer for stepper motor\n");
	return 0;
}

// -------------------------------------------------------------------
// ENDSTEPPERACTION
// Fin de l'action sur un moteur aps à pas
// Fonction appel�e apr�s le timout d�fini par l'utilisateur, Stop le moteur sp�cifi�
// -------------------------------------------------------------------
int endStepperAction(int actionNumber, int motorNb){
	int endOfTask;
	//printf("Action number: %d - End of timer for wheel No: %d\n",actionNumber , motorNb);

	// Stop le moteur
//	setMotorSpeed(motorNb, 0);
        setStepperStepAction(motorNb, BUGGY_STOP, NULL);
        body.stepper[motorNb].direction = BUGGY_STOP;
        
	// Retire l'action de la table et v�rification si toute les actions sont effectu�es
	// Pour la t�che en cours donn�e par le message ALGOID

	endOfTask = removeBuggyTask(actionNumber);

	// Contr�le que toutes les actions ont �t� effectu�e pour la commande recue dans le message ALGOID
	if(endOfTask){
		// R�cup�re l'expediteur original du message ayant provoqu�
		// l'�venement
		char msgTo[32];
		int ptr=getSenderFromMsgId(endOfTask);
		strcpy(msgTo, msgEventHeader[ptr].msgFrom);
		// Lib�re la memorisation de l'expediteur
		removeSenderOfMsgId(endOfTask);

		AlgoidResponse[0].responseType=EVENT_ACTION_END;

		sendResponse(endOfTask, msgTo, EVENT, STEPPER, 1);
		sprintf(reportBuffer, "FIN DES ACTIONS \"MOTEUR PAS A PAS\" pour la tache #%d\n", endOfTask);
		printf(reportBuffer);
		sendMqttReport(endOfTask, reportBuffer);
	}

	return 0;
}


// ----------------------------------------------------------------------
// CHECKSTEPPERSTATUS
// Verificatio du status du moteur pas à pas
// Fonction appel�e apr�s le timout d�fini par l'utilisateur.
// -----------------------------------------------------------------------

int checkStepperStatus(int actionNumber, int encoderName){
	float distance;					// Variable de distance parcourue depuis le start
	//unsigned char encoderNumber;

	//encoderNumber = getOrganNumber(encoderName);

//	distance = getMotorPulses(encoderName);

//	if(distance >=0){
//		distance = (distance*CMPP);
//    	usleep(2200);
//	}else  printf("\n ERROR: I2CBUS READ\n");
	//printf("\n Encodeur #%d -> START %.2f cm  STOP %.2f cm", encoderNumber, startEncoderValue[encoderNumber], stopEncoderValue[encoderNumber]);

//	if(distance >= body.encoder[encoderName].stopEncoderValue)
//		endWheelAction(actionNumber, encoderName);
//	else
//		setTimer(50, &checkMotorEncoder, actionNumber, encoderName, MOTOR);

	return 0;
}

int dummyStepperAction(int actionNumber, int encoderName){
    setTimer(500, &dummyStepperAction, actionNumber, encoderName, MOTOR);
//setStepperStepAction(motorNb, body.stepper[encoderName].speed,  body.stepper[encoderName].step );
    return 0;
}