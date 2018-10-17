#define FIRMWARE_VERSION "1.4.7"

#define DEFAULT_EVENT_STATE 1   

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
#include "asyncSERVO.h"
#include "asyncLED.h"
#include "fileIO.h"

unsigned char ptrSpeedCalc;

int getStartupArg(int count, char *arg[]);

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

int runUpdateCommand(int type);
void runRestartCommand(void);

void resetConfig(void);

char reportBuffer[256];

t_sensor body;
t_system sysInfo;
t_sysConfig sysConfig;

// -------------------------------------------------------------------
// MAIN APPLICATION
// - Cr�ation de t�che de gestion de la messagerie avec ALGOID, (ALGOID->JSON->MQTT BROCKER->JSON->BUGGY)
// - Cr�ation de t�che de gestion des timers pour la commande ON/OFF des roues, de l'acc�l�ration des roues, et timer @ 50mS, 100mS, 10Sec
// - Initialisation de la carte de commande hardware pour les moteurs, capteurs, etc...
// - Initialisation d'un broadcast UDP pour publication de la p�sence du buggy sur le r�seau
// -------------------------------------------------------------------

int main(int argc, char *argv[]) {
	int i;
        int systemDataStreamCounter=0;       // Compteur pour l'envoie periodique du flux de donnees des capteur
                                              // si activ�.
        char welcomeMessage[100];
	system("clear");
        
        getStartupArg(argc, argv);
        
        sprintf(&welcomeMessage[0], "ALGOBOT V%s Build date: %s\n", FIRMWARE_VERSION, __DATE__);		// Formattage du message avec le Nom du client buggy
        printf(welcomeMessage);
        printf ("------------------------------------\n");
        
        
// Cr�ation de la t�che pour la gestion de la messagerie avec ALGOID
	if(InitMessager()) printf ("#[CORE] Creation t�che messagerie : ERREUR\n");
	else printf ("#[CORE] Demarrage tache Messager: OK\n");

        
// Cr�ation de la t�che pour la gestion des diff�rents timers utilis�s
	if(InitTimerManager()) printf ("#[CORE] Creation t�che timer : ERREUR\n");
		else printf ("#[CORE] Demarrage tache timer: OK\n");

// Cr�ation de la t�che pour la gestion hardware
	if(InitHwManager()) printf ("#[CORE] Creation t�che hardware : ERREUR\n");
        else {
            resetHardware();            // Reset les peripheriques hardware                    
            printf ("#[CORE] Demarrage tache hardware: OK\n");
        }

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
        printf("\nTry to load algobot.cfg:\n");
        loadConfigFile();
        
        // Reset configuration to default
        resetConfig();
        
	while(1){
        
        // Check if reset was triggered by user
        if(sysConfig.config.reset>0){
            resetConfig();
            resetHardware();
            systemDataStreamCounter=0;
        }
            
        // Controle periodique de l'envoie du flux de donnees des capteurs (status)
        if(sysConfig.dataStream.state==ON){
            if(systemDataStreamCounter++ >= sysConfig.dataStream.time_ms){
                // Retourne un message "Status" sur topic "Stream"
                makeStatusRequest(DATAFLOW);
                systemDataStreamCounter=0;
            }
        }

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
                
        if(t60secFlag){
            t60secFlag=0;
        }


		// Contr�le du TIMER 100mS
    	// - R�cup�ration de la tension de batterie
    	// - R�cup�ration de la distance mesur�e au sonar
    	// - Gestion des �venements batterie, digital inputs et distance
    	if(t100msFlag){
                        // R�cup�ration des couleur mesur�e sur les capteurs
                        for(i=0;i<NBRGBC;i++){
                            body.rgb[i].red.value=getColorValue(i,RED);
                            body.rgb[i].green.value=getColorValue(i,GREEN);
                            body.rgb[i].blue.value=getColorValue(i,BLUE);
                            body.rgb[i].clear.value=getColorValue(i,CLEAR);
                        }			

                        for(i=0;i<NBMOTOR;i++){
                            
                            body.motor[i].speed= (getMotorFrequency(i)*CMPP) * body.motor[i].direction;
                            //printf("\n----- SPEED #%d:  %d -----\n",i, body.motor[i].speed);
                            body.motor[i].distance=getMotorPulses(i)*CMPP;
                        }

			DINEventCheck();										// Cont�le de l'�tat des entr�es num�rique
															// G�n�re un �venement si changement d'�tat d�tect�

                        BUTTONEventCheck();										// Cont�le de l'�tat des entr�es bouton
															// G�n�re un �venement si changement d'�tat d�tect�
                        
                        COLOREventCheck();										// Cont�le les valeur RGB des capteurs
                        
			DINSafetyCheck();										// Cont�le de l'�tat des entr�es num�rique
			BatterySafetyCheck();
			DistanceSafetyCheck(); 										// effectue une action si safety actif


			body.distance[0].value = getSonarDistance();
			distanceEventCheck();										// Provoque un �venement de type "distance" si la distance mesur�e
															// est hors de la plage sp�cifi�e par l'utilisateur

			body.battery[0].value = getBatteryVoltage();
                        body.battery[0].capacity=(body.battery[0].value-3500)/((4210-3500)/100);
                        batteryEventCheck();
                        


			// est hors a plage sp�cifi�e par les paramettre utilisateur
//			printf("Pulses left: %d    right: %d\n", test[0], test[1]);
			//printf("\nBattery: %d, safetyStop_state: %d safetyStop_value: %d", 0, body.battery[0].safetyStop_state, body.battery[0].safetyStop_value);
//			printf("\nSpeed : G %.1f   D %.1f   ||| Dist G: %.1fcm  Dist D: %.1fcm",
//					body.motor[MOTOR_0].speed, body.motor[MOTOR_1].speed, body.motor[MOTOR_0].distance, body.motor[MOTOR_1].distance);
//			printf(" dist US: %d cm\n", body.distance[0].value);

			t100msFlag=0;												// Quittance le flag 100mS
    	}
        
        sysInfo.startUpTime++;
    	usleep(1000);	// Attente de 1ms
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
    int updateResult;
	switch(AlgoidCommand.msgParam){
		case MOTORS : 	
                                for(i=0;i<AlgoidCommand.msgValueCnt;i++){
                                   
                                    // Controle que le moteur existe...
                                    if(AlgoidCommand.DCmotor[i].motor >= 0 && AlgoidCommand.DCmotor[i].motor <NBMOTOR)
                                        AlgoidResponse[i].MOTresponse.motor=AlgoidCommand.DCmotor[i].motor;
                                    else
                                        AlgoidResponse[i].MOTresponse.motor=-1;
                                            
                                    // Récupération des paramètes de commandes
                                    
                                    // Retourne un message ALGOID si velocit� hors tol�rences
                                    if((AlgoidCommand.DCmotor[i].velocity < -100) ||(AlgoidCommand.DCmotor[i].velocity > 100)){
                                            AlgoidCommand.DCmotor[i].velocity=0;
                                            AlgoidResponse[i].MOTresponse.velocity=-1;
                                    }else
                                        AlgoidResponse[i].MOTresponse.velocity=AlgoidCommand.DCmotor[i].velocity;
                                    
                                    AlgoidResponse[i].MOTresponse.cm=AlgoidCommand.DCmotor[i].cm;
                                    AlgoidResponse[i].MOTresponse.time=AlgoidCommand.DCmotor[i].time;
                                    AlgoidResponse[i].responseType = RESP_STD_MESSAGE;
                                }
                                // Retourne en r�ponse le message v�rifi�
                                sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, RESPONSE, MOTORS, AlgoidCommand.msgValueCnt);  // Retourne une r�ponse d'erreur, (aucun moteur d�fini)
                                
                                runMotorAction(); break;			// Action avec en param�tre MOTEUR, VELOCITE, ACCELERATION, TEMPS d'action
                                
                case pPWM  :
                                for(i=0;i<AlgoidCommand.msgValueCnt;i++){
                                    // Vérification de l'existance de l'index de sortie PWM et défini le mode PWM(FULL)
                                    if(AlgoidCommand.PWMarray[i].id >= 0 && AlgoidCommand.PWMarray[i].id <NBPWM){
                                        AlgoidCommand.PWMarray[i].isServoMode=0;
                                        AlgoidResponse[i].PWMresponse.id=AlgoidCommand.PWMarray[i].id;
                                    }
                                    else
                                        AlgoidResponse[i].PWMresponse.id=-1;
                                            
                                    // R�cup�ration des param�tes 
                                    strcpy(AlgoidResponse[i].PWMresponse.state, AlgoidCommand.PWMarray[i].state);
                                    AlgoidResponse[i].PWMresponse.powerPercent=AlgoidCommand.PWMarray[i].powerPercent;
                                    AlgoidResponse[i].PWMresponse.blinkCount=AlgoidCommand.PWMarray[i].blinkCount;
                                    AlgoidResponse[i].PWMresponse.time=AlgoidCommand.PWMarray[i].time;
                                    AlgoidResponse[i].responseType = RESP_STD_MESSAGE;
                                }

                                // Retourne en r�ponse le message v�rifi�
                                sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, RESPONSE, pPWM, AlgoidCommand.msgValueCnt);     
                    
                                runPwmAction();break;

                case pSERVO  : 
                                for(i=0;i<AlgoidCommand.msgValueCnt;i++){
                                    // Vérification de l'existance de l'index de sortie PWM et défini le mode SERVO
                                    if(AlgoidCommand.PWMarray[i].id >= 0 && AlgoidCommand.PWMarray[i].id <NBPWM){
                                        AlgoidResponse[i].PWMresponse.id=AlgoidCommand.PWMarray[i].id;
                                        AlgoidCommand.PWMarray[i].isServoMode=1;
                                    }
                                    else
                                        AlgoidResponse[i].PWMresponse.id=-1;
                                            
                                    // R�cup�ration des param�tes 
                                    strcpy(AlgoidResponse[i].PWMresponse.state, AlgoidCommand.PWMarray[i].state);

                                    AlgoidResponse[i].PWMresponse.powerPercent=AlgoidCommand.PWMarray[i].powerPercent;
                                    AlgoidResponse[i].responseType = RESP_STD_MESSAGE;
                                }
                                // Retourne en r�ponse le message v�rifi�
                                sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, RESPONSE, pSERVO, AlgoidCommand.msgValueCnt);     
                    
                                runPwmAction();break;
                                
		case pLED  : 	
                                for(i=0;i<AlgoidCommand.msgValueCnt;i++){
                                    // Controle que le moteur existe...
                                    if(AlgoidCommand.LEDarray[i].id >= 0 && AlgoidCommand.LEDarray[i].id <NBLED)
                                        AlgoidResponse[i].LEDresponse.id=AlgoidCommand.LEDarray[i].id;
                                    else
                                        AlgoidResponse[i].LEDresponse.id=-1;
                                            
                                    // R�cup�ration des param�tes 
                                    strcpy(AlgoidResponse[i].LEDresponse.state, AlgoidCommand.LEDarray[i].state);
                                    AlgoidResponse[i].LEDresponse.powerPercent=AlgoidCommand.LEDarray[i].powerPercent;
                                    AlgoidResponse[i].LEDresponse.blinkCount=AlgoidCommand.LEDarray[i].blinkCount;
                                    AlgoidResponse[i].LEDresponse.time=AlgoidCommand.LEDarray[i].time;
                                    AlgoidResponse[i].responseType = RESP_STD_MESSAGE;
                                }
                                // Retourne en r�ponse le message v�rifi�
                                sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, RESPONSE, pLED, AlgoidCommand.msgValueCnt);               
                                
                                runLedAction();
                                break;
                                
            case CONFIG  :
                                for(i=0;i<AlgoidCommand.msgValueCnt;i++){
                                    
                                // CONFIG COMMAND FOR DATASTREAM
                                    // R�cup�re les parametres eventuelle pour la configuration de l'etat de l'envoie du stream par polling
                                    if(!strcmp(AlgoidCommand.Config.stream.state, "on"))
                                        sysConfig.dataStream.state=1; 			// Activation de l'envoie du datastream
                                    else
                                        if(!strcmp(AlgoidCommand.Config.stream.state, "off"))
                                            sysConfig.dataStream.state=0; 		// Desactivation de l'envoie du datastream

                                    
                                    // R�cup�re les parametres eventuelle pour la configuration de l'etat de l'envoie du stream par evenement
                                    if(!strcmp(AlgoidCommand.Config.stream.onEvent, "on"))
                                        sysConfig.dataStream.onEvent=1; 			// Activation de l'envoie du datastream
                                    else
                                        if(!strcmp(AlgoidCommand.Config.stream.onEvent, "off"))
                                            sysConfig.dataStream.onEvent=0; 		// Desactivation de l'envoie du datastr
                                    
                                    if(AlgoidCommand.Config.stream.time>0)
                                        sysConfig.dataStream.time_ms=AlgoidCommand.Config.stream.time;
                                    
                                // CONFIG COMMAND FOR RESET
                                    if(!strcmp(AlgoidCommand.Config.config.reset, "true"))
                                        sysConfig.config.reset=1;
                                                                        
                                    // Préparation des valeurs du message de réponse
                                    AlgoidResponse[i].CONFIGresponse.stream.time=sysConfig.dataStream.time_ms;
                                    if(sysConfig.dataStream.onEvent==0) 
                                        strcpy(AlgoidResponse[i].CONFIGresponse.stream.onEvent, "off");
                                    else strcpy(AlgoidResponse[i].CONFIGresponse.stream.onEvent, "on");

                                    if(sysConfig.dataStream.state==0) 
                                        strcpy(AlgoidResponse[i].CONFIGresponse.stream.state, "off");
                                    else strcpy(AlgoidResponse[i].CONFIGresponse.stream.state, "on");
                                    
                                    if(sysConfig.config.reset==1) 
                                        strcpy(AlgoidResponse[i].CONFIGresponse.config.reset, "true");
                                    else strcpy(AlgoidResponse[i].CONFIGresponse.config.reset, "---");
                                    
                                    AlgoidResponse[i].responseType = RESP_STD_MESSAGE; 
                                }
                                                             
                            // CONFIG COMMAND FOR MOTOR SETTING
                                for(i=0;AlgoidCommand.Config.motor[i].id != -1 && i < NBMOTOR; i++){
                                    // Check if motor exist...
                                    if(AlgoidCommand.Config.motor[i].id >= 0 && AlgoidCommand.Config.motor[i].id <NBMOTOR)
                                        AlgoidResponse[i].CONFIGresponse.motor[0].id=AlgoidCommand.Config.motor[0].id;
                                    else
                                        AlgoidResponse[i].CONFIGresponse.motor[0].id=-1;

                                    if(!strcmp(AlgoidCommand.Config.motor[i].inverted, "true"))
                                        sysConfig.motor[AlgoidCommand.Config.motor[i].id].inverted=1;
                                    else if(!strcmp(AlgoidCommand.Config.motor[i].inverted, "false"))
                                            sysConfig.motor[AlgoidCommand.Config.motor[i].id].inverted=0;
                                }
                                
                                for(i=0;i<NBMOTOR;i++){
                                    printf("\n ------- NEW SETTING ------- Motor ID: %d Inverted: %d \n",i , sysConfig.motor[i].inverted);
                                }                                    
                                
                                    // Préparation des valeurs du message de réponse
                                    AlgoidResponse[i].CONFIGresponse.stream.time=sysConfig.dataStream.time_ms;
                                    if(sysConfig.dataStream.onEvent==0) 
                                        strcpy(AlgoidResponse[i].CONFIGresponse.stream.onEvent, "off");
                                    else strcpy(AlgoidResponse[i].CONFIGresponse.stream.onEvent, "on");

                                    if(sysConfig.dataStream.state==0) 
                                        strcpy(AlgoidResponse[i].CONFIGresponse.stream.state, "off");
                                    else strcpy(AlgoidResponse[i].CONFIGresponse.stream.state, "on");
                                    
                                    if(sysConfig.config.reset==1) 
                                        strcpy(AlgoidResponse[i].CONFIGresponse.config.reset, "true");
                                    else strcpy(AlgoidResponse[i].CONFIGresponse.config.reset, "---");
                                    
                                    AlgoidResponse[i].responseType = RESP_STD_MESSAGE; 
                                                                                            
                                // Retourne en r�ponse le message v�rifi�
                                sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, RESPONSE, CONFIG, AlgoidCommand.msgValueCnt);
                                
                                AlgoidResponse[0].responseType=EVENT_ACTION_BEGIN;
                                sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, EVENT, CONFIG, AlgoidCommand.msgValueCnt);                         // Envoie un message ALGOID de fin de t�che pour l'action �cras�

                                AlgoidResponse[0].responseType=EVENT_ACTION_END;
                                sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, EVENT, CONFIG, AlgoidCommand.msgValueCnt);                         // Envoie un message ALGOID de fin de t�che pour l'action �cras�
  
                                break;
                                
            case SYSTEM :       
                                // RECHERCHE DES MISE A JOURS
                                if(!strcmp(AlgoidCommand.System.application, "check")){
                                    AlgoidResponse[0].responseType=EVENT_ACTION_BEGIN;
                                            
                                    updateResult = runUpdateCommand(0);
                                    
                                    char message[100];
                                    
                                    switch(updateResult){
                                        case 1 :  strcpy(AlgoidResponse[0].SYSCMDresponse.application, "connection error"); break;
                                        case 10 :  strcpy(AlgoidResponse[0].SYSCMDresponse.application, "update available"); break;
                                        case 11 :  strcpy(AlgoidResponse[0].SYSCMDresponse.application, "no update"); break;
                                        default:   
                                                   sprintf(AlgoidResponse[0].SYSCMDresponse.firmwareUpdate, "error %d", updateResult); break;
                                    }

                                    AlgoidResponse[0].responseType = RESP_STD_MESSAGE;
                                    sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, RESPONSE, SYSTEM, AlgoidCommand.msgValueCnt);
                                    // Reset la commande system de type firmware
                                    strcpy(AlgoidCommand.System.application,"");
                                }
                                
                                // MISE A JOUR DE L'APPLICATION
                                if(!strcmp(AlgoidCommand.System.application, "update")){

                                    AlgoidResponse[0].responseType = RESP_STD_MESSAGE;
                                    sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, RESPONSE, SYSTEM, AlgoidCommand.msgValueCnt);
                                    
                                    strcpy(AlgoidResponse[0].SYSCMDresponse.application, "update");
                                    AlgoidResponse[0].responseType=EVENT_ACTION_BEGIN;
 
                                    // Retourne en r�ponse le message v�rifi�
                                    sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, EVENT, SYSTEM, AlgoidCommand.msgValueCnt);
                                    
                                    updateResult = runUpdateCommand(1);
                                  
                                    // FIN DE L'APPLICATION DES CE MOMENT!!!!
                                }
                                
                                // Restart application
                                if(!strcmp(AlgoidCommand.System.application, "restart")){
                                    
                                    strcpy(AlgoidResponse[0].SYSCMDresponse.application, "restart");
                                    AlgoidResponse[0].responseType = RESP_STD_MESSAGE;
                                    sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, RESPONSE, SYSTEM, AlgoidCommand.msgValueCnt);
                                    
                                    AlgoidResponse[0].responseType=EVENT_ACTION_BEGIN;
                                    
                                    usleep(1000);
                                    runRestartCommand();

                                    // FIN DE L'APPLICATION DES CE MOMENT!!!!
                                            
                                }
                                
                                break;
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

		case DINPUT :	makeSensorsRequest();					// Requete d'�tat des entr�es digitale
						break;
                case BUTTON :	makeButtonRequest();					// Requete d'�tat des entr�es digitale type bouton
						break;

		case STATUS :	makeStatusRequest(RESPONSE);				// Requete d'�tat du systeme
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
                        body.motor[i].speed=AlgoidCommand.DCmotor[ptrData].velocity;
                        body.motor[i].accel=AlgoidCommand.DCmotor[ptrData].accel;
                        body.motor[i].decel=AlgoidCommand.DCmotor[ptrData].decel;
                        body.motor[i].cm=AlgoidCommand.DCmotor[ptrData].cm;
                        body.motor[i].time=AlgoidCommand.DCmotor[ptrData].time;
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
                             // D�fini l'�tat de laction comme "d�marr�e" pour message de r�ponse
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
        // Aucun param�tre trouv� ou moteur inexistant
        else{
            
            AlgoidResponse[0].responseType = EVENT_ACTION_ERROR;
            sendResponse(myTaskId, AlgoidMessageRX.msgFrom, EVENT, MOTORS, 1);               // Envoie un message EVENT error
            sprintf(reportBuffer, "ERREUR: Aucun moteur d�fini ou inexistant pour le message #%d\n", AlgoidCommand.msgID);
            printf(reportBuffer);                                                             // Affichage du message dans le shell
            sendMqttReport(AlgoidCommand.msgID, reportBuffer);				      // Envoie le message sur le canal MQTT "Report"
        }
}

// -------------------------------------------------------------------
// runLEDAction
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
                    body.led[i].state=OFF;
                if(!strcmp(AlgoidCommand.LEDarray[ptrData].state,"on"))
                    body.led[i].state=ON;
                if(!strcmp(AlgoidCommand.LEDarray[ptrData].state,"blink"))
                    body.led[i].state=BLINK;
                
                // R�cup�ration des consignes dans le message (si disponible)
                if(AlgoidCommand.LEDarray[ptrData].powerPercent > 0)
                    body.led[i].power=AlgoidCommand.LEDarray[ptrData].powerPercent;
                
                if(AlgoidCommand.LEDarray[ptrData].time > 0)
                    body.led[i].blinkTime=AlgoidCommand.LEDarray[ptrData].time;
                
                if(AlgoidCommand.LEDarray[ptrData].blinkCount > 0)
                    body.led[i].blinkCount=AlgoidCommand.LEDarray[ptrData].blinkCount;
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
                                    power=AlgoidCommand.LEDarray[ptrData].powerPercent;
                                    Count=AlgoidCommand.LEDarray[ptrData].blinkCount;
                                    time=AlgoidCommand.LEDarray[ptrData].time;
                                    // Mode blink
                                    if(body.led[ID].state==BLINK){
                                        
                                        // Verifie la presence de parametres de type "time" et "count", sinon applique des
                                        // valeurs par defaut
                                        if(time<=0){
                                            time=500;
                                            sprintf(reportBuffer, "ATTENTION: Action infinie, aucun parametre defini \"time\"  pour l'action sur la LED %d\n", ID);
                                            printf(reportBuffer);                                                             // Affichage du message dans le shell
                                            sendMqttReport(AlgoidCommand.msgID, reportBuffer);	
                                        }
                                        
                                        if(Count<=0){
                                            Count=1;
                                            sprintf(reportBuffer, "ATTENTION: Action infinie, aucun parametre defini \"count\"  pour l'action sur la LED %d\n", ID);
                                            printf(reportBuffer);                                                             // Affichage du message dans le shell
                                            sendMqttReport(AlgoidCommand.msgID, reportBuffer);				      // Envoie le message sur le canal MQTT "Report"     
                                        }
            
                                        // Creation d'un timer effectu� sans erreur, ni ecrasement d'une ancienne action
                                         setAsyncLedAction(myTaskId, ID, BLINK, time, Count);
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
                    sendResponse(myTaskId, AlgoidMessageRX.msgFrom, EVENT, pLED, 1);                         // Envoie un message ALGOID de fin de t�che pour l'action �cras�
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
// runPWMAction
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
                    body.pwm[i].state=OFF;
                if(!strcmp(AlgoidCommand.PWMarray[ptrData].state,"on"))
                    body.pwm[i].state=ON;
                

                // Blink mode not available in SERVO MODE
                if(!AlgoidCommand.PWMarray[ptrData].isServoMode){
                    if(!strcmp(AlgoidCommand.PWMarray[ptrData].state,"blink"))
                        body.pwm[i].state=BLINK;
                    if(AlgoidCommand.PWMarray[ptrData].time > 0)
                        body.pwm[i].blinkTime=AlgoidCommand.PWMarray[ptrData].time;
                    if(AlgoidCommand.PWMarray[ptrData].blinkCount > 0)
                        body.pwm[i].blinkCount=AlgoidCommand.PWMarray[ptrData].blinkCount;
                }
                else{
                    
                }
                
                // Recuperation des consignes dans le message (si disponible)
                if(AlgoidCommand.PWMarray[ptrData].powerPercent >= 0)
                    body.pwm[i].power=AlgoidCommand.PWMarray[ptrData].powerPercent;
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
                                    power=AlgoidCommand.PWMarray[ptrData].powerPercent;
                                    Count=AlgoidCommand.PWMarray[ptrData].blinkCount;
                                    time=AlgoidCommand.PWMarray[ptrData].time;
                                    
                                    // Check if is a servomotor PWM (500uS .. 2.5mS)
                                    if(!AlgoidCommand.PWMarray[ptrData].isServoMode){
                                        // Mode blink
                                        if(body.pwm[ID].state==BLINK){
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

                                            // Creation d'un timer effectu� sans erreur, ni ecrasement d'une ancienne action
                                             setAsyncPwmAction(myTaskId, ID, BLINK, time, Count);
                                        }
                                        else{
                                            if(body.pwm[ID].state==OFF)
                                                setAsyncPwmAction(myTaskId, ID, OFF, NULL, NULL);

                                            if(body.pwm[ID].state==ON)
                                                setAsyncPwmAction(myTaskId, ID, ON, NULL, NULL);
                                            }
                                    }
                                    else
                                        
                                    {
                                            if(body.pwm[ID].state==OFF)
                                                setAsyncServoAction(myTaskId, ID, OFF, NULL);

                                            if(body.pwm[ID].state==ON)
                                                setAsyncServoAction(myTaskId, ID, ON, NULL);
                                    }
                                    
                                    action++;
                            }
                    }
                    AlgoidResponse[0].responseType=EVENT_ACTION_BEGIN;
                    
                    if(!AlgoidCommand.PWMarray[ptrData].isServoMode)
                        sendResponse(myTaskId, AlgoidMessageRX.msgFrom, EVENT, pPWM, 1);               // Send EVENT message for action begin
                    else
                        sendResponse(myTaskId, AlgoidMessageRX.msgFrom, EVENT, pSERVO, 1);             // Send EVENT message for action begin
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
	int actionID, ptrSender;


	// d�fini un num�ro de tache al�atoire pour l'action � executer si pas de message id saisi
	if(MsgId == 0){
		actionID = rand() & 0xFFFFFF;
		MsgId = actionID;
	}
	else actionID = MsgId;

	// Recherche un emplacement libre dans la table d'action pour inserer les param�tre
	for(i=0;i<50;i++){
		if(ActionTable[i][TASK_NUMBER]==0){
			ActionTable[i][TASK_NUMBER]=actionID;
			ActionTable[i][ACTION_ALGOID_ID]= MsgId;
			ActionTable[i][ACTION_COUNT]=actionCount;
			return(actionID);
		}else{
			if(ActionTable[i][TASK_NUMBER]==actionID)
			{  
				sprintf(reportBuffer, "ERREUR: MessageID / Tache en cours de traitement: #%d\n", actionID);
                                printf(reportBuffer);
                                AlgoidResponse[0].responseType=EVENT_ACTION_END;
                                ptrSender = getSenderFromMsgId(actionID);
                                sendResponse(actionID, msgEventHeader[ptrSender].msgFrom, RESPONSE, ERR_HEADER, 0);			// Envoie un message ALGOID de fin de t�che pour l'action �cras�
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
int makeStatusRequest(int msgType){
	unsigned char i;
	unsigned char ptrData=0;

	AlgoidCommand.msgValueCnt=0;
	AlgoidCommand.msgValueCnt = NBDIN + NBBTN + NBMOTOR + NBSONAR + NBRGBC + NBLED + NBPWM +1 ; // Nombre de VALEUR � transmettre + 1 pour le SystemStatus
     
        // Preparation du message de reponse pour le status systeme
        strcpy(AlgoidResponse[ptrData].SYSresponse.name, ClientID);
        AlgoidResponse[ptrData].SYSresponse.startUpTime=sysInfo.startUpTime;
        AlgoidResponse[ptrData].SYSresponse.wan_online=sysInfo.wan_online;
        AlgoidResponse[ptrData].SYSresponse.rx_message=msg_stats.messageRX;
        AlgoidResponse[ptrData].SYSresponse.tx_message=msg_stats.messageTX;
        
        
        char fv[10];
        sprintf(fv, "%d", getMcuFirmware());
        char hv[10];
        sprintf(hv, "%d", getMcuHWversion());
        
        strcpy(AlgoidResponse[ptrData].SYSresponse.firmwareVersion,FIRMWARE_VERSION);
        strcpy(AlgoidResponse[ptrData].SYSresponse.mcuVersion,fv);
        strcpy(AlgoidResponse[ptrData].SYSresponse.HWrevision,hv);
        AlgoidResponse[ptrData].SYSresponse.battVoltage=body.battery[0].value;
        AlgoidResponse[ptrData].SYSresponse.battPercent=body.battery[0].capacity;
        ptrData++;
        
	for(i=0;i<NBDIN;i++){
		AlgoidResponse[ptrData].DINresponse.id=i;
		AlgoidResponse[ptrData].value=body.proximity[i].state;
                
                if(body.proximity[i].event_enable) strcpy(AlgoidResponse[ptrData].DINresponse.event_state, "on");
                else strcpy(AlgoidResponse[ptrData].DINresponse.event_state, "off");                
		ptrData++;
	}

        for(i=0;i<NBBTN;i++){
                AlgoidResponse[ptrData].BTNresponse.id=i;
                AlgoidResponse[ptrData].value=body.button[i].state;
                
                if(body.button[i].event_enable) strcpy(AlgoidResponse[ptrData].BTNresponse.event_state, "on");
                else strcpy(AlgoidResponse[ptrData].BTNresponse.event_state, "off");
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
                
                if(body.distance[i].event_enable) strcpy(AlgoidResponse[ptrData].DISTresponse.event_state, "on");
                else strcpy(AlgoidResponse[ptrData].DISTresponse.event_state, "off");
                ptrData++;
	}
        
        for(i=0;i<NBRGBC;i++){
		AlgoidResponse[ptrData].RGBresponse.id=i;
		AlgoidResponse[ptrData].RGBresponse.red.value=body.rgb[i].red.value;
                AlgoidResponse[ptrData].RGBresponse.green.value=body.rgb[i].green.value;
                AlgoidResponse[ptrData].RGBresponse.blue.value=body.rgb[i].blue.value;
                AlgoidResponse[ptrData].RGBresponse.clear.value=body.rgb[i].clear.value;
                
                if(body.rgb[i].event_enable) strcpy(AlgoidResponse[ptrData].RGBresponse.event_state, "on");
                else strcpy(AlgoidResponse[ptrData].RGBresponse.event_state, "off");
                
		ptrData++;
	}
        
        for(i=0;i<NBLED;i++){
		AlgoidResponse[ptrData].LEDresponse.id=i;
		AlgoidResponse[ptrData].value=body.led[i].state;
                AlgoidResponse[ptrData].LEDresponse.powerPercent=body.led[i].power;
		ptrData++;
	}
        
        
        for(i=0;i<NBPWM;i++){
		AlgoidResponse[ptrData].PWMresponse.id=i;
		AlgoidResponse[ptrData].value=body.pwm[i].state;
                AlgoidResponse[ptrData].PWMresponse.powerPercent=body.pwm[i].power;
		ptrData++;
	}
                
	// Envoie de la r�ponse MQTT
	sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, msgType, STATUS, AlgoidCommand.msgValueCnt);
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
// MAKEBUTTONREQUEST
// Traitement de la requete BOUTON
// Envoie une message ALGOID de type "response" avec l'�tat des entr�es DIN
// -------------------------------------------------------------------
int makeButtonRequest(void){
	unsigned char i;
        
	// Pas de param�tres sp�cifi�s dans le message, retourne l'ensemble des �tats des DIN
	if(AlgoidCommand.msgValueCnt==0){
		AlgoidCommand.msgValueCnt=NBBTN;
		for(i=0;i<NBBTN;i++){
			AlgoidResponse[i].BTNresponse.id=i;
		}
	}else
		// ENREGISTREMENT DES NOUVEAUX PARAMETRES RECUS
		for(i=0;i<AlgoidCommand.msgValueCnt; i++){
			AlgoidResponse[i].BTNresponse.id = AlgoidCommand.BTNsens[i].id;
			// Contr�le que le capteur soit pris en charge
			if(AlgoidCommand.BTNsens[i].id < NBBTN){
				// Recherche de param�tres suppl�mentaires et enregistrement des donn�e en "local"
				if(!strcmp(AlgoidCommand.BTNsens[i].event_state, "on"))	body.button[AlgoidCommand.BTNsens[i].id].event_enable=1; 			// Activation de l'envoie de messages sur �venements
				else if(!strcmp(AlgoidCommand.BTNsens[i].event_state, "off"))	body.button[AlgoidCommand.BTNsens[i].id].event_enable=0;    // D�sactivation de l'envoie de messages sur �venements

				if(!strcmp(AlgoidCommand.BTNsens[i].safetyStop_state, "on"))	body.button[AlgoidCommand.BTNsens[i].id].safetyStop_state=1; 			// Activation de l'envoie de messages sur �venements
				else if(!strcmp(AlgoidCommand.BTNsens[i].safetyStop_state, "off"))	body.button[AlgoidCommand.BTNsens[i].id].safetyStop_state=0;    // D�sactivation de l'envoie de messages sur �venemen

				body.button[AlgoidCommand.BTNsens[i].id].safetyStop_value = AlgoidCommand.BTNsens[i].safetyStop_value;
			} else
				AlgoidResponse[i].value = -1;
		};

	// RETOURNE EN REPONSE LES PARAMETRES ENREGISTRES ---
	for(i=0;i<AlgoidCommand.msgValueCnt;i++){
		int temp = AlgoidResponse[i].BTNresponse.id;

		// Contr�le que le capteur soit pris en charge
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
	// Envoie de la r�ponse MQTT
	sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, RESPONSE, BUTTON, AlgoidCommand.msgValueCnt);
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
// MAKERGBREQUEST
// Traitement de la requete de mesure de couleur
// // R�cup�re les valeurs des param�tres "EVENT", "EVENT_HIGH", "EVENT_LOW", ANGLE
// Envoie un message ALGOID de type "response" avec les valeurs RGB mesur�es
// -------------------------------------------------------------------
int makeRgbRequest(void){
	unsigned char i;

	// Pas de param�tres sp�cifi� dans le message, retourne l'ensemble des capteur RGB
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

                                        // Param�tre capteur ROUGE
					// Evemenent haut
					if(AlgoidCommand.RGBsens[i].red.event_high!=0)
						body.rgb[AlgoidCommand.RGBsens[i].id].red.event_high=AlgoidCommand.RGBsens[i].red.event_high;
					// Evemenent bas
					if(AlgoidCommand.RGBsens[i].red.event_low!=0)
						body.rgb[AlgoidCommand.RGBsens[i].id].red.event_low=AlgoidCommand.RGBsens[i].red.event_low;
                                        
                                        // Param�tre capteur VERT
                                        // Evemenent haut
					if(AlgoidCommand.RGBsens[i].green.event_high!=0)
                                            body.rgb[AlgoidCommand.RGBsens[i].id].green.event_high=AlgoidCommand.RGBsens[i].green.event_high;
					// Evemenent bas
					if(AlgoidCommand.RGBsens[i].green.event_low!=0)
						body.rgb[AlgoidCommand.RGBsens[i].id].green.event_low=AlgoidCommand.RGBsens[i].green.event_low;
                                        
                                        // Param�tre capteur BLEU
                                        // Evemenent haut
					if(AlgoidCommand.RGBsens[i].blue.event_high!=0)
						body.rgb[AlgoidCommand.RGBsens[i].id].blue.event_high=AlgoidCommand.RGBsens[i].blue.event_high;
					// Evemenent bas
					if(AlgoidCommand.RGBsens[i].blue.event_low!=0)
                                                body.rgb[AlgoidCommand.RGBsens[i].id].blue.event_low=AlgoidCommand.RGBsens[i].blue.event_low;

                                        // Param�tre capteur CLEAR
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
		// R�cup�ration des param�tres actuels et chargement du buffer de r�ponse
		int temp = AlgoidResponse[i].RGBresponse.id;

		if(AlgoidCommand.RGBsens[i].id <NBRGBC){
			AlgoidResponse[i].RGBresponse.red.value=body.rgb[temp].red.value;
                        AlgoidResponse[i].RGBresponse.green.value=body.rgb[temp].green.value;
                        AlgoidResponse[i].RGBresponse.blue.value=body.rgb[temp].blue.value;
                        AlgoidResponse[i].RGBresponse.clear.value=body.rgb[temp].clear.value;

                        // Copie de l'etat de l'evenement
			if(body.rgb[temp].event_enable)strcpy(AlgoidResponse[i].RGBresponse.event_state, "on");
			else strcpy(AlgoidResponse[i].RGBresponse.event_state, "off");
                        
                        // Copie des param�tres �venements haut/bas pour le ROUGE
			AlgoidResponse[i].RGBresponse.red.event_high=body.rgb[temp].red.event_high;
			AlgoidResponse[i].RGBresponse.red.event_low=body.rgb[temp].red.event_low;

                        // Copie des param�tres �venements haut/bas pour le VERT
			AlgoidResponse[i].RGBresponse.green.event_high=body.rgb[temp].green.event_high;
			AlgoidResponse[i].RGBresponse.green.event_low=body.rgb[temp].green.event_low;
                        
                        // Copie des param�tres �venements haut/bas pour le BLEU
			AlgoidResponse[i].RGBresponse.blue.event_high=body.rgb[temp].blue.event_high;
			AlgoidResponse[i].RGBresponse.blue.event_low=body.rgb[temp].blue.event_low;
                        
                        // Copie des param�tres �venements haut/bas pour le CLEAR
			AlgoidResponse[i].RGBresponse.clear.event_high=body.rgb[temp].clear.event_high;
			AlgoidResponse[i].RGBresponse.clear.event_low=body.rgb[temp].clear.event_low;
                        
                        
		} else
			AlgoidResponse[i].value = -1;
	};

	// Envoie de la r�ponse MQTT
	sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, RESPONSE, COLORS, AlgoidCommand.msgValueCnt);

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

	// Pas de param�tres sp�cifi�s dans le message, retourne l'ensemble des �tats des moteurs
	if(AlgoidCommand.msgValueCnt==0){
		AlgoidCommand.msgValueCnt=NBMOTOR;
		for(i=0;i<NBMOTOR;i++){
			AlgoidResponse[i].MOTresponse.motor=i;
		}
	}                

	// RETOURNE EN REPONSE LES PARAMETRES ENREGISTRES ---
	for(i=0;i<AlgoidCommand.msgValueCnt;i++){
		int temp = AlgoidResponse[i].MOTresponse.motor;

		// Contr�le que le moteur soit pris en charge
		if(AlgoidCommand.DCmotor[i].motor < NBMOTOR){
                    
			AlgoidResponse[i].MOTresponse.velocity = body.motor[temp].cm;
                        AlgoidResponse[i].MOTresponse.velocity = body.motor[temp].speed;
                        AlgoidResponse[i].responseType=RESP_STD_MESSAGE;
                        
			
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
	for(i=0;i<NBSONAR;i++){
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
					sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, DISTANCE, NBSONAR);
					distWarningSended[i]=1;
                                        
                                        // Si evenement pour stream activ�, envoie une trame de type status
                                        if(sysConfig.dataStream.onEvent==1)
                                            makeStatusRequest(DATAFLOW);
//                                        printf("CHANGEMENT SONAR%d, VALUE:%d\n", i, body.distance[i].value);
				}
			}
			else if (distWarningSended[i]==1){													// Mesure de distance revenu dans la plage
					AlgoidResponse[i].DISTresponse.id=i;							// Et n'envoie qu'une seule fois le message
					AlgoidResponse[i].value=body.distance[i].value;
					sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, DISTANCE, NBSONAR);
					distWarningSended[i]=0;
                                        
                                        // Si evenement pour stream activ�, envoie une trame de type status
                                        if(sysConfig.dataStream.onEvent==1)
                                            makeStatusRequest(DATAFLOW); 
//                                        printf("CHANGEMENT SONAR%d, VALUE:%d\n", i, body.distance[i].value);
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
                                        
                                        // Si evenement pour stream activ�, envoie une trame de type status
                                        if(sysConfig.dataStream.onEvent==1)
                                            makeStatusRequest(DATAFLOW);                                        
				}
			}
			// Envoie un �venement Fin de niveau bas (+50mV Hysterese)
			else if (battWarningSended[i]==1 && body.battery[i].value > (body.battery[i].event_low + body.battery[i].event_hysteresis)){				// Mesure tension dans la plage
					AlgoidResponse[i].BATTesponse.id=i;											// n'envoie qu'une seule fois apr�s
					AlgoidResponse[i].value=body.battery[i].value;											// une hysterese de 50mV
					sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, BATTERY, 1);
					battWarningSended[i]=0;
                                        
                                        // Si evenement pour stream activ�, envoie une trame de type status
                                        if(sysConfig.dataStream.onEvent==1)
                                            makeStatusRequest(DATAFLOW);                                        
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
			//printf("CHANGEMENT DIN%d, ETAT:%d\n", i, body.proximity[i].state);
			DINevent++;
		}
	}

	if(DINevent>0){
		sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, DINPUT, DINevent);
                
                // Si evenement pour stream activ�, envoie une trame de type status
                if(sysConfig.dataStream.onEvent==1)
                    makeStatusRequest(DATAFLOW);
        }
        

}

// -------------------------------------------------------------------
// BUTTONEVENTCHECK
// V�rifie si une changement d'�tat � eu lieu sur les bouton
// et envoie un event si tel est les cas.
// Seul les DIN ayant change d'etat font partie du message de reponse
// -------------------------------------------------------------------

void BUTTONEventCheck(void){
	// Mise � jour de l'�tat des E/S
	unsigned char ptrBuff=0, BTNevent=0, oldBtnValue[NBBTN], i;

	for(i=0;i<NBBTN;i++){
		// Mise � jour de l'�tat des E/S
		oldBtnValue[i]=body.button[i].state;
		body.button[i].state = getButtonInput(i);

		// V�rifie si un changement a eu lieu sur les entrees et transmet un message
		// "event" listant les modifications
		if(body.button[i].event_enable && (oldBtnValue[i] != body.button[i].state)){
			AlgoidResponse[ptrBuff].BTNresponse.id=i;
			AlgoidResponse[ptrBuff].value=body.button[i].state;
			ptrBuff++;
//			printf("CHANGEMENT BOUTON %d, ETAT:%d\n", i, body.button[i].state);
			BTNevent++;
		}
	}

	if(BTNevent>0){
            sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, BUTTON, BTNevent);
        
            // Si evenement pour stream activ�, envoie une trame de type status
            if(sysConfig.dataStream.onEvent==1)
                makeStatusRequest(DATAFLOW);
        }
}


// -------------------------------------------------------------------
// COLOREVENTCHECK
// V�rifie si une changement d'�tat � eu lieu sur les entr�es num�riques
// -------------------------------------------------------------------
void COLOREventCheck(void){
    
        unsigned char ptrBuff=0, RGBevent=0;
        
	// Mise � jour de l'�tat des couleurs des capteur
	static unsigned char RGB_red_WarningSended[NBRGBC];
        static unsigned char RGB_green_WarningSended[NBRGBC];
        static unsigned char RGB_blue_WarningSended[NBRGBC];
        static unsigned char RGB_clear_WarningSended[NBRGBC];
        
	unsigned char i;

	for(i=0;i<NBRGBC;i++){
		if(body.rgb[i].event_enable){

			int red_event_low_disable, red_event_high_disable;                     
                        int redLowDetected, redHighDetected;
                        
                        int green_event_low_disable, green_event_high_disable;                     
                        int greenLowDetected, greenHighDetected;
                        
                        int blue_event_low_disable, blue_event_high_disable;                     
                        int blueLowDetected, blueHighDetected;

			// Contr�le l' individuelle des evenements sur changement de couleur [ROUGE]
			if(body.rgb[i].red.event_low < 0) red_event_low_disable = 1;
			else red_event_low_disable = 0;

			if(body.rgb[i].red.event_high < 0) red_event_high_disable = 1;
			else red_event_high_disable = 0;

			// Detection des seuils d'alarme
			if(body.rgb[i].red.value < body.rgb[i].red.event_low) redLowDetected = 1;
			else redLowDetected = 0;

			if(body.rgb[i].red.value > body.rgb[i].red.event_high) redHighDetected = 1;
			else redHighDetected = 0;

			// Evaluation des alarmes � envoyer
			if((redLowDetected && !red_event_low_disable) || (redHighDetected && !red_event_high_disable)){				// Mesure tension hors plage
				if(RGB_red_WarningSended[i]==0){														// N'envoie qu'une seule fois l'EVENT
					AlgoidResponse[ptrBuff].RGBresponse.id=i;
					AlgoidResponse[ptrBuff].RGBresponse.red.value=body.rgb[i].red.value;
					ptrBuff++;
                                        RGBevent++;
                                        //sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, COLORS, 1);
					RGB_red_WarningSended[i]=1;
                                        
                                        // Si evenement pour stream activ�, envoie une trame de type status
                                        if(sysConfig.dataStream.onEvent==1)
                                            makeStatusRequest(DATAFLOW);
//                                        printf("CHANGEMENT ROUGE RGB %d, VALUE:%d\n", i, body.rgb[i].red.value);
				}
			}
                        
			// Envoie un �venement Fin de niveau bas (+50mV Hysterese)
			else if (RGB_red_WarningSended[i]==1 && body.rgb[i].red.value > (body.rgb[i].red.event_low + body.rgb[i].red.event_hysteresis)){				// Mesure tension dans la plage
					AlgoidResponse[ptrBuff].RGBresponse.id=i;											// n'envoie qu'une seule fois apr�s
					AlgoidResponse[ptrBuff].RGBresponse.red.value=body.rgb[i].red.value;											// une hysterese de 50mV
                                        ptrBuff++;
                                        RGBevent++;
					//sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, COLORS, 1);
					RGB_red_WarningSended[i]=0;
                                        // Si evenement pour stream activ�, envoie une trame de type status
                                        if(sysConfig.dataStream.onEvent==1)
                                            makeStatusRequest(DATAFLOW);
 //                                        printf("- CHANGEMENT ROUGE RGB %d, VALUE:%d\n", i, body.rgb[i].red.value);
			}
                        
                        // Contr�le l' individuelle des evenements sur changement de couleur [VERT]
			if(body.rgb[i].green.event_low < 0) green_event_low_disable = 1;
			else green_event_low_disable = 0;

			if(body.rgb[i].green.event_high < 0) green_event_high_disable = 1;
			else green_event_high_disable = 0;

			// Detection des seuils d'alarme
			if(body.rgb[i].green.value < body.rgb[i].green.event_low) greenLowDetected = 1;
			else greenLowDetected = 0;

			if(body.rgb[i].green.value > body.rgb[i].green.event_high) greenHighDetected = 1;
			else greenHighDetected = 0;

			// Evaluation des alarmes � envoyer
			if((greenLowDetected && !green_event_low_disable) || (greenHighDetected && !green_event_high_disable)){				// Mesure tension hors plage
				if(RGB_green_WarningSended[i]==0){														// N'envoie qu'une seule fois l'EVENT
					AlgoidResponse[ptrBuff].RGBresponse.id=i;
					AlgoidResponse[ptrBuff].RGBresponse.green.value=body.rgb[i].green.value;
                                        ptrBuff++;
                                        RGBevent++;
					//sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, COLORS, 1);
					RGB_green_WarningSended[i]=1;
                                        
                                        // Si evenement pour stream activ�, envoie une trame de type status
                                        if(sysConfig.dataStream.onEvent==1)
                                            makeStatusRequest(DATAFLOW);
 //                                        printf("CHANGEMENT VERT RGB %d, VALUE:%d\n", i, body.rgb[i].green.value);
				}
			}
                        
			// Envoie un �venement Fin de niveau bas (+50mV Hysterese)
			else if (RGB_green_WarningSended[i]==1 && body.rgb[i].green.value > (body.rgb[i].green.event_low + body.rgb[i].green.event_hysteresis)){				// Mesure tension dans la plage
					AlgoidResponse[ptrBuff].RGBresponse.id=i;											// n'envoie qu'une seule fois apr�s
					AlgoidResponse[ptrBuff].RGBresponse.green.value=body.rgb[i].green.value;											// une hysterese de 50mV
                                        ptrBuff++;
                                        RGBevent++;
					//sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, COLORS, 1);
					RGB_green_WarningSended[i]=0;
                                        // Si evenement pour stream activ�, envoie une trame de type status
                                        if(sysConfig.dataStream.onEvent==1)
                                            makeStatusRequest(DATAFLOW);
 //                                       printf("-CHANGEMENT VERT RGB %d, VALUE:%d\n", i, body.rgb[i].green.value);
			}
                        
                        
                        // Contr�le l' individuelle des evenements sur changement de couleur [BLEU]
			if(body.rgb[i].blue.event_low < 0) blue_event_low_disable = 1;
			else blue_event_low_disable = 0;

			if(body.rgb[i].blue.event_high < 0) blue_event_high_disable = 1;
			else blue_event_high_disable = 0;

			// Detection des seuils d'alarme
			if(body.rgb[i].blue.value < body.rgb[i].blue.event_low) blueLowDetected = 1;
			else blueLowDetected = 0;

			if(body.rgb[i].blue.value > body.rgb[i].blue.event_high) blueHighDetected = 1;
			else blueHighDetected = 0;

			// Evaluation des alarmes � envoyer
			if((blueLowDetected && !blue_event_low_disable) || (blueHighDetected && !blue_event_high_disable)){				// Mesure tension hors plage
				if(RGB_blue_WarningSended[i]==0){														// N'envoie qu'une seule fois l'EVENT
					AlgoidResponse[ptrBuff].RGBresponse.id=i;
					AlgoidResponse[ptrBuff].RGBresponse.blue.value=body.rgb[i].blue.value;
                                        ptrBuff++;
                                        RGBevent++;
					//sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, COLORS, 1);
					RGB_blue_WarningSended[i]=1;
                                        
                                        // Si evenement pour stream activ�, envoie une trame de type status
                                        if(sysConfig.dataStream.onEvent==1)
                                            makeStatusRequest(DATAFLOW);
//                                        printf("CHANGEMENT BLEU RGB %d, VALUE:%d\n", i, body.rgb[i].blue.value);
				}
			}
                        
			// Envoie un �venement Fin de niveau bas (+50mV Hysterese)
			else if (RGB_blue_WarningSended[i]==1 && body.rgb[i].blue.value > (body.rgb[i].blue.event_low + body.rgb[i].blue.event_hysteresis)){				// Mesure tension dans la plage
					AlgoidResponse[ptrBuff].RGBresponse.id=i;											// n'envoie qu'une seule fois apr�s
					AlgoidResponse[ptrBuff].RGBresponse.blue.value=body.rgb[i].blue.value;											// une hysterese de 50mV
                                        ptrBuff++;
                                        RGBevent++;
					//sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, COLORS, 1);
					RGB_blue_WarningSended[i]=0;
                                        // Si evenement pour stream activ�, envoie une trame de type status
                                        if(sysConfig.dataStream.onEvent==1)
                                            makeStatusRequest(DATAFLOW);
 //                                       printf("-CHANGEMENT BLEU RGB %d, VALUE:%d\n", i, body.rgb[i].blue.value);
			}
		}
	}
        
        if(RGBevent>0){
		sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, COLORS, RGBevent);
                
                // Si evenement pour stream activ�, envoie une trame de type status
                if(sysConfig.dataStream.onEvent==1)
                    makeStatusRequest(DATAFLOW);
        }
 
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
// BTNSAFETYCHECK
// V�rifie si une changement d'�tat � eu lieu sur les boutons
// et effectue une action si safety actif et valeur concordante
// -------------------------------------------------------------------
void BTNSafetyCheck(void){
	// Mise � jour de l'�tat des E/S
	unsigned char ptrBuff=0, BTNsafety=0, i;
	static unsigned char safetyAction[NBBTN];

	for(i=0;i<NBBTN;i++){
		// Mise � jour de l'�tat des E/S
		body.button[i].state = getButtonInput(i);

		// V�rifie si un changement a eu lieu sur les entrees et transmet un message
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

int runUpdateCommand(int type){
    int status=0;
    int updateState=0;
    
    printf ("---------- Launching bash script ------------\n");
    
    if(type==0)
        status=system("sh /root/algobotManager.sh check");
    
    if(type==1){
        sendMqttReport(AlgoidCommand.msgID, "WARNING ! APPLICATION IS UPDATING AND WILL RESTART ");// Envoie le message sur le canal MQTT "Report"   
        status=system("sh /root/algobotManager.sh update");
    }
    
    updateState= WEXITSTATUS(status);
  
    printf ("---------- End of bash script ------------\n");
    printf ("Exit bash status: %d\n", updateState);
    
    char message[100];
    sprintf(&message[0], "Exit bash status: %d\n", updateState);	 // Formatage du message avec le Nom du client buggy
    sendMqttReport(AlgoidCommand.msgID, message);                        // Envoie le message sur le canal MQTT "Report"   

    return updateState;
}

void runRestartCommand(void){
    int status=0;
 
     printf ("---------- Launching bash script ------------\n");

        sendMqttReport(AlgoidCommand.msgID, "WARNING ! APPLICATION WILL RESTART ");// Envoie le message sur le canal MQTT "Report"   
        status=system("sh /root/algobotManager.sh restart");
    
    printf ("---------- End of bash script ------------\n");
}

void resetConfig(void){
    int i;
    
    	// Init body membre
	for(i=0;i<NBAIN;i++){
		body.battery[i].safetyStop_value=0;
		body.battery[i].event_enable=DEFAULT_EVENT_STATE;
		body.battery[i].event_high=65535;
		body.battery[i].event_low=0;
		body.battery[i].safetyStop_state=0;
		body.battery[i].safetyStop_value=0;
	}

	for(i=0;i<NBDIN;i++){
		body.proximity[i].em_stop=0;
		body.proximity[i].event_enable=DEFAULT_EVENT_STATE;
		body.proximity[i].safetyStop_state=0;
		body.proximity[i].safetyStop_value=0;
	}
        
        for(i=0;i<NBBTN;i++){
		body.button[i].em_stop=0;
		body.button[i].event_enable=DEFAULT_EVENT_STATE;
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
                sysConfig.motor[i].inverted=0;
	}
        // DEBUG Invert the motor 1 sens         
        sysConfig.motor[0].inverted=1;          // MOTOR 0 INVERTED
        sysConfig.motor[1].inverted=1;          // MOTOR 1 INVERTED
        
        for(i=0;i<NBSONAR;i++){
		body.distance[i].event_enable=DEFAULT_EVENT_STATE;
                body.distance[i].event_high=100;
                body.distance[i].event_low=15;
                body.distance[i].event_hysteresis=0;
                body.distance[i].value=-1;
	}
      
        
        for(i=0;i<NBRGBC;i++){
                body.rgb[i].event_enable=DEFAULT_EVENT_STATE;
                
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
        
        // Initialisation configuration de flux de donn�es periodique
        sysConfig.dataStream.state=ON;
        sysConfig.dataStream.time_ms=1000;
        sysConfig.config.reset=0;
        
        printf("WARNING ! Configuration reset to default value\n");
        sendMqttReport(AlgoidCommand.msgID, "WARNING ! Configuration reset to default value");// Envoie le message sur le canal MQTT "Report"  
}

int getStartupArg(int count, char *arg[]){
    unsigned char i;
    
    for(i=0;i<count;i++){
        printf("ARG #%d : %s\n", count, arg[i]);
        
        if(!strcmp(arg[i], "-n"))
            sprintf(&ClientID, "%s", arg[i+1]);
        
        if(!strcmp(arg[i], "-a"))
            sprintf(&ADDRESS, "%s", arg[i+1]);
    }
}