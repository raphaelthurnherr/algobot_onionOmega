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

int setAsyncServoAction(int actionNumber, int pwmName, int mode, int time);
int checkBlinkServoCount(int actionNumber, int pwmName);
int endServoAction(int actionNumber, int pwmNumber);

// -------------------------------------------------------------------
// SETASYNCSERVOACTION
// Effectue l'action de clignotement
// - D�marrage du timer avec definition de fonction call-back, et no d'action
// - D�marrage du clignotement
// - vitesse de clignotement en mS
// -------------------------------------------------------------------

int setAsyncServoAction(int actionNumber, int pwmName, int mode, int time){
	int setTimerResult;
	int endOfTask;

	// D�marre un timer d'action sur le PWM et sp�cifie la fonction call back � appeler en time-out
	// Valeur en retour >0 signifie que l'action "en retour" � �t� �cras�e

        if(mode==ON)
            setServoPosition(pwmName, robot.pwm[pwmName].power); 
        else
            if(mode==OFF)
            setServoPosition(pwmName, -1);
        
        // Utilise un delais de 5ms sinon message "Begin" arrive apres
        setTimerResult=setTimer(5, &checkBlinkServoCount, actionNumber, pwmName, PWM);     // Consid�re un blink infini  
        
	if(setTimerResult!=0){                                          // Timer pret, action effectu�e
		if(setTimerResult>1){					// Le timer � �t� �cras� par la nouvelle action en retour car sur le meme peripherique
			endOfTask=removeBuggyTask(setTimerResult);	// Supprime l'ancienne t�che qui � �t� �cras�e par la nouvelle action
                        if(endOfTask){
                           

                                sprintf(reportBuffer, "Annulation des actions PWM pour la tache #%d\n", setTimerResult);

                                // Recupere l'expediteur original du message ayant provoque
                                // l'evenement
                                char msgTo[32];
                                int ptr=getSenderFromMsgId(endOfTask);
                                strcpy(msgTo, msgEventHeader[ptr].msgFrom);
                                // Libere la memorisation de l'expediteur
                                removeSenderOfMsgId(endOfTask);
                                
                                AlgoidResponse[0].responseType=EVENT_ACTION_ABORT;
                                sendResponse(endOfTask, AlgoidCommand.msgFrom, EVENT, pSERVO, 1);		// Envoie un message ALGOID de fin de t�che pour l'action �cras�
                                printf(reportBuffer);                                                   // Affichage du message dans le shell
                                sendMqttReport(endOfTask, reportBuffer);				// Envoie le message sur le canal MQTT "Report"
                        }
		}
                
                
                
                return 0;
	}
	else {
            printf("Error, Impossible to set timer Servo\n");
            return -1;
        }
}

// ----------------------------------------------------------------------
// CHECKBLINKSERVOCOUNT
// CONTROLE LE NOMBRE DE CLIGNOTEMENT SUR LA SORTIE SERVO
// Fonction appel�e apr�s le timout d�fini par l'utilisateur.
// -----------------------------------------------------------------------

int checkBlinkServoCount(int actionNumber, int pwmName){
	static int blinkCount=0;     // Variable de comptage du nombre de clignotements       
        static int PWMtoggleState[NBPWM];

        // Si mode blink actif, toggle sur PWM et comptage
        if(robot.pwm[pwmName].state==BLINK){    
            
            // Consigned de clignotement atteinte ?
            if(blinkCount >= robot.pwm[pwmName].blinkCount){
                endServoAction(actionNumber, pwmName);
                blinkCount=0;                                   // Reset le compteur
            }
            else{
                    setTimer(robot.pwm[pwmName].blinkTime, &checkBlinkServoCount, actionNumber, pwmName, PWM);      
            }

            blinkCount++;

            // Realisation du TOGGLE sur la sortie PWM
            if(PWMtoggleState[pwmName]>0){
                setPwmPower(pwmName, 0);
                PWMtoggleState[pwmName]=0;
            }else
            {
                setPwmPower(pwmName, robot.pwm[pwmName].power);
                PWMtoggleState[pwmName]=1;
            }
        }
        else{
            // Termine l'action unique (on/off)
            endServoAction(actionNumber, pwmName);
        }
    
	return 0;
}

// -------------------------------------------------------------------
// ENDPWMACTION
// Fin de l'action de clignotement
// Fonction appel�e apr�s le timout d�fini par l'utilisateur, Stop le clignotement
// -------------------------------------------------------------------
int endServoAction(int actionNumber, int pwmNumber){
	int endOfTask;

	// Retire l'action de la table et v�rification si toute les actions sont effectu�es
	// Pour la t�che en cours donn�e par le message ALGOID

        endOfTask=removeBuggyTask(actionNumber);
        if(endOfTask){
                sprintf(reportBuffer, "FIN DES ACTIONS PWM pour la tache #%d\n", endOfTask);

                // R�cup�re l'expediteur original du message ayant provoqu�
                // l'�venement
                char msgTo[32];
                int ptr=getSenderFromMsgId(endOfTask);
                strcpy(msgTo, msgEventHeader[ptr].msgFrom);
                // Lib�re la memorisation de l'expediteur
                removeSenderOfMsgId(endOfTask);
                AlgoidResponse[0].responseType=EVENT_ACTION_END;
                sendResponse(endOfTask, msgTo, EVENT, pPWM, 1);			// Envoie un message ALGOID de fin de t�che pour l'action �cras�
                printf(reportBuffer);									// Affichage du message dans le shell
                sendMqttReport(endOfTask, reportBuffer);				// Envoie le message sur le canal MQTT "Report"
        }

	return 0;
}
 