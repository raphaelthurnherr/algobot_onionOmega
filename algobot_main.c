#define TASK_NUMBER 0
#define ACTION_ALGOID_ID 1
#define ACTION_COUNT 2

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "buggy_descriptor.h"
#include "messagesManager.h"
#include "linux_json.h"
#include "udpPublish.h"
#include "tools.h"
#include "algobot_main.h"
#include "timerManager.h"
#include "hwManager.h"
#include "asyncTools.h"
#include "asyncPWM.h"
#include "asyncLED.h"

unsigned char ptrSpeedCalc;
int createBuggyTask(int MsgId, int actionCount);
int removeBuggyTask(int actionNumber);

void distanceEventCheck(void);
void batteryEventCheck(void);
void DINEventCheck(void);

void DINSafetyCheck(void);
void BatterySafetyCheck(void);
void DistanceSafetyCheck(void);

int ActionTable[10][3];

// Traitement du message algoid recu
int processAlgoidCommand(void);
int processAlgoidRequest(void);

int makeSensorsRequest(void);
int makeDistanceRequest(void);
int makeBatteryRequest(void);
int makeStatusRequest(void);
int makeMotorRequest(void);

int runMotorAction(void);

int getWDvalue(int wheelName);

int runPwmAction(void);
//int setAsyncPwmAction(int actionNumber, int ledName, int time, int count);
//int endPwmAction(int actionNumber, int wheelNumber);

int getPwmSetting(int name);

int runLedAction(void);

int getLedSetting(int name);


int makeServoAction(void);

int getServoSetting(int servoName);

char reportBuffer[256];

t_sensor body;

// -------------------------------------------------------------------
// MAIN APPLICATION
// - Cr�ation de t�che de gestion de la messagerie avec ALGOID, (ALGOID->JSON->MQTT BROCKER->JSON->BUGGY)
// - Cr�ation de t�che de gestion des timers pour la commande ON/OFF des roues, de l'acc�l�ration des roues, et timer @ 50mS, 100mS, 10Sec
// - Initialisation de la carte de commande hardware pour les moteurs, capteurs, etc...
// - Initialisation d'un broadcast UDP pour publication de la p�sence du buggy sur le r�seau
// -------------------------------------------------------------------

int main(void) {
	int i;

	system("clear");
        printf ("ALGOBOT Beta - Build 180404 \n");
        printf ("----------------------------\n");
        
// Cr�ation de la t�che pour la gestion de la messagerie avec ALGOID
	if(InitMessager()) printf ("#[CORE] Creation t�che messagerie : ERREUR\n");
	else printf ("#[CORE] Demarrage tache Messager: OK\n");

        
// Cr�ation de la t�che pour la gestion des diff�rents timers utilis�s
	if(InitTimerManager()) printf ("#[CORE] Creation t�che timer : ERREUR\n");
		else printf ("#[CORE] Demarrage tache timer: OK\n");

// Cr�ation de la t�che pour la gestion hardware
	if(InitHwManager()) printf ("#[CORE] Creation t�che hardware : ERREUR\n");
		else printf ("#[CORE] Demarrage tache hardware: OK\n");

// Initialisation UDP pour broadcast IP Adresse
	initUDP();

// --------------------------------------------------------------------
// BOUCLE DU PROGRAMME PRINCIPAL
// - Messagerie avec ALGOID, attentes de messages en provenance de l'h�te -> D�marrage du traitement des commandes
// - Annonce UDP de pr�sence du buggy sur le r�seau  chaque 10Seconde
// - Gestion de l'acceleration du Buggy
// - Mesure sur les capteurs de distance, DIN et batterie
// - Gestion des �venements provoqu�s par les capteurs
// --------------------------------------------------------------------

	// ----------- DEBUT DE LA BOUCLE PRINCIPALE ----------

	// Init body membre
	for(i=0;i<NBAIN;i++){
		body.battery[i].safetyStop_value=0;
		body.battery[i].event_enable=0;
		body.battery[i].event_high=65535;
		body.battery[i].event_low=0;
		body.battery[i].safetyStop_state=0;
		body.battery[i].safetyStop_value=0;
	}

	for(i=0;i<NBDIN;i++){
		body.proximity[i].em_stop=0;
		body.proximity[i].event_enable=0;
		body.proximity[i].safetyStop_state=0;
		body.proximity[i].safetyStop_value=0;
	}
        
        for(i=0;i<NBMOTOR;i++){
		body.motor[i].accel=0;
                body.motor[i].cm=0;
                body.motor[i].decel=0;
                body.motor[i].distance=0;
                body.motor[i].speed=0;
                body.motor[i].time=0;
	}

	while(1){

		// Contr�le de la messagerie, recherche d'�ventuels messages ALGOID et effectue les traitements n�c�ssaire
		// selon le type du message [COMMAND, REQUEST, NEGOCIATION, ACK, REPONSE, ERROR, etc...]
		if(pullMsgStack(0)){
			switch(AlgoidCommand.msgType){
				case COMMAND : processAlgoidCommand(); break;						// Traitement du message de type "COMMAND"
				case REQUEST : processAlgoidRequest(); break;						// Traitement du message de type "REQUEST"
				default : ; break;
			}

		}


		// Gestion de la v�locit� pour une acceleration proggressive
    	// modification de la v�locit� environ chaque 50mS
    	if(checkMotorPowerFlag){
			checkDCmotorPower();													// Contr�le si la v�locit� correspond � la consigne
			checkMotorPowerFlag=0;
    	}


		// Contr�le du TIMER 10seconde
    	if(t10secFlag){
    		// Envoie un message UDP sur le r�seau, sur port 53530 (CF udpPublish.h)
    		// Avec le ID du buggy (fourni par le gestionnaire de messagerie)
    		char udpMessage[50];
    		sprintf(&udpMessage[0], "[ %s ] I'm here",ClientID);		// Formattage du message avec le Nom du client buggy
    		sendUDPHeartBit(udpMessage);								// Envoie du message
//		printf("\n Send UDP: %s", udpMessage);
    		t10secFlag=0;
    	}


		// Contr�le du TIMER 100mS
    	// - R�cup�ration de la tension de batterie
    	// - R�cup�ration de la distance mesur�e au sonar
    	// - Gestion des �venements batterie, digital inputs et distance
    	if(t100msFlag){


			DINEventCheck();											// Cont�le de l'�tat des entr�es num�rique
																		// G�n�re un �venement si changement d'�tat d�tect�

			DINSafetyCheck();											// Cont�le de l'�tat des entr�es num�rique
			BatterySafetyCheck();
			DistanceSafetyCheck(); 										// effectue une action si safety actif


			body.distance[0].value = getSonarDistance();
			distanceEventCheck();										// Provoque un �venement de type "distance" si la distance mesur�e
																		// est hors de la plage sp�cifi�e par l'utilisateur

			body.battery[0].value = getBatteryVoltage();

			batteryEventCheck();

                        for(i=0;i<NBMOTOR;i++){
                            body.motor[i].speed= (getMotorFrequency(i)*CMPP) * body.motor[i].direction;
                            body.motor[i].distance=getMotorPulses(i)*CMPP;
                        }

			// est hors a plage sp�cifi�e par les paramettre utilisateur
//			printf("Pulses left: %d    right: %d\n", test[0], test[1]);
			//printf("\nBattery: %d, safetyStop_state: %d safetyStop_value: %d", 0, body.battery[0].safetyStop_state, body.battery[0].safetyStop_value);
//			printf("\nSpeed : G %.1f   D %.1f   ||| Dist G: %.1fcm  Dist D: %.1fcm",
//					body.motor[MOTOR_0].speed, body.motor[MOTOR_1].speed, body.motor[MOTOR_0].distance, body.motor[MOTOR_1].distance);
//			printf(" dist US: %d cm\n", body.distance[0].value);

			t100msFlag=0;												// Quittance le flag 100mS
    	}

    	usleep(1000);													// Attente de 1ms
    }
	// ------------ FIN DE LA BOUCLE PRINCIPALE ----------------------


	// Fermetur du programme
	int endState=CloseMessager();										// Ferme la t�che de messagerie
	if(!endState)
		  printf( "# ARRET tache Messager - status: %d\n", endState);
	else printf( "# ARRET tache Messager erreur - status: %d\n", endState);

	return EXIT_SUCCESS;												// Fin du programme
}

// -------------------------------------------------------------------
// PROCESSCOMMAND
// S�l�ctionne et traite le param�tre de commande recue [LL2WD, BACK, FORWARD, STOP, SPIN, etc...]
// -------------------------------------------------------------------
int processAlgoidCommand(void){
    int i;
    
	switch(AlgoidCommand.msgParam){
		case MOTORS : 	
                                for(i=0;i<AlgoidCommand.msgValueCnt;i++){
                                   
                                    // Controle que le moteur existe...
                                    if(AlgoidCommand.DCmotor[i].motor >= 0 && AlgoidCommand.DCmotor[i].motor <NBMOTOR)
                                        AlgoidResponse[i].MOTresponse.motor=AlgoidCommand.DCmotor[i].motor;
                                    else
                                        AlgoidResponse[i].MOTresponse.motor=-1;
                                            
                                    // R�cup�ration des param�tes 
                                    AlgoidResponse[i].MOTresponse.velocity=AlgoidCommand.DCmotor[i].velocity;
                                    AlgoidResponse[i].MOTresponse.cm=AlgoidCommand.DCmotor[i].cm;
                                    AlgoidResponse[i].MOTresponse.time=AlgoidCommand.DCmotor[i].time;
                                    AlgoidResponse[i].responseType = 3;
                                }
                                // Retourne en r�ponse le message v�rifi�
                                sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, RESPONSE, MOTORS, AlgoidCommand.msgValueCnt);  // Retourne une r�ponse d'erreur, (aucun moteur d�fini)
                                
                                runMotorAction(); break;			// Action avec en param�tre MOTEUR, VELOCITE, ACCELERATION, TEMPS d'action
                                
                case pPWM  : 	
                                for(i=0;i<AlgoidCommand.msgValueCnt;i++){
                                    // Controle que le moteur existe...
                                    if(AlgoidCommand.PWMarray[i].id >= 0 && AlgoidCommand.PWMarray[i].id <NBPWM)
                                        AlgoidResponse[i].PWMresponse.id=AlgoidCommand.PWMarray[i].id;
                                    else
                                        AlgoidResponse[i].PWMresponse.id=-1;
                                            
                                    // R�cup�ration des param�tes 
                                    strcpy(AlgoidResponse[i].PWMresponse.state, AlgoidCommand.PWMarray[i].state);
                                    AlgoidResponse[i].PWMresponse.powerPercent=AlgoidCommand.PWMarray[i].powerPercent;
                                    AlgoidResponse[i].PWMresponse.blinkCount=AlgoidCommand.PWMarray[i].blinkCount;
                                    AlgoidResponse[i].PWMresponse.time=AlgoidCommand.PWMarray[i].time;
                                    AlgoidResponse[i].responseType = 3;
                                }
                                // Retourne en r�ponse le message v�rifi�
                                sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, RESPONSE, pPWM, AlgoidCommand.msgValueCnt);  // Retourne une r�ponse d'erreur, (aucun moteur d�fini)                    
                    
                                runPwmAction();break;
		case pLED  : 	runLedAction();break;
		default : break;
	}

	return 0;
}

// -------------------------------------------------------------------
// PROCESSREQUEST
// S�l�ction et traite le param�tre de requete recu [DISTANCE, TENSION BATTERIE, ENTREE DIGITAL, etc...]
// -------------------------------------------------------------------
int processAlgoidRequest(void){
	switch(AlgoidCommand.msgParam){
		case DISTANCE : makeDistanceRequest();					// Requete de distance
						break;

		case BATTERY :  makeBatteryRequest();					// Requete de tension batterie
						break;

		case DINPUT :	makeSensorsRequest();					// Requet d'�tat des entr�es digitale
						break;

		case STATUS :	makeStatusRequest();					// Requet d'�tat des entr�es digitale
						break;
        	case MOTORS :	makeMotorRequest();					// Requet d'�tat des entr�es digitale
						break;

		default : break;
	}
	return 0;
}


// -------------------------------------------------------------------
// runMotorAction
// Effectue une action avec les param�tre recus: MOTEUR, VELOCITE, ACCELERATION, TEMPS d'action
// -------------------------------------------------------------------
int runMotorAction(void){
	int ptrData;
	int myTaskId;
	unsigned char actionCount=0;
	int action=0;
        int i;
        int ID;

	// Comptabilise le nombre de param�tre (moteur) recu dans le message
	// 
        for(i=0;i<NBMOTOR;i++){
            ptrData=getWDvalue(i);
            if(ptrData>=0){
                actionCount++;
                
                    // Retourne un message ALGOID si velocit� hors tol�rences
                    if((AlgoidCommand.DCmotor[ptrData].velocity < -100) ||(AlgoidCommand.DCmotor[ptrData].velocity > 100))
                            sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom,  WARNING, MOTORS, 0);
                    else{
                        body.motor[i].speed=AlgoidCommand.DCmotor[ptrData].velocity;
                        body.motor[i].accel=AlgoidCommand.DCmotor[ptrData].accel;
                        body.motor[i].decel=AlgoidCommand.DCmotor[ptrData].decel;
                        body.motor[i].cm=AlgoidCommand.DCmotor[ptrData].cm;
                        body.motor[i].time=AlgoidCommand.DCmotor[ptrData].time;
                        
//                        printf("\nDEBUG: WHEEL %d SPEED %d ACCEL %d TIME %d CM %d\n ", AlgoidCommand.DCmotor[ptrData].motor, AlgoidCommand.DCmotor[ptrData].velocity, AlgoidCommand.DCmotor[ptrData].accel, AlgoidCommand.DCmotor[ptrData].time, AlgoidCommand.DCmotor[ptrData].cm);
//                        printf("\nDEBUGG: WHEEL %d SPEED %d ACCEL %d TIME %d CM %d\n ", i, body.motor[i].speed, body.motor[i].accel, body.motor[i].time, body.motor[i].cm);
                    }  
            }
        }

        // Au moin une action � effectuer
        if(actionCount>0){
            // Ouverture d'une t�che pour les toutes les actions du message algoid � effectuer
            // Recois un num�ro de tache en retour
            myTaskId=createBuggyTask(AlgoidCommand.msgID, actionCount);			// 2 actions pour mouvement 2WD

            // D�marrage des actions
            if(myTaskId>0){
                    printf("Creation de tache MOTOR: #%d avec %d actions\n", myTaskId, actionCount);

                    // Sauvegarde du nom de l'emetteur et du ID du message pour la r�ponse
                    // en fin d'�venement
                    saveSenderOfMsgId(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom);

                    for(ptrData=0; action < actionCount && ptrData<10; ptrData++){
                        ID = AlgoidCommand.DCmotor[ptrData].motor;
                        if(ID >= 0){
                            if(body.motor[ID].accel!=0 || body.motor[ID].decel)
                                setMotorAccelDecel(ID, body.motor[ID].accel, body.motor[ID].decel);
                            
                                                        // Effectue l'action sur la roue
                            if(body.motor[ID].cm != 0)
                                    setAsyncMotorAction(myTaskId, ID, body.motor[ID].speed, CENTIMETER, body.motor[ID].cm);
                            else
                                    setAsyncMotorAction(myTaskId, ID, body.motor[ID].speed, MILLISECOND, body.motor[ID].time);
                                                // D�fini l'�tat de laction comme "d�marr�e" pour message de r�pons
                            AlgoidResponse[0].responseType = 1;
                        }
                        action++;
                    }

                    // Retourne un message event ALGOID 
                    sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom,  EVENT, MOTORS, 1);
                    return 0;
            }
            else
                return 1;
        }
        // Aucun param�tre trouv� ou moteur inexistant
        else{
            
            //AlgoidResponse[0].responseType = -1;
            sprintf(reportBuffer, "ERREUR: Aucun moteur d�fini ou inexistant pour le message #%d\n", AlgoidCommand.msgID);
            //sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, EVENT, MOTORS, 1);  // Retourne une r�ponse d'erreur, (aucun moteur d�fini)
            printf(reportBuffer);                                                             // Affichage du message dans le shell
            sendMqttReport(AlgoidCommand.msgID, reportBuffer);				      // Envoie le message sur le canal MQTT "Report"
        }
}

// -------------------------------------------------------------------
// makeServoAction
//
// -------------------------------------------------------------------
int makeServoAction(void){
	int ptrData;
	int myTaskId;
	int endOfTask;
        int i;

	unsigned char actionCount=0;
	unsigned char action=0;

	// Recherche s'il y a des param�tres pour chaque roue
	// Des param�tres recu pour une roue cr�e une action � effectuer
        for(i=0;i<NBSERVO;i++){
            if(getServoSetting(i)>=0)
                actionCount++;
        }

        if(actionCount>0){
            
            // Ouverture d'une t�che pour les toutes les actions du message algoid � effectuer
            // Recois un num�ro de tache en retour
            myTaskId=createBuggyTask(AlgoidCommand.msgID, actionCount);			//

            // D�marrage des actions
            if(myTaskId>0){
                    printf("Creation de tache SERVO: #%d avec %d actions\n", myTaskId, actionCount);

                    // Sauvegarde du nom de l'emetteur et du ID du message pour la r�ponse
                    // en fin d'�venement
                    saveSenderOfMsgId(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom);

                    for(ptrData=0; action < actionCount && ptrData<10; ptrData++){
                            if(AlgoidCommand.PWMout[ptrData].id>=0){
                                    setServoPosition(AlgoidCommand.PWMout[ptrData].id, AlgoidCommand.PWMout[ptrData].angle);

                                    endOfTask=removeBuggyTask(myTaskId);
                                    if(endOfTask>0){
                                            sprintf(reportBuffer, "FIN DES ACTIONS \"SERVO\" pour la tache #%d\n", endOfTask);

                                            // R�cup�re l'expediteur original du message ayant provoqu�
                                            // l'�venement
                                            char msgTo[32];
                                            int ptr=getSenderFromMsgId(endOfTask);
                                            strcpy(msgTo, msgEventHeader[ptr].msgFrom);
                                            // Lib�re la memorisation de l'expediteur
                                            removeSenderOfMsgId(endOfTask);

                                            sendResponse(endOfTask, msgTo, EVENT, pPWM, 0);				// Envoie un message ALGOID de fin de t�che pour l'action �cras�
                                            printf(reportBuffer);									// Affichage du message dans le shell
                                            sendMqttReport(endOfTask, reportBuffer);				// Envoie le message sur le canal MQTT "Report"
                                    }

                                    action++;
                            }
                    }
            }
        }
        else{   
            sprintf(reportBuffer, "ERREUR: ID SERVO INEXISTANT pour le message #%d\n", AlgoidCommand.msgID);
            AlgoidResponse[0].SERVOresponse.id=-1;
            sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, RESPONSE, pSERVO, 1);  // Envoie un message ALGOID de fin de t�che pour l'action �cras�
            printf(reportBuffer);                                                           // Affichage du message dans le shell
            sendMqttReport(AlgoidCommand.msgID, reportBuffer);				// Envoie le message sur le canal MQTT "Report"
        }
	return 0;
}


// -------------------------------------------------------------------
// makeLEDAction
//
// -------------------------------------------------------------------
int runLedAction(void){
	int ptrData;
	int myTaskId;
	int endOfTask;
        int i;
        int ID;
        
        int time=0;
        int power=0;
        int Count=0;
        
	unsigned char actionCount=0;
	unsigned char action=0;

        // R�cup�re l'expediteur original du message ayant provoqu�
        // l'�venement
        
        char msgTo[32];
        
        // Recherche s'il y a des param�tres d�fini pour chaque LED
        // et mise � jour.   
        for(i=0;i<NBLED;i++){
            ptrData=getLedSetting(i);
            if(ptrData>=0){
                actionCount++;          // Incr�mente le nombre de param�tres trouv�s = action suppl�mentaire a effectuer
                
                // R�cup�ration de commande d'�tat de la led dans le message
                if(!strcmp(AlgoidCommand.LEDarray[ptrData].state,"off"))
                    body.led[i].state=0;
                if(!strcmp(AlgoidCommand.LEDarray[ptrData].state,"on"))
                    body.led[i].state=1;
                if(!strcmp(AlgoidCommand.LEDarray[ptrData].state,"blink"))
                    body.led[i].state=2;
                
                // R�cup�ration des consignes dans le message (si disponible)
                if(AlgoidCommand.LEDarray[ptrData].powerPercent > 0)
                    body.led[i].power=AlgoidCommand.LEDarray[ptrData].powerPercent;
                
                if(AlgoidCommand.LEDarray[ptrData].time > 0)
                    body.led[i].blinkTime=AlgoidCommand.LEDarray[ptrData].time;
                
                if(AlgoidCommand.LEDarray[ptrData].blinkCount > 0)
                    body.led[i].blinkCount=AlgoidCommand.LEDarray[ptrData].blinkCount*2;
            }
        }

        
        // VERIFIE L'EXISTANCE DE PARAMETRE DE TYPE LED, CREATION DU NOMBRE D'ACTION ADEQUAT
        // 
        if(actionCount>0){
            // Ouverture d'une t�che pour les toutes les actions du message algoid � effectuer
            // Recois un num�ro de tache en retour
            myTaskId=createBuggyTask(AlgoidCommand.msgID, actionCount);			//

            // D�marrage des actions
            if(myTaskId>0){
                    printf("Creation de tache LED: #%d avec %d actions\n", myTaskId, actionCount);

                    // Sauvegarde du nom de l'emetteur et du ID du message pour la r�ponse
                    // en fin d'�venement
                    saveSenderOfMsgId(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom);
                    
                    for(ptrData=0; action < actionCount && ptrData<10; ptrData++){
                            ID = AlgoidCommand.LEDarray[ptrData].id;
                            if(ID >= 0){
                                    time=AlgoidCommand.LEDarray[ptrData].time;
                                    power=AlgoidCommand.LEDarray[ptrData].powerPercent;
                                    Count=AlgoidCommand.LEDarray[ptrData].blinkCount;

                                    // Mode blink
                                    if(body.led[ID].state==2){
                                        // Creation d'un timer effectu� sans erreur, ni ecrasement de timer
                                         setAsyncLedAction(myTaskId, ID, time, Count);
;                                    }

                                    // Mode on ou off
                                    else{
                                            if(body.led[ID].state==0)
                                                setLedPower(ID, 0); 

                                            if(body.led[ID].state==1)
                                                 setLedPower(ID, body.led[ID].power); 

                                             endOfTask=removeBuggyTask(myTaskId);	// Supprime l'ancienne t�che qui � �t� �cras�e par la nouvelle action

                                            if(endOfTask){
                                                sprintf(reportBuffer, "FIN DES ACTIONS LED pour la tache #%d\n", endOfTask);
                                                int ptr=getSenderFromMsgId(endOfTask);
                                                strcpy(msgTo, msgEventHeader[ptr].msgFrom);
                                                // Lib�re la memorisation de l'expediteur
                                                removeSenderOfMsgId(endOfTask);
                                                AlgoidResponse[0].responseType=0;
                                                sendResponse(endOfTask, msgTo, EVENT, pLED, 1);                         // Envoie un message ALGOID de fin de t�che pour l'action �cras�
                                            }
                                                printf(reportBuffer);							// Affichage du message dans le shell
                                                sendMqttReport(endOfTask, reportBuffer);				// Envoie le message sur le canal MQTT "Report"

                                    }

                                    action++;
                            }
                    }
                    AlgoidResponse[0].responseType=1;
                    sendResponse(myTaskId, AlgoidMessageRX.msgFrom, EVENT, pLED, 1);                         // Envoie un message ALGOID de fin de t�che pour l'action �cras�
            }            
        }
        else{   
                sprintf(reportBuffer, "ERREUR: ID LED INEXISTANT pour le message #%d\n", AlgoidCommand.msgID);
                //AlgoidResponse[0].responseType=-1;
                //sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, RESPONSE, pLED, 1);  // Envoie un message ALGOID de fin de t�che pour l'action �cras�
                printf(reportBuffer);                                                           // Affichage du message dans le shell
                sendMqttReport(AlgoidCommand.msgID, reportBuffer);				// Envoie le message sur le canal MQTT "Report"
        }

        
      
	return 0;
}


// -------------------------------------------------------------------
// makePWMAction
//
// -------------------------------------------------------------------
int runPwmAction(void){
	int ptrData;
	int myTaskId;
	int endOfTask;
        int i;
        int ID;
        
        int time=0;
        int power=0;
        int Count=0;
        
	unsigned char actionCount=0;
	unsigned char action=0;
        
        // R�cup�re l'expediteur original du message ayant provoqu�
        // l'�venement
        char msgTo[32];

        // Recherche s'il y a des param�tres d�fini pour chaque PWM
        // et mise � jour.   
        for(i=0;i<NBPWM;i++){
            ptrData=getPwmSetting(i);
            if(ptrData>=0){
                actionCount++;          // Incr�mente le nombre de param�tres trouv�s = action suppl�mentaire a effectuer
                
                // R�cup�ration de commande d'�tat pour la sortie PWM
                if(!strcmp(AlgoidCommand.PWMarray[ptrData].state,"off"))
                    body.pwm[i].state=0;
                if(!strcmp(AlgoidCommand.PWMarray[ptrData].state,"on"))
                    body.pwm[i].state=1;
                if(!strcmp(AlgoidCommand.PWMarray[ptrData].state,"blink"))
                    body.pwm[i].state=2;
                
                // R�cup�ration des consignes dans le message (si disponible)
                if(AlgoidCommand.PWMarray[ptrData].powerPercent > 0)
                    body.pwm[i].power=AlgoidCommand.PWMarray[ptrData].powerPercent;
                
                if(AlgoidCommand.PWMarray[ptrData].time > 0)
                    body.pwm[i].blinkTime=AlgoidCommand.PWMarray[ptrData].time;
                
                if(AlgoidCommand.PWMarray[ptrData].blinkCount > 0)
                    body.pwm[i].blinkCount=AlgoidCommand.PWMarray[ptrData].blinkCount*2;
            }
        }

        // VERIFIE L'EXISTANCE DE PARAMETRE DE TYPE PWM, CREATION DU NOMBRE D'ACTION ADEQUAT
        // 
        if(actionCount>0){
            // Ouverture d'une t�che pour les toutes les actions du message algoid � effectuer
            // Recois un num�ro de tache en retour
            myTaskId=createBuggyTask(AlgoidCommand.msgID, actionCount);			//

            // D�marrage des actions
            if(myTaskId>0){
                    printf("Creation de tache PWM: #%d avec %d actions\n", myTaskId, actionCount);

                    // Sauvegarde du nom de l'emetteur et du ID du message pour la r�ponse
                    // en fin d'�venement
                    saveSenderOfMsgId(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom);
                    
                    for(ptrData=0; action < actionCount && ptrData<10; ptrData++){
                            ID = AlgoidCommand.PWMarray[ptrData].id;
                            if(ID >= 0){
                                    time=AlgoidCommand.PWMarray[ptrData].time;
                                    power=AlgoidCommand.PWMarray[ptrData].powerPercent;
                                    Count=AlgoidCommand.PWMarray[ptrData].blinkCount;

                                    // Mode blink
                                    if(body.pwm[ID].state==2){
                                        // Creation d'un timer effectu� sans erreur, ni ecrasment de timer
                                         setAsyncPwmAction(myTaskId, ID, time, Count);
                                    }

                                    // Mode on ou off
                                    else{
                                            if(body.pwm[ID].state==0)
                                                setPwmPower(ID, 0); 

                                            if(body.pwm[ID].state==1)
                                                 setPwmPower(ID, body.pwm[ID].power); 

                                             endOfTask=removeBuggyTask(myTaskId);	// Supprime l'ancienne t�che qui � �t� �cras�e par la nouvelle action

                                            if(endOfTask){
                                                sprintf(reportBuffer, "FIN DES ACTIONS PWM pour la tache #%d\n", endOfTask);
                                                int ptr=getSenderFromMsgId(endOfTask);
                                                strcpy(msgTo, msgEventHeader[ptr].msgFrom);
                                                // Lib�re la memorisation de l'expediteur
                                                removeSenderOfMsgId(endOfTask);
                                                AlgoidResponse[0].responseType=0;
                                                sendResponse(endOfTask, msgTo, EVENT, pPWM, 1);                         // Envoie un message ALGOID de fin de t�che pour l'action �cras�
                                            }
                                                printf(reportBuffer);							// Affichage du message dans le shell
                                                sendMqttReport(endOfTask, reportBuffer);				// Envoie le message sur le canal MQTT "Report"

                                    }

                                    action++;
                            }
                    }
                    AlgoidResponse[0].responseType=1;
                    sendResponse(myTaskId, AlgoidMessageRX.msgFrom, EVENT, pPWM, 1);                         // Envoie un message ALGOID de fin de t�che pour l'action �cras�
            }            
        }
        else{   
                sprintf(reportBuffer, "ERREUR: ID PWM INEXISTANT pour le message #%d\n", AlgoidCommand.msgID);
                //AlgoidResponse[0].responseType=-1;
                //sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, RESPONSE, pPWM, 1);                         // Envoie un message ALGOID de fin de t�che pour l'action �cras�
                printf(reportBuffer);							// Affichage du message dans le shell
                sendMqttReport(AlgoidCommand.msgID, reportBuffer);				// Envoie le message sur le canal MQTT "Report"
        }

	return 0;
}


// -------------------------------------------------------------------
// GETWDVALUE
// Recherche dans le message algoid, les param�tres
// [V�locit�, acceleration, sens de rotation et temps d'action] pour la roue sp�cifi�e
// Retourne un pointeur sur le champs de param�tre correspondant au moteur sp�cifi�
// -------------------------------------------------------------------
int getWDvalue(int wheelName){
	int i;
	int searchPtr = -1;

	// V�rifie que le moteur est existant...
		// Recherche dans les donn�e recues la valeur correspondante au param�tre "wheelName"
		for(i=0;i<AlgoidCommand.msgValueCnt;i++){
			if(wheelName == AlgoidCommand.DCmotor[i].motor)
				searchPtr=i;
		}
		return searchPtr;
}

// -------------------------------------------------------------------
// GETSERVOSETTING
// Recherche dans le message algoid, les param�tres
// pour une servomoteur sp�cifi�
// Retourne un pointeur sur le champs de param�tre correspondant au servomoteur sp�cifi�
// -------------------------------------------------------------------
int getServoSetting(int servoName){
	int i;
	int searchPtr = -1;

	// Recherche dans les donn�e recues la valeur correspondante au param�tre "wheelName"
	for(i=0;i<AlgoidCommand.msgValueCnt;i++){
		if(servoName == AlgoidCommand.PWMout[i].id)
		searchPtr=i;
	}
	return searchPtr;
}


// -------------------------------------------------------------------
// GETPWMSETTING
// Recherche dans le message algoid, les param�tres
// pour une PWM sp�cifi�e
// -------------------------------------------------------------------
int getPwmSetting(int name){
	int i;
	int searchPtr = -1;

	// Recherche dans les donn�e recues la valeur correspondante au param�tre "wheelName"
	for(i=0;i<AlgoidCommand.msgValueCnt;i++){
		if(name == AlgoidCommand.PWMarray[i].id)
		searchPtr=i;
	}
	return searchPtr;
}

// -------------------------------------------------------------------
// GETLEDSETTING
// Recherche dans le message algoid, les param�tres
// pour une LED sp�cifi�e
// -------------------------------------------------------------------
int getLedSetting(int name){
	int i;
	int searchPtr = -1;

	// Recherche dans les donn�e recues la valeur correspondante au param�tre "name"
	for(i=0;i<AlgoidCommand.msgValueCnt;i++){
		if(name == AlgoidCommand.LEDarray[i].id)
		searchPtr=i;
	}
	return searchPtr;
}


// -------------------------------------------------------------------
// CREATBUGGYTASK Creation d'une tache avec le nombre
// d'actions � effectuer,
// - Retourne le num�ro d'action attribu�
// - Retourne 0 si table des taches pleine (Impossible de cr�er)
// - Retourne -1 si Message ID existe d�j�
// -------------------------------------------------------------------

int createBuggyTask(int MsgId, int actionCount){
	int i;
	int actionID;


	// d�fini un num�ro de tache al�atoire pour l'action � executer si pas de message id saisi
	if(MsgId == 0){
		actionID = rand() & 0xFFFFFF;
		MsgId = actionID;
	}
	else actionID = MsgId;

	// Recherche un emplacement libre dans la table d'action pour inserer les param�tre
	for(i=0;i<10;i++){
		if(ActionTable[i][TASK_NUMBER]==0){
			ActionTable[i][TASK_NUMBER]=actionID;
			ActionTable[i][ACTION_ALGOID_ID]= MsgId;
			ActionTable[i][ACTION_COUNT]=actionCount;
			return(actionID);
		}else{
			if(ActionTable[i][TASK_NUMBER]==actionID)
			{
                               
				sprintf(reportBuffer, "ERREUR: Tache d�ja existante et en cours de traitement: %d\n", actionID);
                                printf(reportBuffer);
                                AlgoidResponse[0].responseType=0;
                                sendResponse(actionID, getSenderFromMsgId(actionID), RESPONSE, ERR_HEADER, 0);			// Envoie un message ALGOID de fin de t�che pour l'action �cras�
				sendMqttReport(actionID, reportBuffer);
				return -1;
				}
		}
	}
	sprintf(reportBuffer, "ERREUR: Table de t�ches pleine\n");
        printf(reportBuffer);
	sendMqttReport(actionID, reportBuffer);
	return(0);
}

// -------------------------------------------------------------------
// removeBuggyTask
// Mise � jour, soustrait l'action d'une tache
// - Retourne le MESSAGE ID correspondant � la tache si plus d'action � effectuer
// - Retourne 0 si actions restante
// - Retourne -1 si tache inexistante
// -------------------------------------------------------------------

int removeBuggyTask(int actionNumber){
	int i, algoidMsgId;

	// Recherche la tache correspondante dans la t�ble des action
	for(i=0;i<10;i++){
		if(ActionTable[i][TASK_NUMBER]==actionNumber){
			ActionTable[i][ACTION_COUNT]--;
			//printf("UPDATE ACTION %d  reste: %d\n", actionNumber, ActionTable[i][ACTION_COUNT]);
			if((ActionTable[i][ACTION_COUNT]) <=0){
				algoidMsgId=ActionTable[i][ACTION_ALGOID_ID];
				ActionTable[i][TASK_NUMBER]=0;				// Reset/Lib�re l'occupation de la t�che
				ActionTable[i][ACTION_ALGOID_ID]= 0;
				ActionTable[i][ACTION_COUNT]=0;
				return(algoidMsgId);					// Retourn le num�ro d'action termin�
			} else return 0;								// Action non termin�es
		}
	}
	return(-1);												// T�che inexistante
}


// -------------------------------------------------------------------
// MAKESTATUSREQUEST
// Traitement de la requete STATUS
// Envoie une message ALGOID de type "response" avec l'�tat des entr�es DIN, tension batterie, distance sonar, vitesse et distance des roues
// -------------------------------------------------------------------
int makeStatusRequest(void){
	unsigned char i;
	unsigned char ptrData=0;

	AlgoidCommand.msgValueCnt=0;

	AlgoidCommand.msgValueCnt = NBDIN + NBAIN + NBPWM + NBMOTOR; // Nombre de DIN � transmettre

	for(i=0;i<NBDIN;i++){
		AlgoidResponse[ptrData].DINresponse.id=i;
		AlgoidResponse[ptrData].value=body.proximity[i].state;
		ptrData++;
	}

	for(i=0;i<NBAIN;i++){
		AlgoidResponse[ptrData].BATTesponse.id=i;
		AlgoidResponse[ptrData].value=body.battery[i].value;
		ptrData++;
	}

	for(i=0;i<NBPWM;i++){
		AlgoidResponse[ptrData].DISTresponse.id=i;
		AlgoidResponse[ptrData].value=body.distance[i].value;
		ptrData++;
	}

	for(i=0;i<NBMOTOR;i++){
		AlgoidResponse[ptrData].MOTresponse.motor=i;
		AlgoidResponse[ptrData].MOTresponse.velocity=body.motor[i].speed;
		AlgoidResponse[ptrData].MOTresponse.cm=body.motor[i].distance;
		ptrData++;
	}


	// Envoie de la r�ponse MQTT
	sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, RESPONSE, STATUS, AlgoidCommand.msgValueCnt);
	return (1);
}


// -------------------------------------------------------------------
// MAKESENSORREQUEST
// Traitement de la requete SENSORS
// Envoie une message ALGOID de type "response" avec l'�tat des entr�es DIN
// -------------------------------------------------------------------
int makeSensorsRequest(void){
	unsigned char i;

	// Pas de param�tres sp�cifi�s dans le message, retourne l'ensemble des �tats des DIN
	if(AlgoidCommand.msgValueCnt==0){
		AlgoidCommand.msgValueCnt=NBDIN;
		for(i=0;i<NBDIN;i++){
			AlgoidResponse[i].DINresponse.id=i;
		}
	}else
		// ENREGISTREMENT DES NOUVEAUX PARAMETRES RECUS
		for(i=0;i<AlgoidCommand.msgValueCnt; i++){
			AlgoidResponse[i].DINresponse.id = AlgoidCommand.DINsens[i].id;
			// Contr�le que le capteur soit pris en charge
			if(AlgoidCommand.DINsens[i].id < NBDIN){
				// Recherche de param�tres suppl�mentaires et enregistrement des donn�e en "local"
				if(!strcmp(AlgoidCommand.DINsens[i].event_state, "on"))	body.proximity[AlgoidCommand.DINsens[i].id].event_enable=1; 			// Activation de l'envoie de messages sur �venements
				else if(!strcmp(AlgoidCommand.DINsens[i].event_state, "off"))	body.proximity[AlgoidCommand.DINsens[i].id].event_enable=0;    // D�sactivation de l'envoie de messages sur �venements

				if(!strcmp(AlgoidCommand.DINsens[i].safetyStop_state, "on"))	body.proximity[AlgoidCommand.DINsens[i].id].safetyStop_state=1; 			// Activation de l'envoie de messages sur �venements
				else if(!strcmp(AlgoidCommand.DINsens[i].safetyStop_state, "off"))	body.proximity[AlgoidCommand.DINsens[i].id].safetyStop_state=0;    // D�sactivation de l'envoie de messages sur �venemen

				body.proximity[AlgoidCommand.DINsens[i].id].safetyStop_value = AlgoidCommand.DINsens[i].safetyStop_value;
			} else
				AlgoidResponse[i].value = -1;
		};

	// RETOURNE EN REPONSE LES PARAMETRES ENREGISTRES ---
	for(i=0;i<AlgoidCommand.msgValueCnt;i++){
		int temp = AlgoidResponse[i].DINresponse.id;

		// Contr�le que le capteur soit pris en charge
		if(AlgoidCommand.DINsens[i].id < NBDIN){
			AlgoidResponse[i].value = body.proximity[temp].state;
			if(body.proximity[temp].event_enable) strcpy(AlgoidResponse[i].DINresponse.event_state, "on");
				else strcpy(AlgoidResponse[i].DINresponse.event_state, "off");

			if(body.proximity[temp].safetyStop_state) strcpy(AlgoidResponse[i].DINresponse.safetyStop_state, "on");
				else strcpy(AlgoidResponse[i].DINresponse.safetyStop_state, "off");
			AlgoidResponse[i].DINresponse.safetyStop_value = body.proximity[temp].safetyStop_value;
		} else
			AlgoidResponse[i].value = -1;
	//---
	}
	// Envoie de la r�ponse MQTT
	sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, RESPONSE, DINPUT, AlgoidCommand.msgValueCnt);
	return (1);
}

// -------------------------------------------------------------------
// MAKEDISTANCEREQUEST
// Traitement de la requete de mesure de distance
// // R�cup�re les valeurs des param�tres "EVENT", "EVENT_HIGH", "EVENT_LOW", ANGLE
// Envoie un message ALGOID de type "response" avec la valeur distance mesur�e
// -------------------------------------------------------------------
int makeDistanceRequest(void){
	unsigned char i;

	// Pas de param�tres sp�cifi� dans le message, retourne l'ensemble des distances
	if(AlgoidCommand.msgValueCnt==0){
		AlgoidCommand.msgValueCnt=NBSONAR;
		for(i=0;i<NBSONAR;i++){
			AlgoidResponse[i].DISTresponse.id=i;
		}
	}else
			// ENREGISTREMENT DES NOUVEAUX PARAMETRES RECUS
			for(i=0;i<AlgoidCommand.msgValueCnt; i++){
				AlgoidResponse[i].DISTresponse.id=AlgoidCommand.DISTsens[i].id;

				if(AlgoidCommand.DISTsens[i].id <NBSONAR){

					// Activation de l'envoie de messages sur �venements
					if(!strcmp(AlgoidCommand.DISTsens[i].event_state, "on")){
							body.distance[AlgoidCommand.DISTsens[i].id].event_enable=1;
							saveSenderOfMsgId(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom);
					}
					else if(!strcmp(AlgoidCommand.DISTsens[i].event_state, "off")){
						body.distance[AlgoidCommand.DISTsens[i].id].event_enable=0;
						removeSenderOfMsgId(AlgoidCommand.msgID);
					}

					// Evemenent haut
					if(AlgoidCommand.DISTsens[i].event_high!=0)
						body.distance[AlgoidCommand.DISTsens[i].id].event_high=AlgoidCommand.DISTsens[i].event_high;
					// Evemenent bas
					if(AlgoidCommand.DISTsens[i].event_low!=0)
						body.distance[AlgoidCommand.DISTsens[i].id].event_low=AlgoidCommand.DISTsens[i].event_low;

					if(!strcmp(AlgoidCommand.DISTsens[i].safetyStop_state, "on")) body.distance[AlgoidCommand.DISTsens[i].id].safetyStop_state=1;
					else if(!strcmp(AlgoidCommand.DISTsens[i].safetyStop_state, "off")) body.distance[AlgoidCommand.DISTsens[i].id].safetyStop_state=0;
					body.distance[AlgoidCommand.DISTsens[i].id].safetyStop_value = AlgoidCommand.DISTsens[i].safetyStop_value;
				} else
					AlgoidResponse[i].value = -1;
			};

	for(i=0;i<AlgoidCommand.msgValueCnt; i++){
		// RETOURNE EN REPONSE LES PARAMETRES ENREGISTRES
		// R�cup�ration des param�tres actuels et chargement du buffer de r�ponse
		int temp = AlgoidResponse[i].DISTresponse.id;

		if(AlgoidCommand.DISTsens[i].id <NBSONAR){
			AlgoidResponse[i].value=body.distance[temp].value;
			//AlgoidResponse[i].DISTresponse.angle=angle[AlgoidCommand.DISTsens[i].angle];

			if(body.distance[temp].event_enable)strcpy(AlgoidResponse[i].DISTresponse.event_state, "on");
			else strcpy(AlgoidResponse[i].DISTresponse.event_state, "off");
			AlgoidResponse[i].DISTresponse.event_high=body.distance[temp].event_high;
			AlgoidResponse[i].DISTresponse.event_low=body.distance[temp].event_low;

			if(body.distance[temp].safetyStop_state)strcpy(AlgoidResponse[i].DISTresponse.safetyStop_state, "on");
			else strcpy(AlgoidResponse[i].DISTresponse.safetyStop_state, "off");
			AlgoidResponse[i].DISTresponse.safetyStop_value=body.distance[temp].safetyStop_value;
		} else
			AlgoidResponse[i].value = -1;
	};

	// Envoie de la r�ponse MQTT
	sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, RESPONSE, DISTANCE, AlgoidCommand.msgValueCnt);

		return 1;
}


// -------------------------------------------------------------------
// MAKEBATTERYREQUEST
// Traitement de la requete de mesure de tension batterie
// R�cup�re les valeurs des param�tres "EVENT", "EVENT_HIGH", "EVENT_LOW"
// Envoie un message ALGOID de type "response" avec la valeur des param�tres enregistr�s
// -------------------------------------------------------------------

int makeBatteryRequest(void){
	unsigned char i;

	// Pas de param�tres sp�cifi� dans le message, retourne l'ensemble des �tats des batteries
	if(AlgoidCommand.msgValueCnt==0){
		AlgoidCommand.msgValueCnt=1;
		for(i=0;i<2;i++){
			AlgoidResponse[i].BATTesponse.id=i;
		}
	}else
			for(i=0;i<AlgoidCommand.msgValueCnt; i++){
				AlgoidResponse[i].BATTesponse.id=AlgoidCommand.BATTsens[i].id;

				if(AlgoidCommand.BATTsens[i].id <NBAIN){
					// ENREGISTREMENT DES NOUVEAUX PARAMETRES RECUS
					// Recherche de param�tres suppl�mentaires
					// Evenement activ�es
					if(!strcmp(AlgoidCommand.BATTsens[i].event_state, "on")){
						body.battery[AlgoidCommand.BATTsens[i].id].event_enable=1;
						saveSenderOfMsgId(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom);
					}
					else if(!strcmp(AlgoidCommand.BATTsens[i].event_state, "off")){
						body.battery[AlgoidCommand.BATTsens[i].id].event_enable=0;
						removeSenderOfMsgId(AlgoidCommand.msgID);
					}
					// Evemenent haut
					if(AlgoidCommand.BATTsens[i].event_high!=0) body.battery[AlgoidCommand.BATTsens[i].id].event_high=AlgoidCommand.BATTsens[i].event_high;
					if(AlgoidCommand.BATTsens[i].event_high!=0) body.battery[AlgoidCommand.BATTsens[i].id].event_low=AlgoidCommand.BATTsens[i].event_low;

					if(!strcmp(AlgoidCommand.BATTsens[i].safetyStop_state, "on")) body.battery[AlgoidCommand.BATTsens[i].id].safetyStop_state=1;
					else if(!strcmp(AlgoidCommand.BATTsens[i].safetyStop_state, "off")) body.battery[AlgoidCommand.BATTsens[i].id].safetyStop_state=0;
					if(AlgoidCommand.BATTsens[i].safetyStop_value!=0) body.battery[AlgoidCommand.BATTsens[i].id].safetyStop_value=AlgoidCommand.BATTsens[i].safetyStop_value;
				}else
					AlgoidResponse[i].value = -1;
			};

	for(i=0;i<AlgoidCommand.msgValueCnt; i++){
		// RETOURNE EN REPONSE LES PARAMETRES ENREGISTRES
		int temp = AlgoidResponse[i].BATTesponse.id;

		if(AlgoidCommand.BATTsens[i].id <NBAIN){
			AlgoidResponse[i].value=body.battery[temp].value;

			if(body.battery[temp].event_enable){
				strcpy(AlgoidResponse[i].BATTesponse.event_state, "on");
				saveSenderOfMsgId(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom);
			}
			else{
				strcpy(AlgoidResponse[i].BATTesponse.event_state, "off");
				removeSenderOfMsgId(AlgoidCommand.msgID);
			}
			AlgoidResponse[i].BATTesponse.event_high=body.battery[temp].event_high;
			AlgoidResponse[i].BATTesponse.event_low=body.battery[temp].event_low;

			if(body.battery[temp].safetyStop_state)strcpy(AlgoidResponse[i].BATTesponse.safetyStop_state, "on");
			else strcpy(AlgoidResponse[i].BATTesponse.safetyStop_state, "off");
			AlgoidResponse[i].BATTesponse.safetyStop_value=body.battery[temp].safetyStop_value;
		} else
			AlgoidResponse[i].value = -1;
	};
	// Envoie de la r�ponse MQTT
	sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, RESPONSE, BATTERY, AlgoidCommand.msgValueCnt);
		return 1;
}


// -------------------------------------------------------------------
// MAKEMOTORREQUEST
// Traitement de la requete SENSORS
// Envoie une message ALGOID de type "response" avec l'�tat des entr�es DIN
// -------------------------------------------------------------------
int makeMotorRequest(void){
	unsigned char i;

	// Pas de param�tres sp�cifi�s dans le message, retourne l'ensemble des �tats des DIN
	if(AlgoidCommand.msgValueCnt==0){
		AlgoidCommand.msgValueCnt=NBMOTOR;
		for(i=0;i<NBMOTOR;i++){
			AlgoidResponse[i].MOTresponse.motor=i;
		}
	}                

	// RETOURNE EN REPONSE LES PARAMETRES ENREGISTRES ---
	for(i=0;i<AlgoidCommand.msgValueCnt;i++){
		int temp = AlgoidResponse[i].MOTresponse.motor;

		// Contr�le que le capteur soit pris en charge
		if(AlgoidCommand.DCmotor[i].motor < NBMOTOR){
			AlgoidResponse[i].MOTresponse.velocity = body.motor[temp].cm;
                        AlgoidResponse[i].MOTresponse.velocity = body.motor[temp].speed;
                        AlgoidResponse[i].responseType=3;
                        
			
		} else
			AlgoidResponse[i].MOTresponse.motor = -1;
	//---
	}
	// Envoie de la r�ponse MQTT
	sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, RESPONSE, MOTORS, AlgoidCommand.msgValueCnt);
	return (1);
}


// -------------------------------------------------------------------
// DISTANCEEVENTCHECK
// Contr�le si la distance mesur�e est hors de la plage d�fini par l'utilisateur
// et envoie un message de type "event" si tel est le cas.
// Un deuxi�me "event" est envoy� lorsque la mesure de distance entre � nouveau dans la
// plage d�finie.
// -------------------------------------------------------------------
void distanceEventCheck(void){
	static unsigned char distWarningSended[1];
	unsigned char i;
	// Contr�le periodique des mesures de distances pour envoie d'evenement
	for(i=0;i<NBPWM;i++){
		// V�rification si envoie des EVENT activ�s
		if(body.distance[i].event_enable){

			int event_low_disable, event_high_disable, distLowDetected, distHighDetected;

			// Contr�le l' individuelle des evenements ( = si valeur < 0)
			if(body.distance[i].event_low < 0) event_low_disable = 1;
			else event_low_disable = 0;

			if(body.distance[i].event_high < 0) event_high_disable = 1;
			else event_high_disable = 0;

			// Detection des seuils d'alarme
			if(body.distance[i].value < body.distance[i].event_low) distLowDetected = 1;
			else distLowDetected = 0;

			if(body.distance[i].value > body.distance[i].event_high) distHighDetected = 1;
			else distHighDetected = 0;

			// Evaluation des alarmes � envoyer
			if((distLowDetected && !event_low_disable) || (distHighDetected && !event_high_disable)){		// Mesure de distance hors plage
				if(distWarningSended[i]==0){													// N'envoie l' event qu'une seule fois
					AlgoidResponse[i].DISTresponse.id=i;
					AlgoidResponse[i].value=body.distance[i].value;
					sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, DISTANCE, NBPWM);
					distWarningSended[i]=1;
				}
			}
			else if (distWarningSended[i]==1){													// Mesure de distance revenu dans la plage
					AlgoidResponse[i].DISTresponse.id=i;									// Et n'envoie qu'une seule fois le message
					AlgoidResponse[i].value=body.distance[i].value;
					sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, DISTANCE, NBPWM);
					distWarningSended[i]=0;
			}
		}
	}
}


// -------------------------------------------------------------------
// BATTERYEVENTCHECK
// Contr�le si la tension mesur�e est hors de la plage d�fini par l'utilisateur
// et envoie un message de type "event" si tel est le cas.
// Un deuxi�me "event" est envoy� lorsque la tension batterie entre � nouveau dans la
// plage d�finie.
// -------------------------------------------------------------------
// -------------------------------------------------------------------
void batteryEventCheck(void){
	static unsigned char battWarningSended[1];
	unsigned char i;
	// Contr�le periodique des mesures de tension batterie pour envoie d'evenement
	for(i=0;i<NBAIN;i++){
		if(body.battery[i].event_enable){

			int event_low_disable, event_high_disable, battLowDetected, battHighDetected;

			// Contr�le l' individuelle des evenements ( = si valeur < 0)
			if(body.battery[i].event_low < 0) event_low_disable = 1;
			else event_low_disable = 0;

			if(body.battery[i].event_high < 0) event_high_disable = 1;
			else event_high_disable = 0;

			// Detection des seuils d'alarme
			if(body.battery[i].value < body.battery[i].event_low) battLowDetected = 1;
			else battLowDetected = 0;

			if(body.battery[i].value > body.battery[i].event_high) battHighDetected = 1;
			else battHighDetected = 0;

			// Evaluation des alarmes � envoyer
			if((battLowDetected && !event_low_disable) || (battHighDetected && !event_high_disable)){				// Mesure tension hors plage
				if(battWarningSended[i]==0){														// N'envoie qu'une seule fois l'EVENT
					AlgoidResponse[i].BATTesponse.id=i;
					AlgoidResponse[i].value=body.battery[i].value;
					sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, BATTERY, 1);
					battWarningSended[i]=1;
				}
			}
			// Envoie un �venement Fin de niveau bas (+50mV Hysterese)
			else if (battWarningSended[i]==1 && body.battery[i].value > (body.battery[i].event_low + body.battery[i].event_hysteresis)){				// Mesure tension dans la plage
					AlgoidResponse[i].BATTesponse.id=i;											// n'envoie qu'une seule fois apr�s
					AlgoidResponse[i].value=body.battery[i].value;											// une hysterese de 50mV
					sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, BATTERY, 1);
					battWarningSended[i]=0;
			}
		}
	}
}


// -------------------------------------------------------------------
// DINEVENTCHECK
// V�rifie si une changement d'�tat � eu lieu sur les entr�es num�riques
// et envoie un event si tel est les cas.
// Seul les DIN ayant chang� d'�tat font partie du message de r�ponse
// -------------------------------------------------------------------
void DINEventCheck(void){
	// Mise � jour de l'�tat des E/S
	unsigned char ptrBuff=0, DINevent=0, oldDinValue[NBDIN], i;

	for(i=0;i<NBDIN;i++){
		// Mise � jour de l'�tat des E/S
		oldDinValue[i]=body.proximity[i].state;
		body.proximity[i].state = getDigitalInput(i);

		// V�rifie si un changement a eu lieu sur les entrees et transmet un message
		// "event" listant les modifications
		if(body.proximity[i].event_enable && (oldDinValue[i] != body.proximity[i].state)){
			AlgoidResponse[ptrBuff].DINresponse.id=i;
			AlgoidResponse[ptrBuff].value=body.proximity[i].state;
			ptrBuff++;
			printf("CHANGEMENT DIN%d, ETAT:%d\n", i, body.proximity[i].state);
			DINevent++;
		}
	}

	if(DINevent>0)
		sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, DINPUT, DINevent);
}


// -------------------------------------------------------------------
// DINSAFETYCHECK
// V�rifie si une changement d'�tat � eu lieu sur les entr�es num�riques
// et effectue une action si safety actif et valeur concordante
// -------------------------------------------------------------------
void DINSafetyCheck(void){
	// Mise � jour de l'�tat des E/S
	unsigned char ptrBuff=0, DINsafety=0, i;
	static unsigned char safetyAction[NBDIN];

	for(i=0;i<NBDIN;i++){
		// Mise � jour de l'�tat des E/S
		body.proximity[i].state = getDigitalInput(i);

		// V�rifie si un changement a eu lieu sur les entrees et transmet un message
		// "event" listant les modifications
		if(body.proximity[i].safetyStop_state){
			if((body.proximity[i].safetyStop_value == body.proximity[i].state)){
				if(!safetyAction[i]){
					ptrBuff++;
					printf("SAFETY DETECTION ON DIN%d, ETAT:%d\n", i, body.proximity[i].state);
					DINsafety++;
					safetyAction[i]=1;
				}
			} else safetyAction[i]=0;
		}
	}
	if(DINsafety>0){
		// ACTION A EFFECTUER
		//sendResponse(AlgoidCommand.msgID, EVENT, DINPUT, DINsafety);
	}
}

// -------------------------------------------------------------------
// BATTERYSAFETYCHECK
// V�rifie si une changement d'�tat � eu lieu sur les entr�es num�riques
// et effectue une action si safety actif et valeur concordante
// -------------------------------------------------------------------
void BatterySafetyCheck(void){
	// Mise � jour de l'�tat des E/S
	unsigned char ptrBuff=0, AINsafety=0, i;
	static unsigned char safetyAction[NBAIN];

	for(i=0;i<NBAIN;i++){
		// Mise � jour de l'�tat des E/S
		body.battery[i].value = getBatteryVoltage();

		// V�rifie si un changement a eu lieu sur les entrees et transmet un message
		// "event" listant les modifications
		if(body.battery[i].safetyStop_state){
			if((body.battery[i].safetyStop_value > body.battery[i].value)){
				if(!safetyAction[i]){
					ptrBuff++;
					printf("SAFETY DETECTION ON BATTERY%d, VALUE:%d\n", i, body.battery[i].value);
					AINsafety++;
					safetyAction[i]=1;
				}
			} else safetyAction[i]=0;
		}
	}
	if(AINsafety>0){
		// ACTION A EFFECTUER
		//sendResponse(AlgoidCommand.msgID, EVENT, DINPUT, DINsafety);
	}
}

// -------------------------------------------------------------------
// DISTANCESAFETYCHECK
// V�rifie si une changement d'�tat � eu lieu sur les entr�es num�riques
// et effectue une action si safety actif et valeur concordante
// -------------------------------------------------------------------
void DistanceSafetyCheck(void){
	// Mise � jour de l'�tat des E/S
	unsigned char ptrBuff=0, DISTsafety=0, i;
	static unsigned char safetyAction[NBPWM];

	for(i=0;i<NBPWM;i++){
		// Mise � jour de l'�tat des E/S
		body.distance[i].value = getSonarDistance();

		// V�rifie si un changement a eu lieu sur les entrees et transmet un message
		// "event" listant les modifications
		if(body.distance[i].safetyStop_state){
			if((body.distance[i].safetyStop_value > body.distance[i].value)){
				if(!safetyAction[i]){
					ptrBuff++;
					printf("SAFETY DETECTION ON SONAR %d, VALUE:%d\n", i, body.distance[i].value);
					DISTsafety++;
					safetyAction[i]=1;
				}
			} else safetyAction[i]=0;
		}
	}
	if(DISTsafety>0){
		// ACTION A EFFECTUER
		//sendResponse(AlgoidCommand.msgID, EVENT, DINPUT, DINsafety);
	}
}
