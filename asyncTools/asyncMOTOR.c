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
// Effectue l'action sur une roue sp�cifi�e
// - D�marrage du timer avec definition de fonction call-back, et no d'action
// - D�marrage du mouvement de la roue sp�cifi�e
// - V�locit� entre -100 et +100 qui d�fini le sens de rotation du moteur
// -------------------------------------------------------------------

int setAsyncMotorAction(int actionNumber, int motorNb, int veloc, char unit, int value){
	int myDirection;
	int setTimerResult;
	int endOfTask;           
        
	if(veloc == 0){
		myDirection=BUGGY_STOP;
		body.motor[motorNb].direction = 0;
	}else
        {
            if(veloc < 0){
                // Check if motor inversion requiered and modify if necessary
                if(!sysConfig.motor[motorNb].inverted) myDirection=BUGGY_FORWARD;
                else myDirection=BUGGY_BACK;
                body.motor[motorNb].direction = -1;
                veloc *=-1;					// Convertion en valeur positive
            }else{      
                // Check if motor inversion requiered and modify if necessary
                if(!sysConfig.motor[motorNb].inverted) myDirection=BUGGY_BACK;
                else myDirection=BUGGY_FORWARD;
            body.motor[motorNb].direction = 1;
            }            
        }


	// D�marre de timer d'action sur la roue et sp�cifie la fonction call back � appeler en time-out
	// Valeur en retour >0 signifie que l'action "en retour" � �t� �cras�e
	switch(unit){
		case  MILLISECOND:  setTimerResult=setTimer(value, &endWheelAction, actionNumber, motorNb, MOTOR); break;
		case  CENTIMETER:   //motorNb = getOrganNumber(motorNb);
                                    body.encoder[motorNb].startEncoderValue=getMotorPulses(motorNb)*(sysConfig.wheel[motorNb]._MMPP/10); // (/10 = Convert millimeter per pulse to centimeter per pulse)
                                    body.encoder[motorNb].stopEncoderValue = body.encoder[motorNb].startEncoderValue+ value;
                                    setTimerResult=setTimer(50, &checkMotorEncoder, actionNumber, motorNb, MOTOR); break;// D�marre un timer pour contr�le de distance chaque 35mS
                                   
                case  INFINITE:     setTimerResult=setTimer(100, &dummyMotorAction, actionNumber, motorNb, MOTOR); break;
		default: printf("\n!!! ERROR Function [setAsyncMotorAction] -> unknown mode");break;
	}

	if(setTimerResult!=0){						// Timer pret, action effectu�e ()
		if(setTimerResult>1){					// Le timer � �t� �cras� par la nouvelle action en retour car sur la m�me roue
			endOfTask=removeBuggyTask(setTimerResult);	// Supprime l'ancienne t�che qui � �t� �cras�e par la nouvelle action
			if(endOfTask){
				sprintf(reportBuffer, "Annulation des actions moteur pour la tache #%d\n", endOfTask);

				// R�cup�re l'expediteur original du message ayant provoqu�
				// l'�venement
				char msgTo[32];
				int ptr=getSenderFromMsgId(endOfTask);
				strcpy(msgTo, msgEventHeader[ptr].msgFrom);
				// Lib�re la memorisation de l'expediteur
				removeSenderOfMsgId(endOfTask);

				AlgoidResponse[0].responseType=EVENT_ACTION_ABORT;
				sendResponse(endOfTask, AlgoidCommand.msgFrom, EVENT, MOTORS, 1);			// Envoie un message ALGOID de fin de t�che pour l'action �cras�
				printf(reportBuffer);									// Affichage du message dans le shell
				sendMqttReport(endOfTask, reportBuffer);                                                // Envoie le message sur le canal MQTT "Report"
			}
		}
                
		// D�fini le "nouveau" sens de rotation � applique au moteur ainsi que la consigne de vitesse
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
// Fonction appel�e apr�s le timout d�fini par l'utilisateur, Stop le moteur sp�cifi�
// -------------------------------------------------------------------
int endWheelAction(int actionNumber, int motorNb){
	int endOfTask;
	//printf("Action number: %d - End of timer for wheel No: %d\n",actionNumber , motorNb);

	// Stop le moteur
	setMotorSpeed(motorNb, 0);
        body.motor[motorNb].direction = BUGGY_STOP;
        
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

		sendResponse(endOfTask, msgTo, EVENT, MOTORS, 1);
		sprintf(reportBuffer, "FIN DES ACTIONS \"WHEEL\" pour la tache #%d\n", endOfTask);
		printf(reportBuffer);
		sendMqttReport(endOfTask, reportBuffer);
	}

	return 0;
}


// ----------------------------------------------------------------------
// CHECKMOTORENCODER
// Contr�le la distance parcourue et stop la roue si destination atteinte
// Fonction appel�e apr�s le timout d�fini par l'utilisateur.
// -----------------------------------------------------------------------

int checkMotorEncoder(int actionNumber, int encoderName){
	float distance;					// Variable de distance parcourue depuis le start
	//unsigned char encoderNumber;

	//encoderNumber = getOrganNumber(encoderName);

	distance = getMotorPulses(encoderName);

	if(distance >=0){
		distance = distance * (sysConfig.wheel[encoderName]._MMPP/10);
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