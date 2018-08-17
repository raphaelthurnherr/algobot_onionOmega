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

int setAsyncLedAction(int actionNumber, int ledName, int mode, int time, int count);
int endLedAction(int actionNumber, int ledNumber);
int checkBlinkLedCount(int actionNumber, int ledName);

// -------------------------------------------------------------------
// SETASYNCLEDACTION
// Effectue l'action de clignotement
// - D�marrage du timer avec definition de fonction call-back, et no d'action
// - D�marrage du clignotement
// - vitesse de clignotement en mS
// -------------------------------------------------------------------

int setAsyncLedAction(int actionNumber, int ledName, int mode, int time, int count){
	int setTimerResult;
	int endOfTask;

	// D�marre un timer d'action sur la led et sp�cifie la fonction call back � appeler en time-out
	// Valeur en retour >0 signifie que l'action "en retour" � �t� �cras�e
        if(mode==BLINK){
            setTimerResult=setTimer(time, &checkBlinkLedCount, actionNumber, ledName, LED);
        }
        else{
            if(mode==ON)
                setLedPower(ledName, body.led[ledName].power); 
            else
                if(mode==OFF)
                    setLedPower(ledName, 0);
            // Utilise un delais de 5ms sinon message "Begin" arrive apres
            setTimerResult=setTimer(5, &checkBlinkLedCount, actionNumber, ledName, LED);     // Consid�re un blink infini  
        }
        
	if(setTimerResult!=0){                                          // Timer pret, action effectu�e
		if(setTimerResult>1){					// Le timer � �t� �cras� par la nouvelle action en retour car sur le meme peripherique
			endOfTask=removeBuggyTask(setTimerResult);	// Supprime l'ancienne t�che qui � �t� �cras�e par la nouvelle action
                        if(endOfTask){
                                sprintf(reportBuffer, "Annulation des actions LED pour la tache #%d\n", setTimerResult);

                                // R�cup�re l'expediteur original du message ayant provoqu�
                                // l'�venement
                                char msgTo[32];
                                int ptr=getSenderFromMsgId(endOfTask);
                                strcpy(msgTo, msgEventHeader[ptr].msgFrom);
                                // Lib�re la memorisation de l'expediteur
                                removeSenderOfMsgId(endOfTask);
                                
                                AlgoidResponse[0].responseType=EVENT_ACTION_ABORT;
                                sendResponse(endOfTask, AlgoidCommand.msgFrom, EVENT, pLED, 1);		// Envoie un message ALGOID de fin de t�che pour l'action �cras�
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

// -------------------------------------------------------------------
// ENDBLINKACTION
// Fin de l'action de clignotement
// Fonction appel�e apr�s le timout d�fini par l'utilisateur, Stop le clignotement
// -------------------------------------------------------------------
int endLedAction(int actionNumber, int LedNumber){
	int endOfTask;
	// Retire l'action de la table et v�rification si toute les actions sont effectu�es
	// Pour la t�che en cours donn�e par le message ALGOID

        endOfTask=removeBuggyTask(actionNumber);
        if(endOfTask>0){
                sprintf(reportBuffer, "FIN DES ACTIONS LED pour la tache #%d\n", endOfTask);

                // R�cup�re l'expediteur original du message ayant provoqu�
                // l'�venement
                char msgTo[32];
                int ptr=getSenderFromMsgId(endOfTask);
                strcpy(msgTo, msgEventHeader[ptr].msgFrom);
                // Lib�re la memorisation de l'expediteur
                removeSenderOfMsgId(endOfTask);
                AlgoidResponse[0].responseType=EVENT_ACTION_END;
                sendResponse(endOfTask, msgTo, EVENT, pLED, 1);			// Envoie un message ALGOID de fin de t�che pour l'action �cras�
                printf(reportBuffer);									// Affichage du message dans le shell
                sendMqttReport(endOfTask, reportBuffer);				// Envoie le message sur le canal MQTT "Report"
        }

	return 0;
}

// ----------------------------------------------------------------------
// CHECKBLINKCOUNT
// CONTROLE LE NOMBRE DE CLIGNOTEMENT SUR LA LED
// Fonction appel�e apr�s le timout d�fini par l'utilisateur.
// -----------------------------------------------------------------------

int checkBlinkLedCount(int actionNumber, int ledName){
	static int blinkCount=0;     // Variable de comptage du nombre de clignotements       
        static int LEDtoggleState[NBLED];

         
        // Si mode blink actif, toggle sur LED et comptage
        if(body.led[ledName].state==BLINK){
            
            // Performe the LED toggle
            if(LEDtoggleState[ledName]>0){
                setLedPower(ledName, 0);
                LEDtoggleState[ledName]=0;
            }else
            {
                setLedPower(ledName, body.led[ledName].power);
                LEDtoggleState[ledName]=1;
            }
           
            // Consigned de clignotement atteinte ?
            if(blinkCount >= body.led[ledName].blinkCount-1){
                body.led[ledName].state=LEDtoggleState[ledName];      // Update the actual state of led
                endLedAction(actionNumber, ledName);
                blinkCount=0;                                   // Reset le compteur
            }
            else{
                    setTimer(body.led[ledName].blinkTime, &checkBlinkLedCount, actionNumber, ledName, LED);                    
                    blinkCount++;
            }
        }
        else{
            // Termine l'action unique (on/off)
            endLedAction(actionNumber, ledName);
            blinkCount=0;
        }
    
	return 0;
}