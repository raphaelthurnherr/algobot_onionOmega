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

int setAsyncLedAction(int actionNumber, int ledName, int time, int count);
int endLedAction(int actionNumber, int ledNumber);
int checkBlinkLedCount(int actionNumber, int ledName);
int dummyLedAction(int actionNumber, int ledName); // Fonction sans action, appelee en cas de blink infini

// -------------------------------------------------------------------
// SETASYNCLEDACTION
// Effectue l'action de clignotement
// - Démarrage du timer avec definition de fonction call-back, et no d'action
// - Démarrage du clignotement
// - vitesse de clignotement en mS
// -------------------------------------------------------------------

int setAsyncLedAction(int actionNumber, int ledName, int time, int count){
	int setTimerResult;
	int endOfTask;

	// Démarre un timer d'action sur la led et spécifie la fonction call back à appeler en time-out
	// Valeur en retour >0 signifie que l'action "en retour" à été écrasée
        if(count>0)
            setTimerResult=setTimer(time, &checkBlinkLedCount, actionNumber, ledName, LED);
        else
            setTimerResult=setTimer(time, &dummyLedAction, actionNumber, ledName, LED);     // Considère un blink infini
        
        
	if(setTimerResult!=0){                                          // Timer pret, action effectuée
		if(setTimerResult>1){					// Le timer à été écrasé par la nouvelle action en retour car sur le meme peripherique
			endOfTask=removeBuggyTask(setTimerResult);	// Supprime l'ancienne tâche qui à été écrasée par la nouvelle action
                        if(endOfTask){
                                sprintf(reportBuffer, "Annulation des actions LED pour la tache #%d\n", setTimerResult);

                                // Récupère l'expediteur original du message ayant provoqué
                                // l'évenement
                                char msgTo[32];
                                int ptr=getSenderFromMsgId(endOfTask);
                                strcpy(msgTo, msgEventHeader[ptr].msgFrom);
                                // Libère la memorisation de l'expediteur
                                removeSenderOfMsgId(endOfTask);
                                
                                AlgoidResponse[0].responseType=2;
                                sendResponse(endOfTask, AlgoidCommand.msgFrom, EVENT, pLED, 1);		// Envoie un message ALGOID de fin de tâche pour l'action écrasé
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
// Fonction appelée après le timout défini par l'utilisateur, Stop le clignotement
// -------------------------------------------------------------------
int endLedAction(int actionNumber, int LedNumber){
	int endOfTask;
	// Retire l'action de la table et vérification si toute les actions sont effectuées
	// Pour la tâche en cours donnée par le message ALGOID

        endOfTask=removeBuggyTask(actionNumber);
        if(endOfTask>0){
                sprintf(reportBuffer, "FIN DES ACTIONS LED pour la tache #%d\n", endOfTask);

                // Récupère l'expediteur original du message ayant provoqué
                // l'évenement
                char msgTo[32];
                int ptr=getSenderFromMsgId(endOfTask);
                strcpy(msgTo, msgEventHeader[ptr].msgFrom);
                // Libère la memorisation de l'expediteur
                removeSenderOfMsgId(endOfTask);
                AlgoidResponse[0].responseType=0;
                sendResponse(endOfTask, msgTo, EVENT, pLED, 1);			// Envoie un message ALGOID de fin de tâche pour l'action écrasé
                printf(reportBuffer);									// Affichage du message dans le shell
                sendMqttReport(endOfTask, reportBuffer);				// Envoie le message sur le canal MQTT "Report"
        }

	return 0;
}

// ----------------------------------------------------------------------
// CHECKBLINKCOUNT
// CONTROLE LE NOMBRE DE CLIGNOTEMENT SUR LA LED
// Fonction appelée après le timout défini par l'utilisateur.
// -----------------------------------------------------------------------

int checkBlinkLedCount(int actionNumber, int ledName){
	static int blinkCount=0;     // Variable de comptage du nombre de clignotements       

        // Inverse l'état de la led
        if(body.led[ledName].state>0){
            setLedPower(ledName, 0);
            body.led[ledName].state=0;
        }else
        {
            setLedPower(ledName, body.led[ledName].power);
            body.led[ledName].state=1;
        }
        
        // Contrôle le nombre de clignotement
	if(blinkCount >= body.led[ledName].blinkCount){
            endLedAction(actionNumber, ledName);
            blinkCount=0;                                   // Reset le compteur
        }
		
	else{
		setTimer(body.led[ledName].blinkTime, &checkBlinkLedCount, actionNumber, ledName, LED);      
        }
        
        blinkCount++;
        
	return 0;
}


// ----------------------------------------------------------------------
// DUMMYLEDACTION
// Fonction sans action, appelee en cas de blink infini
// -----------------------------------------------------------------------

int dummyLedAction(int actionNumber, int ledName){
    	static int blinkCount=0;     // Variable de comptage du nombre de clignotements       

        // Inverse l'état de la led
        if(body.led[ledName].state>0){
            setLedPower(ledName, 0);
            body.led[ledName].state=0;
        }else
        {
            setLedPower(ledName, body.led[ledName].power);
            body.led[ledName].state=1;
        }
    setTimer(100, &dummyLedAction, actionNumber, ledName, LED);
    return 0;
}