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

int setAsyncPwmAction(int actionNumber, int pwmName, int mode, int time, int count);
int checkBlinkPwmCount(int actionNumber, int pwmName);
int endPwmAction(int actionNumber, int pwmNumber);

// -------------------------------------------------------------------
// SETASYNCPWMACTION
// Effectue l'action de clignotement
// - Démarrage du timer avec definition de fonction call-back, et no d'action
// - Démarrage du clignotement
// - vitesse de clignotement en mS
// -------------------------------------------------------------------

int setAsyncPwmAction(int actionNumber, int pwmName, int mode, int time, int count){
	int setTimerResult;
	int endOfTask;

	// Démarre un timer d'action sur la led et spécifie la fonction call back à appeler en time-out
	// Valeur en retour >0 signifie que l'action "en retour" à été écrasée
        if(mode==INFINITE){
            setTimerResult=setTimer(time, &checkBlinkPwmCount, actionNumber, pwmName, PWM);
        }
        else{
            if(mode==ON)
                setPwmPower(pwmName, body.pwm[pwmName].power); 
            else
                if(mode==OFF)
                    setPwmPower(pwmName, 0);
            // Utilise un delais de 5ms sinon message "Begin" arrive apres
            setTimerResult=setTimer(5, &checkBlinkPwmCount, actionNumber, pwmName, PWM);     // Considère un blink infini  
        }
        
	if(setTimerResult!=0){                                          // Timer pret, action effectuée
		if(setTimerResult>1){					// Le timer à été écrasé par la nouvelle action en retour car sur le meme peripherique
			endOfTask=removeBuggyTask(setTimerResult);	// Supprime l'ancienne tâche qui à été écrasée par la nouvelle action
                        if(endOfTask){
                                sprintf(reportBuffer, "Annulation des actions PWM pour la tache #%d\n", setTimerResult);

                                // Récupère l'expediteur original du message ayant provoqué
                                // l'évenement
                                char msgTo[32];
                                int ptr=getSenderFromMsgId(endOfTask);
                                strcpy(msgTo, msgEventHeader[ptr].msgFrom);
                                // Libère la memorisation de l'expediteur
                                removeSenderOfMsgId(endOfTask);
                                
                                AlgoidResponse[0].responseType=EVENT_ACTION_ABORT;
                                sendResponse(endOfTask, AlgoidCommand.msgFrom, EVENT, pPWM, 1);		// Envoie un message ALGOID de fin de tâche pour l'action écrasé
                                printf(reportBuffer);                                                   // Affichage du message dans le shell
                                sendMqttReport(endOfTask, reportBuffer);				// Envoie le message sur le canal MQTT "Report"
                        }
		}
                
                
                
                return 0;
	}
	else {
            printf("Error, Impossible to set timer led\n");
            return -1;
        }
}


// ----------------------------------------------------------------------
// CHECKBLINKPWMCOUNT
// CONTROLE LE NOMBRE DE CLIGNOTEMENT SUR LA SORTIE PWM
// Fonction appelée après le timout défini par l'utilisateur.
// -----------------------------------------------------------------------

int checkBlinkPwmCount(int actionNumber, int pwmName){
	static int blinkCount=0;     // Variable de comptage du nombre de clignotements       
        static int PWMtoggleState[NBPWM];

        // Si mode blink actif, toggle sur PWM et comptage
        if(body.pwm[pwmName].state==LED_BLINK){    
            
            // Consigned de clignotement atteinte ?
            if(blinkCount >= body.pwm[pwmName].blinkCount){
                endPwmAction(actionNumber, pwmName);
                blinkCount=0;                                   // Reset le compteur
            }
            else{
                    setTimer(body.pwm[pwmName].blinkTime, &checkBlinkPwmCount, actionNumber, pwmName, PWM);      
            }

            blinkCount++;

            // Realisation du TOGGLE sur la sortie PWM
            if(PWMtoggleState[pwmName]>0){
                setPwmPower(pwmName, 0);
                PWMtoggleState[pwmName]=0;
            }else
            {
                setPwmPower(pwmName, body.pwm[pwmName].power);
                PWMtoggleState[pwmName]=1;
            }
        }
        else{
            // Termine l'action unique (on/off)
            endPwmAction(actionNumber, pwmName);
        }
    
	return 0;
}

// -------------------------------------------------------------------
// ENDPWMACTION
// Fin de l'action de clignotement
// Fonction appelée après le timout défini par l'utilisateur, Stop le clignotement
// -------------------------------------------------------------------
int endPwmAction(int actionNumber, int pwmNumber){
	int endOfTask;

	// Retire l'action de la table et vérification si toute les actions sont effectuées
	// Pour la tâche en cours donnée par le message ALGOID

        endOfTask=removeBuggyTask(actionNumber);
        if(endOfTask){
                sprintf(reportBuffer, "FIN DES ACTIONS PWM pour la tache #%d\n", endOfTask);

                // Récupère l'expediteur original du message ayant provoqué
                // l'évenement
                char msgTo[32];
                int ptr=getSenderFromMsgId(endOfTask);
                strcpy(msgTo, msgEventHeader[ptr].msgFrom);
                // Libère la memorisation de l'expediteur
                removeSenderOfMsgId(endOfTask);
                AlgoidResponse[0].responseType=EVENT_ACTION_END;
                sendResponse(endOfTask, msgTo, EVENT, pPWM, 1);			// Envoie un message ALGOID de fin de tâche pour l'action écrasé
                printf(reportBuffer);									// Affichage du message dans le shell
                sendMqttReport(endOfTask, reportBuffer);				// Envoie le message sur le canal MQTT "Report"
        }

	return 0;
}
 