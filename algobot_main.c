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
void BUTTONEventCheck(void);
void COLOREventCheck(void);

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
int makeStatusRequest(int msgType);
int makeMotorRequest(void);
int makeButtonRequest(void);
int makeRgbRequest(void);

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
t_system sysInfo;
t_sysConfig sysConfig;

// -------------------------------------------------------------------
// MAIN APPLICATION
// - Création de tâche de gestion de la messagerie avec ALGOID, (ALGOID->JSON->MQTT BROCKER->JSON->BUGGY)
// - Création de tâche de gestion des timers pour la commande ON/OFF des roues, de l'accélération des roues, et timer @ 50mS, 100mS, 10Sec
// - Initialisation de la carte de commande hardware pour les moteurs, capteurs, etc...
// - Initialisation d'un broadcast UDP pour publication de la pésence du buggy sur le réseau
// -------------------------------------------------------------------

int main(void) {
	int i;
        int systemDataStreamCounter =0;       // Compteur pour l'envoie periodique du flux de donnees des capteur
                                              // si activé.

	system("clear");
        printf ("ALGOBOT Beta - Build 180622 \n");
        printf ("----------------------------\n");
        
// Création de la tâche pour la gestion de la messagerie avec ALGOID
	if(InitMessager()) printf ("#[CORE] Creation tâche messagerie : ERREUR\n");
	else printf ("#[CORE] Demarrage tache Messager: OK\n");

        
// Création de la tâche pour la gestion des différents timers utilisés
	if(InitTimerManager()) printf ("#[CORE] Creation tâche timer : ERREUR\n");
		else printf ("#[CORE] Demarrage tache timer: OK\n");

// Création de la tâche pour la gestion hardware
	if(InitHwManager()) printf ("#[CORE] Creation tâche hardware : ERREUR\n");
		else {
                    //getHWInfo(systemInfo.firmwareVersion, systemInfo.HWrevision);
                    printf ("#[CORE] Demarrage tache hardware: OK\n");
                }

// Initialisation UDP pour broadcast IP Adresse
	initUDP();
        
// --------------------------------------------------------------------
// BOUCLE DU PROGRAMME PRINCIPAL
// - Messagerie avec ALGOID, attentes de messages en provenance de l'hôte -> Démarrage du traitement des commandes
// - Annonce UDP de présence du buggy sur le réseau  chaque 10Seconde
// - Gestion de l'acceleration du Buggy
// - Mesure sur les capteurs de distance, DIN et batterie
// - Gestion des évenements provoqués par les capteurs
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
        
        for(i=0;i<NBBTN;i++){
		body.button[i].em_stop=0;
		body.button[i].event_enable=0;
		body.button[i].safetyStop_state=0;
		body.button[i].safetyStop_value=0;
	}
        
        for(i=0;i<NBMOTOR;i++){
		body.motor[i].accel=0;
                body.motor[i].cm=0;
                body.motor[i].decel=0;
                body.motor[i].distance=0;
                body.motor[i].speed=0;
                body.motor[i].time=0;
	}
        
        for(i=0;i<NBRGBC;i++){
                body.rgb[i].event_enable=0;
                
		body.rgb[i].red.value=-1;
		body.rgb[i].red.event_low=0;
		body.rgb[i].red.event_high=65535;
                
                body.rgb[i].green.value=-1;
		body.rgb[i].green.event_low=0;
		body.rgb[i].green.event_high=65535;
                
                body.rgb[i].blue.value=-1;
		body.rgb[i].blue.event_low=0;
		body.rgb[i].blue.event_high=65535;
                
                body.rgb[i].clear.value=-1;
		body.rgb[i].clear.event_low=0;
		body.rgb[i].clear.event_high=65535;
	}

        sysInfo.startUpTime=0;
        
        // ------------ Initialisation de la configuration systeme
        
        // Initialisation configuration de flux de données periodique
        sysConfig.statusStream.state=ON;
        sysConfig.statusStream.time_ms=250;
                
	while(1){
            
        // Controle periodique de l'envoie du flux de donnees des capteurs (status)
        if(sysConfig.statusStream.state==ON){
            if(systemDataStreamCounter++ == sysConfig.statusStream.time_ms){
                
                // Retourne un message "Status" sur topic "Stream"
                
                makeStatusRequest(DATAFLOW);
                systemDataStreamCounter=0;
            }
        }

        // Contrôle de la messagerie, recherche d'éventuels messages ALGOID et effectue les traitements nécéssaire
        // selon le type du message [COMMAND, REQUEST, NEGOCIATION, ACK, REPONSE, ERROR, etc...]
        if(pullMsgStack(0)){
                switch(AlgoidCommand.msgType){
                        case COMMAND : processAlgoidCommand(); break;						// Traitement du message de type "COMMAND"
                        case REQUEST : processAlgoidRequest(); break;						// Traitement du message de type "REQUEST"
                        default : ; break;
                }

        }


	// Gestion de la vélocité pour une acceleration proggressive
    	// modification de la vélocité environ chaque 50mS
    	if(checkMotorPowerFlag){
			checkDCmotorPower();													// Contrôle si la vélocité correspond à la consigne
			checkMotorPowerFlag=0;
    	}


		// Contrôle du TIMER 10seconde
    	if(t10secFlag){
    		// Envoie un message UDP sur le réseau, sur port 53530 (CF udpPublish.h)
    		// Avec le ID du buggy (fourni par le gestionnaire de messagerie)
    		char udpMessage[50];
    		sprintf(&udpMessage[0], "[ %s ] I'm here",ClientID);		// Formattage du message avec le Nom du client buggy
    		sendUDPHeartBit(udpMessage);								// Envoie du message
//		printf("\n Send UDP: %s", udpMessage);
    		t10secFlag=0;
    	}
                
        if(t60secFlag){
            sysInfo.startUpTime++;
            t60secFlag=0;
        }


		// Contrôle du TIMER 100mS
    	// - Récupération de la tension de batterie
    	// - Récupération de la distance mesurée au sonar
    	// - Gestion des évenements batterie, digital inputs et distance
    	if(t100msFlag){


			DINEventCheck();										// Contôle de l'état des entrées numérique
															// Génère un évenement si changement d'état détecté

                        BUTTONEventCheck();										// Contôle de l'état des entrées bouton
															// Génère un évenement si changement d'état détecté
                        
                        COLOREventCheck();										// Contôle les valeur RGB des capteurs
                        
			DINSafetyCheck();										// Contôle de l'état des entrées numérique
			BatterySafetyCheck();
			DistanceSafetyCheck(); 										// effectue une action si safety actif


			body.distance[0].value = getSonarDistance();
			distanceEventCheck();										// Provoque un évenement de type "distance" si la distance mesurée
															// est hors de la plage spécifiée par l'utilisateur

			body.battery[0].value = getBatteryVoltage();

                        // Récupération des couleur mesurée sur les capteurs
                        for(i=0;i<NBRGBC;i++){
                            body.rgb[i].red.value=getColorValue(i,RED);
                            body.rgb[i].green.value=getColorValue(i,GREEN);
                            body.rgb[i].blue.value=getColorValue(i,BLUE);
                            body.rgb[i].clear.value=getColorValue(i,CLEAR);
                        }
                        
			batteryEventCheck();

                        for(i=0;i<NBMOTOR;i++){
                            body.motor[i].speed= (getMotorFrequency(i)*CMPP) * body.motor[i].direction;
                            body.motor[i].distance=getMotorPulses(i)*CMPP;
                        }

			// est hors a plage spécifiée par les paramettre utilisateur
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
	int endState=CloseMessager();										// Ferme la tâche de messagerie
	if(!endState)
		  printf( "# ARRET tache Messager - status: %d\n", endState);
	else printf( "# ARRET tache Messager erreur - status: %d\n", endState);

	return EXIT_SUCCESS;												// Fin du programme
}

// -------------------------------------------------------------------
// PROCESSCOMMAND
// Séléctionne et traite le paramètre de commande recue [LL2WD, BACK, FORWARD, STOP, SPIN, etc...]
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
                                            
                                    // Récupération des paramètes de commandes
                                    
                                    // Retourne un message ALGOID si velocité hors tolérences
                                    if((AlgoidCommand.DCmotor[i].velocity < -100) ||(AlgoidCommand.DCmotor[i].velocity > 100)){
                                            AlgoidCommand.DCmotor[i].velocity=0;
                                            AlgoidResponse[i].MOTresponse.velocity=-1;
                                    }else
                                        AlgoidResponse[i].MOTresponse.velocity=AlgoidCommand.DCmotor[i].velocity;
                                    
                                    AlgoidResponse[i].MOTresponse.cm=AlgoidCommand.DCmotor[i].cm;
                                    AlgoidResponse[i].MOTresponse.time=AlgoidCommand.DCmotor[i].time;
                                    AlgoidResponse[i].responseType = RESP_STD_MESSAGE;
                                }
                                // Retourne en réponse le message vérifié
                                sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, RESPONSE, MOTORS, AlgoidCommand.msgValueCnt);  // Retourne une réponse d'erreur, (aucun moteur défini)
                                
                                runMotorAction(); break;			// Action avec en paramètre MOTEUR, VELOCITE, ACCELERATION, TEMPS d'action
                                
                case pPWM  : 	
                                for(i=0;i<AlgoidCommand.msgValueCnt;i++){
                                    // Controle que le moteur existe...
                                    if(AlgoidCommand.PWMarray[i].id >= 0 && AlgoidCommand.PWMarray[i].id <NBPWM)
                                        AlgoidResponse[i].PWMresponse.id=AlgoidCommand.PWMarray[i].id;
                                    else
                                        AlgoidResponse[i].PWMresponse.id=-1;
                                            
                                    // Récupération des paramètes 
                                    strcpy(AlgoidResponse[i].PWMresponse.state, AlgoidCommand.PWMarray[i].state);
                                    AlgoidResponse[i].PWMresponse.powerPercent=AlgoidCommand.PWMarray[i].powerPercent;
                                    AlgoidResponse[i].PWMresponse.blinkCount=AlgoidCommand.PWMarray[i].blinkCount;
                                    AlgoidResponse[i].PWMresponse.time=AlgoidCommand.PWMarray[i].time;
                                    AlgoidResponse[i].responseType = RESP_STD_MESSAGE;
                                }
                                // Retourne en réponse le message vérifié
                                sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, RESPONSE, pPWM, AlgoidCommand.msgValueCnt);     
                    
                                runPwmAction();break;
		case pLED  : 	
                                for(i=0;i<AlgoidCommand.msgValueCnt;i++){
                                    // Controle que le moteur existe...
                                    if(AlgoidCommand.LEDarray[i].id >= 0 && AlgoidCommand.LEDarray[i].id <NBLED)
                                        AlgoidResponse[i].LEDresponse.id=AlgoidCommand.LEDarray[i].id;
                                    else
                                        AlgoidResponse[i].LEDresponse.id=-1;
                                            
                                    // Récupération des paramètes 
                                    strcpy(AlgoidResponse[i].LEDresponse.state, AlgoidCommand.LEDarray[i].state);
                                    AlgoidResponse[i].LEDresponse.powerPercent=AlgoidCommand.LEDarray[i].powerPercent;
                                    AlgoidResponse[i].LEDresponse.blinkCount=AlgoidCommand.LEDarray[i].blinkCount;
                                    AlgoidResponse[i].LEDresponse.time=AlgoidCommand.LEDarray[i].time;
                                    AlgoidResponse[i].responseType = RESP_STD_MESSAGE;
                                }
                                // Retourne en réponse le message vérifié
                                sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, RESPONSE, pLED, AlgoidCommand.msgValueCnt);               
                                
                                runLedAction();break;
		default : break;
	}

	return 0;
}

// -------------------------------------------------------------------
// PROCESSREQUEST
// Séléction et traite le paramètre de requete recu [DISTANCE, TENSION BATTERIE, ENTREE DIGITAL, etc...]
// -------------------------------------------------------------------
int processAlgoidRequest(void){
	switch(AlgoidCommand.msgParam){
		case DISTANCE : makeDistanceRequest();					// Requete de distance
						break;

		case BATTERY :  makeBatteryRequest();					// Requete de tension batterie
						break;

		case DINPUT :	makeSensorsRequest();					// Requete d'état des entrées digitale
						break;
                case BUTTON :	makeButtonRequest();					// Requete d'état des entrées digitale type bouton
						break;

		case STATUS :	makeStatusRequest(RESPONSE);					// Requete d'état du systeme
						break;
        	case MOTORS :	makeMotorRequest();					// Requete commande moteur

                case COLORS :	makeRgbRequest();					// Requete commande moteur
                                                break;
                                                

		default : break;
	}
	return 0;
}


// -------------------------------------------------------------------
// runMotorAction
// Effectue une action avec les paramètre recus: MOTEUR, VELOCITE, ACCELERATION, TEMPS d'action
// -------------------------------------------------------------------
int runMotorAction(void){
	int ptrData;
	int myTaskId;
	unsigned char actionCount=0;
	int action=0;
        int i;
        int ID;

	// Comptabilise le nombre de paramètre (moteur) recu dans le message
	// 
        for(i=0;i<NBMOTOR;i++){
            ptrData=getWDvalue(i);
            if(ptrData>=0){
                actionCount++;   
                        body.motor[i].speed=AlgoidCommand.DCmotor[ptrData].velocity;
                        body.motor[i].accel=AlgoidCommand.DCmotor[ptrData].accel;
                        body.motor[i].decel=AlgoidCommand.DCmotor[ptrData].decel;
                        body.motor[i].cm=AlgoidCommand.DCmotor[ptrData].cm;
                        body.motor[i].time=AlgoidCommand.DCmotor[ptrData].time;
            }
        }

        // Au moin une action à effectuer
        if(actionCount>0){
            // Ouverture d'une tâche pour les toutes les actions du message algoid à effectuer
            // Recois un numéro de tache en retour
            myTaskId=createBuggyTask(AlgoidCommand.msgID, actionCount);			// 2 actions pour mouvement 2WD

            // Démarrage des actions
            if(myTaskId>0){
                    printf("Creation de tache MOTOR: #%d avec %d actions\n", myTaskId, actionCount);

                    // Sauvegarde du nom de l'emetteur et du ID du message pour la réponse
                    // en fin d'évenement
                    saveSenderOfMsgId(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom);

                    for(ptrData=0; action < actionCount && ptrData<10; ptrData++){
                        ID = AlgoidCommand.DCmotor[ptrData].motor;
                        if(ID >= 0){
                            if(body.motor[ID].accel!=0 || body.motor[ID].decel)
                                setMotorAccelDecel(ID, body.motor[ID].accel, body.motor[ID].decel);
                            
                            // Effectue l'action sur la roue
                            if(body.motor[ID].cm <=0 && body.motor[ID].time<=0){                                
                                sprintf(reportBuffer, "ATTENTION: Action infinie, aucun parametre defini \"time\" ou \"cm\" pour l'action sur le moteur %d\n", ID);
                                printf(reportBuffer);                                                             // Affichage du message dans le shell
                                sendMqttReport(AlgoidCommand.msgID, reportBuffer);				      // Envoie le message sur le canal MQTT "Report"     
                                setAsyncMotorAction(myTaskId, ID, body.motor[ID].speed, INFINITE, NULL);
                            }else
                            {
                                if(body.motor[ID].cm > 0)
                                        setAsyncMotorAction(myTaskId, ID, body.motor[ID].speed, CENTIMETER, body.motor[ID].cm);
                                else{
                                        setAsyncMotorAction(myTaskId, ID, body.motor[ID].speed, MILLISECOND, body.motor[ID].time);
                                }
                            }
                             // Défini l'état de laction comme "démarrée" pour message de réponse
                            AlgoidResponse[0].responseType = EVENT_ACTION_BEGIN;
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
        // Aucun paramètre trouvé ou moteur inexistant
        else{
            
            AlgoidResponse[0].responseType = EVENT_ACTION_ERROR;
            sendResponse(myTaskId, AlgoidMessageRX.msgFrom, EVENT, MOTORS, 1);               // Envoie un message EVENT error
            sprintf(reportBuffer, "ERREUR: Aucun moteur défini ou inexistant pour le message #%d\n", AlgoidCommand.msgID);
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

	// Recherche s'il y a des paramètres pour chaque roue
	// Des paramètres recu pour une roue crée une action à effectuer
        for(i=0;i<NBSERVO;i++){
            if(getServoSetting(i)>=0)
                actionCount++;
        }

        if(actionCount>0){
            
            // Ouverture d'une tâche pour les toutes les actions du message algoid à effectuer
            // Recois un numéro de tache en retour
            myTaskId=createBuggyTask(AlgoidCommand.msgID, actionCount);			//

            // Démarrage des actions
            if(myTaskId>0){
                    printf("Creation de tache SERVO: #%d avec %d actions\n", myTaskId, actionCount);

                    // Sauvegarde du nom de l'emetteur et du ID du message pour la réponse
                    // en fin d'évenement
                    saveSenderOfMsgId(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom);

                    for(ptrData=0; action < actionCount && ptrData<10; ptrData++){
                            if(AlgoidCommand.PWMout[ptrData].id>=0){
                                    setServoPosition(AlgoidCommand.PWMout[ptrData].id, AlgoidCommand.PWMout[ptrData].angle);

                                    endOfTask=removeBuggyTask(myTaskId);
                                    if(endOfTask>0){
                                            sprintf(reportBuffer, "FIN DES ACTIONS \"SERVO\" pour la tache #%d\n", endOfTask);

                                            // Récupère l'expediteur original du message ayant provoqué
                                            // l'évenement
                                            char msgTo[32];
                                            int ptr=getSenderFromMsgId(endOfTask);
                                            strcpy(msgTo, msgEventHeader[ptr].msgFrom);
                                            // Libère la memorisation de l'expediteur
                                            removeSenderOfMsgId(endOfTask);

                                            sendResponse(endOfTask, msgTo, EVENT, pPWM, 0);				// Envoie un message ALGOID de fin de tâche pour l'action écrasé
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
            sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, RESPONSE, pSERVO, 1);  // Envoie un message ALGOID de fin de tâche pour l'action écrasé
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
        int i;
        int ID;
        
        int time=-1;
        int power=0;
        int Count=-1;
        
	unsigned char actionCount=0;
	unsigned char action=0;

        // Récupère l'expediteur original du message ayant provoqué
        // l'évenement
        char msgTo[32];
        
        // Recherche s'il y a des paramètres défini pour chaque LED
        // et mise à jour.   
        for(i=0;i<NBLED;i++){
            ptrData=getLedSetting(i);
            if(ptrData>=0){
                actionCount++;          // Incrémente le nombre de paramètres trouvés = action supplémentaire a effectuer
                
                // Récupération de commande d'état de la led dans le message
                if(!strcmp(AlgoidCommand.LEDarray[ptrData].state,"off"))
                    body.led[i].state=LED_OFF;
                if(!strcmp(AlgoidCommand.LEDarray[ptrData].state,"on"))
                    body.led[i].state=LED_ON;
                if(!strcmp(AlgoidCommand.LEDarray[ptrData].state,"blink"))
                    body.led[i].state=LED_BLINK;
                
                // Récupération des consignes dans le message (si disponible)
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
            // Ouverture d'une tâche pour les toutes les actions du message algoid à effectuer
            // Recois un numéro de tache en retour
            myTaskId=createBuggyTask(AlgoidCommand.msgID, actionCount);			//

            // Démarrage des actions
            if(myTaskId>0){
                    printf("Creation de tache LED: #%d avec %d actions\n", myTaskId, actionCount);

                    // Sauvegarde du nom de l'emetteur et du ID du message pour la réponse
                    // en fin d'évenement
                    saveSenderOfMsgId(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom);
                    
                    for(ptrData=0; action < actionCount && ptrData<10; ptrData++){
                            ID = AlgoidCommand.LEDarray[ptrData].id;
                            if(ID >= 0){
                                    power=AlgoidCommand.LEDarray[ptrData].powerPercent;
                                    Count=AlgoidCommand.LEDarray[ptrData].blinkCount;
                                    time=AlgoidCommand.LEDarray[ptrData].time;
                                    // Mode blink
                                    if(body.led[ID].state==LED_BLINK){
                                        
                                        // Verifie la presence de parametres de type "time" et "count", sinon applique des
                                        // valeurs par defaut
                                        if(time<=0){
                                            time=500;
                                            sprintf(reportBuffer, "ATTENTION: Action infinie, aucun parametre defini \"time\"  pour l'action sur la LED %d\n", ID);
                                            printf(reportBuffer);                                                             // Affichage du message dans le shell
                                            sendMqttReport(AlgoidCommand.msgID, reportBuffer);	
                                        }
                                        
                                        if(Count<=0){
                                            sprintf(reportBuffer, "ATTENTION: Action infinie, aucun parametre defini \"count\"  pour l'action sur la LED %d\n", ID);
                                            printf(reportBuffer);                                                             // Affichage du message dans le shell
                                            sendMqttReport(AlgoidCommand.msgID, reportBuffer);				      // Envoie le message sur le canal MQTT "Report"     
                                        }
            
                                        // Creation d'un timer effectué sans erreur, ni ecrasement d'une ancienne action
                                         setAsyncLedAction(myTaskId, ID, INFINITE, time, Count);
;                                    }

                                    // Mode on ou off
                                    else{
                                            if(body.led[ID].state==OFF)
                                                setAsyncLedAction(myTaskId, ID, OFF, NULL, NULL);

                                            if(body.led[ID].state==ON)
                                                setAsyncLedAction(myTaskId, ID, ON, NULL, NULL);
                                    }

                                    action++;
                            }
                    }
                    AlgoidResponse[0].responseType=EVENT_ACTION_BEGIN;
                    sendResponse(myTaskId, AlgoidMessageRX.msgFrom, EVENT, pLED, 1);                         // Envoie un message ALGOID de fin de tâche pour l'action écrasé
            }            
        }
        else{   
                sprintf(reportBuffer, "ERREUR: ID LED INEXISTANT pour le message #%d\n", AlgoidCommand.msgID);
                AlgoidResponse[0].responseType=EVENT_ACTION_ERROR;
                sendResponse(myTaskId, AlgoidMessageRX.msgFrom, EVENT, pLED, 1);               // Envoie un message EVENT error
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
        int i;
        int ID;
        
        int time=0;
        int power=0;
        int Count=0;
        
	unsigned char actionCount=0;
	unsigned char action=0;
        
        // Récupère l'expediteur original du message ayant provoqué
        // l'évenement
        char msgTo[32];

        // Recherche s'il y a des paramètres défini pour chaque PWM
        // et mise à jour.   
        for(i=0;i<NBPWM;i++){
            ptrData=getPwmSetting(i);
            if(ptrData>=0){
                actionCount++;          // Incrémente le nombre de paramètres trouvés = action supplémentaire a effectuer
                
                // Récupération de commande d'état pour la sortie PWM
                if(!strcmp(AlgoidCommand.PWMarray[ptrData].state,"off"))
                    body.pwm[i].state=LED_OFF;
                if(!strcmp(AlgoidCommand.PWMarray[ptrData].state,"on"))
                    body.pwm[i].state=LED_ON;
                if(!strcmp(AlgoidCommand.PWMarray[ptrData].state,"blink"))
                    body.pwm[i].state=LED_BLINK;
                
                // Récupération des consignes dans le message (si disponible)
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
            // Ouverture d'une tâche pour les toutes les actions du message algoid à effectuer
            // Recois un numéro de tache en retour
            myTaskId=createBuggyTask(AlgoidCommand.msgID, actionCount);			//

            // Démarrage des actions
            if(myTaskId>0){
                    printf("Creation de tache PWM: #%d avec %d actions\n", myTaskId, actionCount);

                    // Sauvegarde du nom de l'emetteur et du ID du message pour la réponse
                    // en fin d'évenement
                    saveSenderOfMsgId(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom);
                    
                    for(ptrData=0; action < actionCount && ptrData<10; ptrData++){
                            ID = AlgoidCommand.PWMarray[ptrData].id;
                            if(ID >= 0){
                                    power=AlgoidCommand.PWMarray[ptrData].powerPercent;
                                    Count=AlgoidCommand.PWMarray[ptrData].blinkCount;
                                    time=AlgoidCommand.PWMarray[ptrData].time;
                                    // Mode blink
                                    if(body.pwm[ID].state==LED_BLINK){
                                        
                                        // Verifie la presence de parametres de type "time" et "count", sinon applique des
                                        // valeurs par defaut
                                        if(time<=0){
                                            time=500;
                                            sprintf(reportBuffer, "ATTENTION: Action infinie, aucun parametre defini \"time\"  pour l'action PWM %d\n", ID);
                                            printf(reportBuffer);                                                             // Affichage du message dans le shell
                                            sendMqttReport(AlgoidCommand.msgID, reportBuffer);	
                                        }
                                        
                                        if(Count<=0){
                                            sprintf(reportBuffer, "ATTENTION: Action infinie, aucun parametre defini \"count\"  pour l'action PWM %d\n", ID);
                                            printf(reportBuffer);                                                             // Affichage du message dans le shell
                                            sendMqttReport(AlgoidCommand.msgID, reportBuffer);				      // Envoie le message sur le canal MQTT "Report"     
                                        }
            
                                        // Creation d'un timer effectué sans erreur, ni ecrasement d'une ancienne action
                                         setAsyncPwmAction(myTaskId, ID, INFINITE, time, Count);
;                                    }

                                    // Mode on ou off
                                    else{
                                            if(body.pwm[ID].state==OFF)
                                                setAsyncPwmAction(myTaskId, ID, OFF, NULL, NULL);

                                            if(body.pwm[ID].state==ON)
                                                setAsyncPwmAction(myTaskId, ID, ON, NULL, NULL);
                                    }

                                    action++;
                            }
                    }
                    AlgoidResponse[0].responseType=EVENT_ACTION_BEGIN;
                    sendResponse(myTaskId, AlgoidMessageRX.msgFrom, EVENT, pPWM, 1);                         // Envoie un message ALGOID de fin de tâche pour l'action écrasé
            }            
        }
        else{   
                sprintf(reportBuffer, "ERREUR: ID PWM INEXISTANT pour le message #%d\n", AlgoidCommand.msgID);
                AlgoidResponse[0].responseType=EVENT_ACTION_ERROR;
                sendResponse(myTaskId, AlgoidMessageRX.msgFrom, EVENT, pPWM, 1);               // Envoie un message EVENT error
                printf(reportBuffer);							// Affichage du message dans le shell
                sendMqttReport(AlgoidCommand.msgID, reportBuffer);				// Envoie le message sur le canal MQTT "Report"
        }
	return 0;
}


// -------------------------------------------------------------------
// GETWDVALUE
// Recherche dans le message algoid, les paramètres
// [Vélocité, acceleration, sens de rotation et temps d'action] pour la roue spécifiée
// Retourne un pointeur sur le champs de paramètre correspondant au moteur spécifié
// -------------------------------------------------------------------
int getWDvalue(int wheelName){
	int i;
	int searchPtr = -1;

	// Vérifie que le moteur est existant...
		// Recherche dans les donnée recues la valeur correspondante au paramètre "wheelName"
		for(i=0;i<AlgoidCommand.msgValueCnt;i++){
			if(wheelName == AlgoidCommand.DCmotor[i].motor)
				searchPtr=i;
		}
		return searchPtr;
}

// -------------------------------------------------------------------
// GETSERVOSETTING
// Recherche dans le message algoid, les paramètres
// pour une servomoteur spécifié
// Retourne un pointeur sur le champs de paramètre correspondant au servomoteur spécifié
// -------------------------------------------------------------------
int getServoSetting(int servoName){
	int i;
	int searchPtr = -1;

	// Recherche dans les donnée recues la valeur correspondante au paramètre "wheelName"
	for(i=0;i<AlgoidCommand.msgValueCnt;i++){
		if(servoName == AlgoidCommand.PWMout[i].id)
		searchPtr=i;
	}
	return searchPtr;
}


// -------------------------------------------------------------------
// GETPWMSETTING
// Recherche dans le message algoid, les paramètres
// pour une PWM spécifiée
// -------------------------------------------------------------------
int getPwmSetting(int name){
	int i;
	int searchPtr = -1;

	// Recherche dans les donnée recues la valeur correspondante au paramètre "wheelName"
	for(i=0;i<AlgoidCommand.msgValueCnt;i++){
		if(name == AlgoidCommand.PWMarray[i].id)
		searchPtr=i;
	}
	return searchPtr;
}

// -------------------------------------------------------------------
// GETLEDSETTING
// Recherche dans le message algoid, les paramètres
// pour une LED spécifiée
// -------------------------------------------------------------------
int getLedSetting(int name){
	int i;
	int searchPtr = -1;

	// Recherche dans les donnée recues la valeur correspondante au paramètre "name"
	for(i=0;i<AlgoidCommand.msgValueCnt;i++){
		if(name == AlgoidCommand.LEDarray[i].id)
		searchPtr=i;
	}
	return searchPtr;
}


// -------------------------------------------------------------------
// CREATBUGGYTASK Creation d'une tache avec le nombre
// d'actions à effectuer,
// - Retourne le numéro d'action attribué
// - Retourne 0 si table des taches pleine (Impossible de créer)
// - Retourne -1 si Message ID existe déjà
// -------------------------------------------------------------------

int createBuggyTask(int MsgId, int actionCount){
	int i;
	int actionID;


	// défini un numéro de tache aléatoire pour l'action à executer si pas de message id saisi
	if(MsgId == 0){
		actionID = rand() & 0xFFFFFF;
		MsgId = actionID;
	}
	else actionID = MsgId;

	// Recherche un emplacement libre dans la table d'action pour inserer les paramètre
	for(i=0;i<10;i++){
		if(ActionTable[i][TASK_NUMBER]==0){
			ActionTable[i][TASK_NUMBER]=actionID;
			ActionTable[i][ACTION_ALGOID_ID]= MsgId;
			ActionTable[i][ACTION_COUNT]=actionCount;
			return(actionID);
		}else{
			if(ActionTable[i][TASK_NUMBER]==actionID)
			{
                               
				sprintf(reportBuffer, "ERREUR: Tache déja existante et en cours de traitement: %d\n", actionID);
                                printf(reportBuffer);
                                AlgoidResponse[0].responseType=EVENT_ACTION_END;
                                sendResponse(actionID, getSenderFromMsgId(actionID), RESPONSE, ERR_HEADER, 0);			// Envoie un message ALGOID de fin de tâche pour l'action écrasé
				sendMqttReport(actionID, reportBuffer);
				return -1;
                        }
		}
	}
	sprintf(reportBuffer, "ERREUR: Table de tâches pleine\n");
        printf(reportBuffer);
	sendMqttReport(actionID, reportBuffer);
	return(0);
}

// -------------------------------------------------------------------
// removeBuggyTask
// Mise à jour, soustrait l'action d'une tache
// - Retourne le MESSAGE ID correspondant à la tache si plus d'action à effectuer
// - Retourne 0 si actions restante
// - Retourne -1 si tache inexistante
// -------------------------------------------------------------------

int removeBuggyTask(int actionNumber){
	int i, algoidMsgId;

	// Recherche la tache correspondante dans la tâble des action
	for(i=0;i<10;i++){
		if(ActionTable[i][TASK_NUMBER]==actionNumber){
			ActionTable[i][ACTION_COUNT]--;
			//printf("UPDATE ACTION %d  reste: %d\n", actionNumber, ActionTable[i][ACTION_COUNT]);
			if((ActionTable[i][ACTION_COUNT]) <=0){
				algoidMsgId=ActionTable[i][ACTION_ALGOID_ID];
				ActionTable[i][TASK_NUMBER]=0;				// Reset/Libère l'occupation de la tâche
				ActionTable[i][ACTION_ALGOID_ID]= 0;
				ActionTable[i][ACTION_COUNT]=0;
				return(algoidMsgId);					// Retourn le numéro d'action terminé
			} else return 0;								// Action non terminées
		}
	}
	return(-1);												// Tâche inexistante
}


// -------------------------------------------------------------------
// MAKESTATUSREQUEST
// Traitement de la requete STATUS
// Envoie une message ALGOID de type "response" avec l'état des entrées DIN, tension batterie, distance sonar, vitesse et distance des roues
// -------------------------------------------------------------------
int makeStatusRequest(int msgType){
	unsigned char i;
	unsigned char ptrData=0;

	AlgoidCommand.msgValueCnt=0;

	AlgoidCommand.msgValueCnt = NBDIN + NBBTN + NBMOTOR + NBSONAR + NBPWM +1 ; // Nombre de VALEUR à transmettre + 1 pour le SystemStatus

        // Retourne le système status
        strcpy(AlgoidResponse[ptrData].SYSresponse.name, ClientID);
        AlgoidResponse[ptrData].SYSresponse.startUpTime=sysInfo.startUpTime;
        
        char fv[10];
        sprintf(fv, "%d", getMcuFirmware());
        char hv[10];
        sprintf(hv, "%d", getMcuHWversion());
        
        strcpy(AlgoidResponse[ptrData].SYSresponse.firmwareVersion,"18.06.02");
        strcpy(AlgoidResponse[ptrData].SYSresponse.mcuVersion,fv);
        strcpy(AlgoidResponse[ptrData].SYSresponse.HWrevision,hv);
        AlgoidResponse[ptrData].SYSresponse.battVoltage=body.battery[0].value;
        ptrData++;
        
	for(i=0;i<NBDIN;i++){
		AlgoidResponse[ptrData].DINresponse.id=i;
		AlgoidResponse[ptrData].value=body.proximity[i].state;
		ptrData++;
	}

        for(i=0;i<NBBTN;i++){
                AlgoidResponse[ptrData].BTNresponse.id=i;
                AlgoidResponse[ptrData].value=body.button[i].state;
                ptrData++;
	}
        
	for(i=0;i<NBMOTOR;i++){
		AlgoidResponse[ptrData].MOTresponse.motor=i;
		AlgoidResponse[ptrData].MOTresponse.velocity=body.motor[i].speed;
		AlgoidResponse[ptrData].MOTresponse.cm=body.motor[i].distance;
		ptrData++;
	}
        
        for(i=0;i<NBSONAR;i++){
                AlgoidResponse[ptrData].DISTresponse.id=i;
                AlgoidResponse[ptrData].value=body.distance[i].value;
                ptrData++;
	}
        
        for(i=0;i<NBPWM;i++){
		AlgoidResponse[ptrData].PWMresponse.id=i;
		AlgoidResponse[ptrData].value=body.pwm[i].state;
                AlgoidResponse[ptrData].PWMresponse.powerPercent=body.pwm[i].power;
		ptrData++;
	}
        
        for(i=0;i<NBRGBC;i++){
		AlgoidResponse[ptrData].RGBresponse.id=i;
		AlgoidResponse[ptrData].RGBresponse.red.value=body.rgb[i].red.value;
                AlgoidResponse[ptrData].RGBresponse.green.value=body.rgb[i].green.value;
                AlgoidResponse[ptrData].RGBresponse.blue.value=body.rgb[i].blue.value;
                AlgoidResponse[ptrData].RGBresponse.blue.value=body.rgb[i].clear.value;
		ptrData++;
	}

        
	// Envoie de la réponse MQTT
	sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, msgType, STATUS, AlgoidCommand.msgValueCnt);
	return (1);
}


// -------------------------------------------------------------------
// MAKESENSORREQUEST
// Traitement de la requete SENSORS
// Envoie une message ALGOID de type "response" avec l'état des entrées DIN
// -------------------------------------------------------------------
int makeSensorsRequest(void){
	unsigned char i;

	// Pas de paramètres spécifiés dans le message, retourne l'ensemble des états des DIN
	if(AlgoidCommand.msgValueCnt==0){
		AlgoidCommand.msgValueCnt=NBDIN;
		for(i=0;i<NBDIN;i++){
			AlgoidResponse[i].DINresponse.id=i;
		}
	}else
		// ENREGISTREMENT DES NOUVEAUX PARAMETRES RECUS
		for(i=0;i<AlgoidCommand.msgValueCnt; i++){
			AlgoidResponse[i].DINresponse.id = AlgoidCommand.DINsens[i].id;
			// Contrôle que le capteur soit pris en charge
			if(AlgoidCommand.DINsens[i].id < NBDIN){
				// Recherche de paramètres supplémentaires et enregistrement des donnée en "local"
				if(!strcmp(AlgoidCommand.DINsens[i].event_state, "on"))	body.proximity[AlgoidCommand.DINsens[i].id].event_enable=1; 			// Activation de l'envoie de messages sur évenements
				else if(!strcmp(AlgoidCommand.DINsens[i].event_state, "off"))	body.proximity[AlgoidCommand.DINsens[i].id].event_enable=0;    // Désactivation de l'envoie de messages sur évenements

				if(!strcmp(AlgoidCommand.DINsens[i].safetyStop_state, "on"))	body.proximity[AlgoidCommand.DINsens[i].id].safetyStop_state=1; 			// Activation de l'envoie de messages sur évenements
				else if(!strcmp(AlgoidCommand.DINsens[i].safetyStop_state, "off"))	body.proximity[AlgoidCommand.DINsens[i].id].safetyStop_state=0;    // Désactivation de l'envoie de messages sur évenemen

				body.proximity[AlgoidCommand.DINsens[i].id].safetyStop_value = AlgoidCommand.DINsens[i].safetyStop_value;
			} else
				AlgoidResponse[i].value = -1;
		};

	// RETOURNE EN REPONSE LES PARAMETRES ENREGISTRES ---
	for(i=0;i<AlgoidCommand.msgValueCnt;i++){
		int temp = AlgoidResponse[i].DINresponse.id;

		// Contrôle que le capteur soit pris en charge
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
	// Envoie de la réponse MQTT
	sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, RESPONSE, DINPUT, AlgoidCommand.msgValueCnt);
	return (1);
}


// -------------------------------------------------------------------
// MAKEBUTTONREQUEST
// Traitement de la requete BOUTON
// Envoie une message ALGOID de type "response" avec l'état des entrées DIN
// -------------------------------------------------------------------
int makeButtonRequest(void){
	unsigned char i;
        
	// Pas de paramètres spécifiés dans le message, retourne l'ensemble des états des DIN
	if(AlgoidCommand.msgValueCnt==0){
		AlgoidCommand.msgValueCnt=NBBTN;
		for(i=0;i<NBBTN;i++){
			AlgoidResponse[i].BTNresponse.id=i;
		}
	}else
		// ENREGISTREMENT DES NOUVEAUX PARAMETRES RECUS
		for(i=0;i<AlgoidCommand.msgValueCnt; i++){
			AlgoidResponse[i].BTNresponse.id = AlgoidCommand.BTNsens[i].id;
			// Contrôle que le capteur soit pris en charge
			if(AlgoidCommand.BTNsens[i].id < NBBTN){
				// Recherche de paramètres supplémentaires et enregistrement des donnée en "local"
				if(!strcmp(AlgoidCommand.BTNsens[i].event_state, "on"))	body.button[AlgoidCommand.BTNsens[i].id].event_enable=1; 			// Activation de l'envoie de messages sur évenements
				else if(!strcmp(AlgoidCommand.BTNsens[i].event_state, "off"))	body.button[AlgoidCommand.BTNsens[i].id].event_enable=0;    // Désactivation de l'envoie de messages sur évenements

				if(!strcmp(AlgoidCommand.BTNsens[i].safetyStop_state, "on"))	body.button[AlgoidCommand.BTNsens[i].id].safetyStop_state=1; 			// Activation de l'envoie de messages sur évenements
				else if(!strcmp(AlgoidCommand.BTNsens[i].safetyStop_state, "off"))	body.button[AlgoidCommand.BTNsens[i].id].safetyStop_state=0;    // Désactivation de l'envoie de messages sur évenemen

				body.button[AlgoidCommand.BTNsens[i].id].safetyStop_value = AlgoidCommand.BTNsens[i].safetyStop_value;
			} else
				AlgoidResponse[i].value = -1;
		};

	// RETOURNE EN REPONSE LES PARAMETRES ENREGISTRES ---
	for(i=0;i<AlgoidCommand.msgValueCnt;i++){
		int temp = AlgoidResponse[i].BTNresponse.id;

		// Contrôle que le capteur soit pris en charge
		if(AlgoidCommand.BTNsens[i].id < NBBTN){
			AlgoidResponse[i].value = body.button[temp].state;
			if(body.button[temp].event_enable) strcpy(AlgoidResponse[i].BTNresponse.event_state, "on");
				else strcpy(AlgoidResponse[i].BTNresponse.event_state, "off");

			if(body.button[temp].safetyStop_state) strcpy(AlgoidResponse[i].BTNresponse.safetyStop_state, "on");
				else strcpy(AlgoidResponse[i].BTNresponse.safetyStop_state, "off");
			AlgoidResponse[i].BTNresponse.safetyStop_value = body.button[temp].safetyStop_value;
		} else
			AlgoidResponse[i].value = -1;
	//---
	}
	// Envoie de la réponse MQTT
	sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, RESPONSE, BUTTON, AlgoidCommand.msgValueCnt);
	return (1);
}

// -------------------------------------------------------------------
// MAKEDISTANCEREQUEST
// Traitement de la requete de mesure de distance
// // Récupère les valeurs des paramètres "EVENT", "EVENT_HIGH", "EVENT_LOW", ANGLE
// Envoie un message ALGOID de type "response" avec la valeur distance mesurée
// -------------------------------------------------------------------
int makeDistanceRequest(void){
	unsigned char i;

	// Pas de paramètres spécifié dans le message, retourne l'ensemble des distances
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

					// Activation de l'envoie de messages sur évenements
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
		// Récupération des paramètres actuels et chargement du buffer de réponse
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

	// Envoie de la réponse MQTT
	sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, RESPONSE, DISTANCE, AlgoidCommand.msgValueCnt);

		return 1;
}

// -------------------------------------------------------------------
// MAKERGBREQUEST
// Traitement de la requete de mesure de couleur
// // Récupère les valeurs des paramètres "EVENT", "EVENT_HIGH", "EVENT_LOW", ANGLE
// Envoie un message ALGOID de type "response" avec les valeurs RGB mesurées
// -------------------------------------------------------------------
int makeRgbRequest(void){
	unsigned char i;

	// Pas de paramètres spécifié dans le message, retourne l'ensemble des capteur RGB
	if(AlgoidCommand.msgValueCnt==0){
		AlgoidCommand.msgValueCnt=NBRGBC;
		for(i=0;i<NBRGBC;i++){
			AlgoidResponse[i].RGBresponse.id=i;
		}
	}else
			// ENREGISTREMENT DES NOUVEAUX PARAMETRES RECUS
			for(i=0;i<AlgoidCommand.msgValueCnt; i++){
				AlgoidResponse[i].RGBresponse.id=AlgoidCommand.RGBsens[i].id;

				if(AlgoidCommand.RGBsens[i].id <NBRGBC){

					// PARAMETRAGE DE L'ENVOIE DES MESSAGES SUR EVENEMENTS.
					if(!strcmp(AlgoidCommand.RGBsens[i].event_state, "on")){
							body.rgb[AlgoidCommand.RGBsens[i].id].event_enable=1;
							saveSenderOfMsgId(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom);
					}
					else if(!strcmp(AlgoidCommand.RGBsens[i].event_state, "off")){
						body.rgb[AlgoidCommand.RGBsens[i].id].event_enable=0;
						removeSenderOfMsgId(AlgoidCommand.msgID);
					}

                                        // Paramètre capteur ROUGE
					// Evemenent haut
					if(AlgoidCommand.RGBsens[i].red.event_high!=0)
						body.rgb[AlgoidCommand.RGBsens[i].id].red.event_high=AlgoidCommand.RGBsens[i].red.event_high;
					// Evemenent bas
					if(AlgoidCommand.RGBsens[i].red.event_low!=0)
						body.rgb[AlgoidCommand.RGBsens[i].id].red.event_low=AlgoidCommand.RGBsens[i].red.event_low;
                                        
                                        // Paramètre capteur VERT
                                        // Evemenent haut
					if(AlgoidCommand.RGBsens[i].green.event_high!=0)
                                            body.rgb[AlgoidCommand.RGBsens[i].id].green.event_high=AlgoidCommand.RGBsens[i].green.event_high;
					// Evemenent bas
					if(AlgoidCommand.RGBsens[i].green.event_low!=0)
						body.rgb[AlgoidCommand.RGBsens[i].id].green.event_low=AlgoidCommand.RGBsens[i].green.event_low;
                                        
                                        // Paramètre capteur BLEU
                                        // Evemenent haut
					if(AlgoidCommand.RGBsens[i].blue.event_high!=0)
						body.rgb[AlgoidCommand.RGBsens[i].id].blue.event_high=AlgoidCommand.RGBsens[i].blue.event_high;
					// Evemenent bas
					if(AlgoidCommand.RGBsens[i].blue.event_low!=0)
                                                body.rgb[AlgoidCommand.RGBsens[i].id].blue.event_low=AlgoidCommand.RGBsens[i].blue.event_low;

                                        // Paramètre capteur CLEAR
                                        // Evemenent haut
					if(AlgoidCommand.RGBsens[i].clear.event_high!=0)
						body.rgb[AlgoidCommand.RGBsens[i].id].clear.event_high=AlgoidCommand.RGBsens[i].clear.event_high;
					// Evemenent bas
					if(AlgoidCommand.RGBsens[i].clear.event_low!=0)
						body.rgb[AlgoidCommand.RGBsens[i].id].clear.event_low=AlgoidCommand.RGBsens[i].clear.event_low;
				} else
					AlgoidResponse[i].value = -1;
			};

	for(i=0;i<AlgoidCommand.msgValueCnt; i++){
		// RETOURNE EN REPONSE LES PARAMETRES ENREGISTRES
		// Récupération des paramètres actuels et chargement du buffer de réponse
		int temp = AlgoidResponse[i].RGBresponse.id;

		if(AlgoidCommand.RGBsens[i].id <NBRGBC){
			AlgoidResponse[i].RGBresponse.red.value=body.rgb[temp].red.value;
                        AlgoidResponse[i].RGBresponse.green.value=body.rgb[temp].green.value;
                        AlgoidResponse[i].RGBresponse.blue.value=body.rgb[temp].blue.value;
                        AlgoidResponse[i].RGBresponse.clear.value=body.rgb[temp].clear.value;

                        // Copie de l'etat de l'evenement
			if(body.rgb[temp].event_enable)strcpy(AlgoidResponse[i].RGBresponse.event_state, "on");
			else strcpy(AlgoidResponse[i].RGBresponse.event_state, "off");
                        
                        // Copie des paramètres évenements haut/bas pour le ROUGE
			AlgoidResponse[i].RGBresponse.red.event_high=body.rgb[temp].red.event_high;
			AlgoidResponse[i].RGBresponse.red.event_low=body.rgb[temp].red.event_low;

                        // Copie des paramètres évenements haut/bas pour le VERT
			AlgoidResponse[i].RGBresponse.green.event_high=body.rgb[temp].green.event_high;
			AlgoidResponse[i].RGBresponse.green.event_low=body.rgb[temp].green.event_low;
                        
                        // Copie des paramètres évenements haut/bas pour le BLEU
			AlgoidResponse[i].RGBresponse.blue.event_high=body.rgb[temp].blue.event_high;
			AlgoidResponse[i].RGBresponse.blue.event_low=body.rgb[temp].blue.event_low;
                        
                        // Copie des paramètres évenements haut/bas pour le CLEAR
			AlgoidResponse[i].RGBresponse.clear.event_high=body.rgb[temp].clear.event_high;
			AlgoidResponse[i].RGBresponse.clear.event_low=body.rgb[temp].clear.event_low;
                        
                        
		} else
			AlgoidResponse[i].value = -1;
	};

	// Envoie de la réponse MQTT
	sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, RESPONSE, COLORS, AlgoidCommand.msgValueCnt);

		return 1;
}

// -------------------------------------------------------------------
// MAKEBATTERYREQUEST
// Traitement de la requete de mesure de tension batterie
// Récupère les valeurs des paramètres "EVENT", "EVENT_HIGH", "EVENT_LOW"
// Envoie un message ALGOID de type "response" avec la valeur des paramètres enregistrés
// -------------------------------------------------------------------

int makeBatteryRequest(void){
	unsigned char i;

	// Pas de paramètres spécifié dans le message, retourne l'ensemble des états des batteries
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
					// Recherche de paramètres supplémentaires
					// Evenement activées
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
	// Envoie de la réponse MQTT
	sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, RESPONSE, BATTERY, AlgoidCommand.msgValueCnt);
		return 1;
}


// -------------------------------------------------------------------
// MAKEMOTORREQUEST
// Traitement de la requete SENSORS
// Envoie une message ALGOID de type "response" avec l'état des entrées DIN
// -------------------------------------------------------------------
int makeMotorRequest(void){
	unsigned char i;

	// Pas de paramètres spécifiés dans le message, retourne l'ensemble des états des moteurs
	if(AlgoidCommand.msgValueCnt==0){
		AlgoidCommand.msgValueCnt=NBMOTOR;
		for(i=0;i<NBMOTOR;i++){
			AlgoidResponse[i].MOTresponse.motor=i;
		}
	}                

	// RETOURNE EN REPONSE LES PARAMETRES ENREGISTRES ---
	for(i=0;i<AlgoidCommand.msgValueCnt;i++){
		int temp = AlgoidResponse[i].MOTresponse.motor;

		// Contrôle que le moteur soit pris en charge
		if(AlgoidCommand.DCmotor[i].motor < NBMOTOR){
                    
			AlgoidResponse[i].MOTresponse.velocity = body.motor[temp].cm;
                        AlgoidResponse[i].MOTresponse.velocity = body.motor[temp].speed;
                        AlgoidResponse[i].responseType=RESP_STD_MESSAGE;
                        
			
		} else
			AlgoidResponse[i].MOTresponse.motor = -1;
	//---
	}
	// Envoie de la réponse MQTT
	sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, RESPONSE, MOTORS, AlgoidCommand.msgValueCnt);
	return (1);
}


// -------------------------------------------------------------------
// DISTANCEEVENTCHECK
// Contrôle si la distance mesurée est hors de la plage défini par l'utilisateur
// et envoie un message de type "event" si tel est le cas.
// Un deuxième "event" est envoyé lorsque la mesure de distance entre à nouveau dans la
// plage définie.
// -------------------------------------------------------------------
void distanceEventCheck(void){
	static unsigned char distWarningSended[1];
	unsigned char i;
	// Contrôle periodique des mesures de distances pour envoie d'evenement
	for(i=0;i<NBPWM;i++){
		// Vérification si envoie des EVENT activés
		if(body.distance[i].event_enable){

			int event_low_disable, event_high_disable, distLowDetected, distHighDetected;

			// Contrôle l' individuelle des evenements ( = si valeur < 0)
			if(body.distance[i].event_low < 0) event_low_disable = 1;
			else event_low_disable = 0;

			if(body.distance[i].event_high < 0) event_high_disable = 1;
			else event_high_disable = 0;

			// Detection des seuils d'alarme
			if(body.distance[i].value < body.distance[i].event_low) distLowDetected = 1;
			else distLowDetected = 0;

			if(body.distance[i].value > body.distance[i].event_high) distHighDetected = 1;
			else distHighDetected = 0;

			// Evaluation des alarmes à envoyer
			if((distLowDetected && !event_low_disable) || (distHighDetected && !event_high_disable)){		// Mesure de distance hors plage
				if(distWarningSended[i]==0){													// N'envoie l' event qu'une seule fois
					AlgoidResponse[i].DISTresponse.id=i;
					AlgoidResponse[i].value=body.distance[i].value;
					sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, DISTANCE, NBSONAR);
					distWarningSended[i]=1;
				}
			}
			else if (distWarningSended[i]==1){													// Mesure de distance revenu dans la plage
					AlgoidResponse[i].DISTresponse.id=i;							// Et n'envoie qu'une seule fois le message
					AlgoidResponse[i].value=body.distance[i].value;
					sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, DISTANCE, NBSONAR);
					distWarningSended[i]=0;
			}
		}
	}
}


// -------------------------------------------------------------------
// BATTERYEVENTCHECK
// Contrôle si la tension mesurée est hors de la plage défini par l'utilisateur
// et envoie un message de type "event" si tel est le cas.
// Un deuxième "event" est envoyé lorsque la tension batterie entre à nouveau dans la
// plage définie.
// -------------------------------------------------------------------
// -------------------------------------------------------------------
void batteryEventCheck(void){
	static unsigned char battWarningSended[1];
	unsigned char i;
	// Contrôle periodique des mesures de tension batterie pour envoie d'evenement
	for(i=0;i<NBAIN;i++){
		if(body.battery[i].event_enable){

			int event_low_disable, event_high_disable, battLowDetected, battHighDetected;

			// Contrôle l' individuelle des evenements ( = si valeur < 0)
			if(body.battery[i].event_low < 0) event_low_disable = 1;
			else event_low_disable = 0;

			if(body.battery[i].event_high < 0) event_high_disable = 1;
			else event_high_disable = 0;

			// Detection des seuils d'alarme
			if(body.battery[i].value < body.battery[i].event_low) battLowDetected = 1;
			else battLowDetected = 0;

			if(body.battery[i].value > body.battery[i].event_high) battHighDetected = 1;
			else battHighDetected = 0;

			// Evaluation des alarmes à envoyer
			if((battLowDetected && !event_low_disable) || (battHighDetected && !event_high_disable)){				// Mesure tension hors plage
				if(battWarningSended[i]==0){														// N'envoie qu'une seule fois l'EVENT
					AlgoidResponse[i].BATTesponse.id=i;
					AlgoidResponse[i].value=body.battery[i].value;
					sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, BATTERY, 1);
					battWarningSended[i]=1;
				}
			}
			// Envoie un évenement Fin de niveau bas (+50mV Hysterese)
			else if (battWarningSended[i]==1 && body.battery[i].value > (body.battery[i].event_low + body.battery[i].event_hysteresis)){				// Mesure tension dans la plage
					AlgoidResponse[i].BATTesponse.id=i;											// n'envoie qu'une seule fois après
					AlgoidResponse[i].value=body.battery[i].value;											// une hysterese de 50mV
					sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, BATTERY, 1);
					battWarningSended[i]=0;
			}
		}
	}
}


// -------------------------------------------------------------------
// DINEVENTCHECK
// Vérifie si une changement d'état à eu lieu sur les entrées numériques
// et envoie un event si tel est les cas.
// Seul les DIN ayant changé d'état font partie du message de réponse
// -------------------------------------------------------------------
void DINEventCheck(void){
	// Mise à jour de l'état des E/S
	unsigned char ptrBuff=0, DINevent=0, oldDinValue[NBDIN], i;

	for(i=0;i<NBDIN;i++){
		// Mise à jour de l'état des E/S
		oldDinValue[i]=body.proximity[i].state;
		body.proximity[i].state = getDigitalInput(i);

		// Vérifie si un changement a eu lieu sur les entrees et transmet un message
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
// BUTTONEVENTCHECK
// Vérifie si une changement d'état à eu lieu sur les bouton
// et envoie un event si tel est les cas.
// Seul les DIN ayant change d'etat font partie du message de reponse
// -------------------------------------------------------------------

void BUTTONEventCheck(void){
	// Mise à jour de l'état des E/S
	unsigned char ptrBuff=0, BTNevent=0, oldBtnValue[NBBTN], i;

	for(i=0;i<NBBTN;i++){
		// Mise à jour de l'état des E/S
		oldBtnValue[i]=body.button[i].state;
		body.button[i].state = getButtonInput(i);

		// Vérifie si un changement a eu lieu sur les entrees et transmet un message
		// "event" listant les modifications
		if(body.button[i].event_enable && (oldBtnValue[i] != body.button[i].state)){
			AlgoidResponse[ptrBuff].BTNresponse.id=i;
			AlgoidResponse[ptrBuff].value=body.button[i].state;
			ptrBuff++;
			printf("CHANGEMENT BOUTON %d, ETAT:%d\n", i, body.button[i].state);
			BTNevent++;
		}
	}

	if(BTNevent>0)
		sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, BUTTON, BTNevent);
}


// -------------------------------------------------------------------
// COLOREVENTCHECK
// Vérifie si une changement d'état à eu lieu sur les entrées numériques
// -------------------------------------------------------------------
void COLOREventCheck(void){
	// Mise à jour de l'état des couleurs des capteur
	static unsigned char RGBWarningSended[1];
	unsigned char i;

	for(i=0;i<NBRGBC;i++){
		if(body.rgb[i].event_enable){

			int red_event_low_disable, red_event_high_disable;                     
                        int redLowDetected, redHighDetected;

			// Contrôle l' individuelle des evenements ( = si valeur < 0)
			if(body.rgb[i].red.event_low < 0) red_event_low_disable = 1;
			else red_event_low_disable = 0;

			if(body.rgb[i].red.event_high < 0) red_event_high_disable = 1;
			else red_event_high_disable = 0;

			// Detection des seuils d'alarme
			if(body.rgb[i].red.value < body.rgb[i].red.event_low) redLowDetected = 1;
			else redLowDetected = 0;

			if(body.rgb[i].red.value > body.rgb[i].red.event_high) redHighDetected = 1;
			else redHighDetected = 0;

			// Evaluation des alarmes à envoyer
			if((redLowDetected && !red_event_low_disable) || (redHighDetected && !red_event_high_disable)){				// Mesure tension hors plage
				if(RGBWarningSended[i]==0){														// N'envoie qu'une seule fois l'EVENT
					AlgoidResponse[i].RGBresponse.id=i;
					AlgoidResponse[i].RGBresponse.red.value=body.rgb[i].red.value;
					sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, COLORS, 1);
					RGBWarningSended[i]=1;
				}
			}
			// Envoie un évenement Fin de niveau bas (+50mV Hysterese)
			else if (RGBWarningSended[i]==1 && body.rgb[i].red.value > (body.rgb[i].red.event_low + body.rgb[i].red.event_hysteresis)){				// Mesure tension dans la plage
					AlgoidResponse[i].RGBresponse.id=i;											// n'envoie qu'une seule fois après
					AlgoidResponse[i].RGBresponse.red.value=body.rgb[i].red.value;											// une hysterese de 50mV
					sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, COLORS, 1);
					RGBWarningSended[i]=0;
			}
		}
	}
 
}

// -------------------------------------------------------------------
// DINSAFETYCHECK
// Vérifie si une changement d'état à eu lieu sur les entrées numériques
// et effectue une action si safety actif et valeur concordante
// -------------------------------------------------------------------
void DINSafetyCheck(void){
	// Mise à jour de l'état des E/S
	unsigned char ptrBuff=0, DINsafety=0, i;
	static unsigned char safetyAction[NBDIN];

	for(i=0;i<NBDIN;i++){
		// Mise à jour de l'état des E/S
		body.proximity[i].state = getDigitalInput(i);

		// Vérifie si un changement a eu lieu sur les entrees et transmet un message
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
// BTNSAFETYCHECK
// Vérifie si une changement d'état à eu lieu sur les boutons
// et effectue une action si safety actif et valeur concordante
// -------------------------------------------------------------------
void BTNSafetyCheck(void){
	// Mise à jour de l'état des E/S
	unsigned char ptrBuff=0, BTNsafety=0, i;
	static unsigned char safetyAction[NBBTN];

	for(i=0;i<NBBTN;i++){
		// Mise à jour de l'état des E/S
		body.button[i].state = getButtonInput(i);

		// Vérifie si un changement a eu lieu sur les entrees et transmet un message
		// "event" listant les modifications
		if(body.button[i].safetyStop_state){
			if((body.button[i].safetyStop_value == body.button[i].state)){
				if(!safetyAction[i]){
					ptrBuff++;
					printf("SAFETY DETECTION ON BUTTON%d, ETAT:%d\n", i, body.button[i].state);
					BTNsafety++;
					safetyAction[i]=1;
				}
			} else safetyAction[i]=0;
		}
	}
	if(BTNsafety>0){
		// ACTION A EFFECTUER
		//sendResponse(AlgoidCommand.msgID, EVENT, DINPUT, DINsafety);
	}
}

// -------------------------------------------------------------------
// BATTERYSAFETYCHECK
// Vérifie si une changement d'état à eu lieu sur les entrées numériques
// et effectue une action si safety actif et valeur concordante
// -------------------------------------------------------------------
void BatterySafetyCheck(void){
	// Mise à jour de l'état des E/S
	unsigned char ptrBuff=0, AINsafety=0, i;
	static unsigned char safetyAction[NBAIN];

	for(i=0;i<NBAIN;i++){
		// Mise à jour de l'état des E/S
		body.battery[i].value = getBatteryVoltage();

		// Vérifie si un changement a eu lieu sur les entrees et transmet un message
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
// Vérifie si une changement d'état à eu lieu sur les entrées numériques
// et effectue une action si safety actif et valeur concordante
// -------------------------------------------------------------------
void DistanceSafetyCheck(void){
	// Mise à jour de l'état des E/S
	unsigned char ptrBuff=0, DISTsafety=0, i;
	static unsigned char safetyAction[NBPWM];

	for(i=0;i<NBPWM;i++){
		// Mise à jour de l'état des E/S
		body.distance[i].value = getSonarDistance();

		// Vérifie si un changement a eu lieu sur les entrees et transmet un message
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
