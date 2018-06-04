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
// - D�marrage du timer avec definition de fonction call-back, et no d'action
// - D�marrage du clignotement
// - vitesse de clignotement en mS
// -------------------------------------------------------------------

int setAsyncLedAction(int actionNumber, int ledName, int time, int count){
	int setTimerResult;
	int endOfTask;

	// D�marre un timer d'action sur la led et sp�cifie la fonction call back � appeler en time-out
	// Valeur en retour >0 signifie que l'action "en retour" � �t� �cras�e
        if(count>0)
            setTimerResult=setTimer(time, &checkBlinkLedCount, actionNumber, ledName, LED);
        else
            setTimerResult=setTimer(time, &dummyLedAction, actionNumber, ledName, LED);     // Consid�re un blink infini
        
        
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
                                
                                AlgoidResponse[0].responseType=2;
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
                AlgoidResponse[0].responseType=0;
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

        // Inverse l'�tat de la led
        if(body.led[ledName].state>0){
            setLedPower(ledName, 0);
            body.led[ledName].state=0;
        }else
        {
            setLedPower(ledName, body.led[ledName].power);
            body.led[ledName].state=1;
        }
        
        // Contr�le le nombre de clignotement
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

        // Inverse l'�tat de la led
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