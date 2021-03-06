#define FIRMWARE_VERSION "1.6.2e"

#define DEFAULT_EVENT_STATE 1   

#define TASK_NUMBER 0
#define ACTION_ALGOID_ID 1
#define ACTION_COUNT 2

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "buildNumber.h"

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
#include "asyncMOTOR.h"
#include "configManager.h"
#include "type.h"

unsigned char ptrSpeedCalc;

int getStartupArg(int count, char *arg[]);

int createBuggyTask(int MsgId, int actionCount);
int removeBuggyTask(int actionNumber);

void distanceEventCheck(void);
void batteryEventCheck(void);
void DINEventCheck(void);
void BUTTONEventCheck(void);
void COLOREventCheck(void);

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
int runStepperAction(void);

int getWDvalue(int wheelName);
int getStepperValue(int motorName);

int runPwmAction(void);

//int setAsyncPwmAction(int actionNumber, int ledName, int time, int count);
//int endPwmAction(int actionNumber, int wheelNumber);

int getPwmSetting(int name);

int runLedAction(void);

int getLedSetting(int name);

int runUpdateCommand(int type);
void runRestartCommand(void);
int runCloudTestCommand(void);

void resetConfig(void);

char reportBuffer[256];

t_robotMap robot;
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
        
        sprintf(&welcomeMessage[0], "KEHOPS V%s - %s - build #%d\n", FIRMWARE_VERSION, __DATE__ , BUILD_CODE);		// Formattage du message avec le Nom du client buggy
        printf(welcomeMessage);
        printf ("------------------------------------\n");
        
// Cr�ation de la t�che pour la gestion de la messagerie avec ALGOID
	if(InitMessager()) printf ("#[CORE] Creation t�che messagerie : ERREUR\n");
	else printf ("#[CORE] Demarrage tache Messager: OK\n");

        
// Cr�ation de la t�che pour la gestion des différents timers utilisés
	if(InitTimerManager()) printf ("#[CORE] Creation tâche timer : ERREUR\n");
		else printf ("#[CORE] Demarrage tâche timer: OK\n");

// Cr�ation de la t�che pour la gestion hardware
	if(InitHwManager()) printf ("#[CORE] Creation tâche hardware : ERREUR\n");
        else {
            printf ("#[CORE] Demarrage tâche hardware: OK\n");
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
        resetConfig();
        resetHardware(&sysConfig);            // Reset les peripheriques hardware selon configuration initiale                   

        t_device device;
        
        //struct t_actuator *Pactuator, hoho;
        //Pactuator = &hoho;
        
        struct {
            int diameter;
            int speed_cm;
            struct a_motor *motorAlias;
        }wheel[2];
        
        wheel[0].motorAlias = &device.actuator.motor[0];
        wheel[1].motorAlias = &device.actuator.motor[1];
        
        
        
        device.actuator.motor[0].speed=666;
        device.actuator.motor[1].speed=999;
                
        printf("\nMa vitesse moteur device[0]: %d", device.actuator.motor[0].speed);
        printf("\nMa vitesse moteur device[1]: %d", device.actuator.motor[1].speed);
//        printf("\nMa wheel.motorAlias %d\n", wheel.motorAlias->motor.speed);
        printf("\nMa wheel.motorAlias %d\n", wheel[0].motorAlias->speed);    
        printf("\nMa wheel.motorAlias %d\n", wheel[1].motorAlias->speed);  
        
        wheel[1].motorAlias->speed = 876;
       printf("-----\nMa wheel.motorAlias %d\n", wheel[1].motorAlias->speed);  
       printf("\nMa wheel.motorAlias %d\n", device.actuator.motor[1].speed);       
       printf("\nMa vitesse moteur device[0]: %d", device.actuator.motor[0].speed);
               printf("\nMa wheel.motorAlias %d\n", wheel[0].motorAlias->speed);
        
        // Check internet connectivity
        if(runCloudTestCommand())
            printf ("#[CORE] Connexion au serveur cloud OK\n");
        else 
            printf ("#[CORE] Connexion au serveur cloud ERREUR !\n");
                 
	while(1){
        
        // Check if reset was triggered by user
        if(sysConfig.config.reset>0){
            // Reset configuration to default value
            resetConfig();
            resetHardware(&sysConfig);
            systemDataStreamCounter=0;
            runCloudTestCommand();
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
                            robot.rgb[i].red.value=getColorValue(i,RED);
                            robot.rgb[i].green.value=getColorValue(i,GREEN);
                            robot.rgb[i].blue.value=getColorValue(i,BLUE);
                            robot.rgb[i].clear.value=getColorValue(i,CLEAR);
                        }			

                        for(i=0;i<NBMOTOR;i++){
                            
                            // Convert millimeter per pulse to centimeter per pulse and calculation of distance
                            robot.motor[i].speed_cmS = (float)(getMotorFrequency(i)) * (sysConfig.wheel[i]._MMPP / 10.0);
                            robot.motor[i].speed_rpm = 60 * (float)(getMotorFrequency(i)) / (sysConfig.wheel[i].pulsePerRot);
                            robot.motor[i].distance_cm = (float)(getMotorPulses(i)) * (sysConfig.wheel[i]._MMPP / 10.0);
                           // printf("\n----- SPEED RPM#: %.2f -----\n", robot.motor[i].speed_rpm);
                            //printf("\n----- DISTANCE #%d:  %2f -----\n",i, robot.motor[i].distance_cm);
                        }
/*
                       int setpoint;
                       setpoint = PID_speedControl(robot.motor[0].speed_cmS, robot.motor[0].velocity);
                       setpoint = rescaleMotorPower(0, setpoint);
                       motorSpeedSetpoint(0, setpoint);                      
                        */                      
			DINEventCheck();										// Cont�le de l'�tat des entr�es num�rique
															// G�n�re un �venement si changement d'�tat d�tect�

                        BUTTONEventCheck();										// Cont�le de l'�tat des entr�es bouton
															// G�n�re un �venement si changement d'�tat d�tect�
                        
                        COLOREventCheck();										// Cont�le les valeur RGB des capteurs
                        
			robot.distance[0].value = getSonarDistance();
			distanceEventCheck();										// Provoque un �venement de type "distance" si la distance mesur�e
															// est hors de la plage sp�cifi�e par l'utilisateur

			robot.battery[0].value = getBatteryVoltage();
                        robot.battery[0].capacity=(robot.battery[0].value-3500)/((4210-3500)/100);
                        batteryEventCheck();

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
    int i, valCnt;
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
                                
		case STEPPER : 	
                                for(i=0;i<AlgoidCommand.msgValueCnt;i++){
                                   
                                    // Controle que le moteur existe...
                                    if(AlgoidCommand.StepperMotor[i].motor >= 0 && AlgoidCommand.StepperMotor[i].motor < NBSTEPPER)
                                        AlgoidResponse[i].STEPPERresponse.motor=AlgoidCommand.StepperMotor[i].motor;
                                    else
                                        AlgoidResponse[i].STEPPERresponse.motor=-1;
                                            
                                    // Récupération des paramètes de commandes
                                    
                                    // Retourne un message ALGOID si velocit� hors tol�rences
                                    if((AlgoidCommand.StepperMotor[i].velocity < -100) ||(AlgoidCommand.StepperMotor[i].velocity > 100)){
                                            AlgoidCommand.StepperMotor[i].velocity=0;
                                            AlgoidResponse[i].STEPPERresponse.velocity=-1;
                                    }else
                                        AlgoidResponse[i].STEPPERresponse.velocity=AlgoidCommand.StepperMotor[i].velocity;
                                    
                                    AlgoidResponse[i].STEPPERresponse.step=AlgoidCommand.StepperMotor[i].step;
                                    AlgoidResponse[i].STEPPERresponse.rotation=AlgoidCommand.StepperMotor[i].rotation;
                                    AlgoidResponse[i].responseType = RESP_STD_MESSAGE;
                                }
                                // Retourne en r�ponse le message v�rifi�
                                sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, RESPONSE, STEPPER, AlgoidCommand.msgValueCnt);  // Retourne une r�ponse d'erreur, (aucun moteur d�fini)
                                
                                runStepperAction(); break;			// Action avec en param�tre MOTEUR, VELOCITE, PAS, ROTATION
                                
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
                                for(valCnt=0;valCnt<AlgoidCommand.msgValueCnt;valCnt++){
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
                                    
                                // CONFIG COMMAND FOR MOTOR SETTING
                                    for(i=0;i<AlgoidCommand.Config.motValueCnt; i++){
                                        AlgoidResponse[valCnt].CONFIGresponse.motValueCnt=AlgoidCommand.Config.motValueCnt;
                                        // Check if motor exist...
                                        if(AlgoidCommand.Config.motor[i].id >= 0 && AlgoidCommand.Config.motor[i].id <NBMOTOR){
                                            // Save config for motor inversion
                                            if(!strcmp(AlgoidCommand.Config.motor[i].inverted, "on")){
                                                sysConfig.motor[AlgoidCommand.Config.motor[i].id].inverted=1;
                                                strcpy(AlgoidResponse[valCnt].CONFIGresponse.motor[i].inverted, "on");
                                            }
                                            else if(!strcmp(AlgoidCommand.Config.motor[i].inverted, "off")){
                                                    sysConfig.motor[AlgoidCommand.Config.motor[i].id].inverted=0;
                                                    strcpy(AlgoidResponse[valCnt].CONFIGresponse.motor[i].inverted, "off");
                                            }

                                            // Save config for motor Min PWM for run
                                            sysConfig.motor[AlgoidCommand.Config.motor[i].id].minPWM = AlgoidCommand.Config.motor[i].minPWM;
                                            
                                            // Save config for motor Min Max RPM
                                            sysConfig.motor[AlgoidCommand.Config.motor[i].id].minRPM=AlgoidCommand.Config.motor[i].minRPM;
                                            sysConfig.motor[AlgoidCommand.Config.motor[i].id].maxRPM=AlgoidCommand.Config.motor[i].maxRPM;
                                            
                                            // Save config for motor PID regulator
                                            if(!strcmp(AlgoidCommand.Config.motor[i].rpmRegulator.PIDstate, "on")){
                                                sysConfig.motor[AlgoidCommand.Config.motor[i].id].rpmRegulator.PIDstate=1;
                                                strcpy(AlgoidResponse[valCnt].CONFIGresponse.motor[i].rpmRegulator.PIDstate, "on");
                                            }
                                            else if(!strcmp(AlgoidCommand.Config.motor[i].rpmRegulator.PIDstate, "off")){
                                                    sysConfig.motor[AlgoidCommand.Config.motor[i].id].rpmRegulator.PIDstate=0;
                                                    strcpy(AlgoidResponse[valCnt].CONFIGresponse.motor[i].rpmRegulator.PIDstate, "off");
                                            }
                                            
                                            sysConfig.motor[AlgoidCommand.Config.motor[i].id].rpmRegulator.PID_Kp=AlgoidCommand.Config.motor[i].rpmRegulator.PID_Kp;
                                            sysConfig.motor[AlgoidCommand.Config.motor[i].id].rpmRegulator.PID_Ki=AlgoidCommand.Config.motor[i].rpmRegulator.PID_Ki;
                                            sysConfig.motor[AlgoidCommand.Config.motor[i].id].rpmRegulator.PID_Kd=AlgoidCommand.Config.motor[i].rpmRegulator.PID_Kd;
                                            
                                            AlgoidResponse[valCnt].CONFIGresponse.motor[i].minRPM = AlgoidCommand.Config.motor[i].minRPM;
                                            AlgoidResponse[valCnt].CONFIGresponse.motor[i].id = AlgoidCommand.Config.motor[i].id;
                                            AlgoidResponse[valCnt].CONFIGresponse.motor[i].rpmRegulator.PID_Kp = AlgoidCommand.Config.motor[i].rpmRegulator.PID_Kp;
                                            AlgoidResponse[valCnt].CONFIGresponse.motor[i].rpmRegulator.PID_Ki = AlgoidCommand.Config.motor[i].rpmRegulator.PID_Ki;
                                            AlgoidResponse[valCnt].CONFIGresponse.motor[i].rpmRegulator.PID_Kd = AlgoidCommand.Config.motor[i].rpmRegulator.PID_Kd;
                                        }
                                        else
                                            AlgoidResponse[valCnt].CONFIGresponse.motor[i].id=-1;
                                    }

                                // CONFIG COMMAND FOR WHEEL SETTING
                                    for(i=0;i<AlgoidCommand.Config.wheelValueCnt; i++){
                                        AlgoidResponse[valCnt].CONFIGresponse.wheelValueCnt=AlgoidCommand.Config.wheelValueCnt;
                                        // Check if motor exist...
                                        if(AlgoidCommand.Config.wheel[i].id >= 0 && AlgoidCommand.Config.wheel[i].id <NBMOTOR){
                                            // Save config for motor inversion
                                                sysConfig.wheel[AlgoidCommand.Config.wheel[i].id].diameter = AlgoidCommand.Config.wheel[i].diameter;
                                                sysConfig.wheel[AlgoidCommand.Config.wheel[i].id].pulsePerRot = AlgoidCommand.Config.wheel[i].pulsesPerRot;
                                                // Calculation of value for centimeter for each pulse
                                                sysConfig.wheel[AlgoidCommand.Config.wheel[i].id]._MMPP = (sysConfig.wheel[AlgoidCommand.Config.wheel[i].id].diameter * 3.1415926535897932384)/sysConfig.wheel[AlgoidCommand.Config.wheel[i].id].pulsePerRot;

                                            AlgoidResponse[valCnt].CONFIGresponse.wheel[i].id = AlgoidCommand.Config.wheel[i].id;
                                        }
                                        else
                                            AlgoidResponse[valCnt].CONFIGresponse.wheel[i].id=-1;
                                    }                                    
                                    
                                // CONFIG COMMAND FOR STEPPER SETTING
                                    for(i=0;i<AlgoidCommand.Config.stepperValueCnt; i++){
                                        AlgoidResponse[valCnt].CONFIGresponse.stepperValueCnt=AlgoidCommand.Config.stepperValueCnt;
                                        // Check if motor exist...
                                        if(AlgoidCommand.Config.stepper[i].id >= 0 && AlgoidCommand.Config.stepper[i].id < NBSTEPPER){
                                            // Save config for motor inversion
                                            if(!strcmp(AlgoidCommand.Config.stepper[i].inverted, "on")){
                                                sysConfig.stepper[AlgoidCommand.Config.stepper[i].id].inverted=1;
                                                strcpy(AlgoidResponse[valCnt].CONFIGresponse.stepper[i].inverted, "on");
                                            }
                                            else if(!strcmp(AlgoidCommand.Config.stepper[i].inverted, "off")){
                                                    sysConfig.stepper[AlgoidCommand.Config.stepper[i].id].inverted=0;
                                                    strcpy(AlgoidResponse[valCnt].CONFIGresponse.stepper[i].inverted, "off");
                                            }
                                            
                                             sysConfig.stepper[AlgoidCommand.Config.stepper[i].id].ratio=AlgoidCommand.Config.stepper[i].ratio;
                                             sysConfig.stepper[AlgoidCommand.Config.stepper[i].id].stepPerRot=AlgoidCommand.Config.stepper[i].stepsPerRot;

                                            AlgoidResponse[valCnt].CONFIGresponse.stepper[i].id = AlgoidCommand.Config.stepper[i].id;
                                        }
                                        else
                                            AlgoidResponse[valCnt].CONFIGresponse.stepper[i].id=-1;
                                    }                                    

                                // CONFIG COMMAND FOR LED SETTING
                                    for(i=0;i<AlgoidCommand.Config.ledValueCnt; i++){
                                        AlgoidResponse[valCnt].CONFIGresponse.ledValueCnt=AlgoidCommand.Config.ledValueCnt;
                                        
                                        // Check if led exist...
                                        if(AlgoidCommand.Config.led[i].id >= 0 && AlgoidCommand.Config.led[i].id <NBLED){
                                            sysConfig.led[AlgoidCommand.Config.led[i].id].power=AlgoidCommand.Config.led[i].power;
                                            AlgoidResponse[valCnt].CONFIGresponse.led[i].power=AlgoidCommand.Config.led[i].power;
                                            // Save config for led inversion
                                            if(!strcmp(AlgoidCommand.Config.led[i].state, "on")){
                                                sysConfig.led[AlgoidCommand.Config.led[i].id].state=1;
                                                strcpy(AlgoidResponse[valCnt].CONFIGresponse.led[i].state, "on");
                                            }
                                            else if(!strcmp(AlgoidCommand.Config.led[i].state, "off")){
                                                    sysConfig.led[AlgoidCommand.Config.led[i].id].state=0;
                                                    strcpy(AlgoidResponse[valCnt].CONFIGresponse.led[i].state, "off");
                                            }

                                            AlgoidResponse[valCnt].CONFIGresponse.led[i].id = AlgoidCommand.Config.led[i].id;
                                        }
                                        else
                                            AlgoidResponse[valCnt].CONFIGresponse.led[i].id=-1;
                                    }

                                // CONFIG COMMAND FOR SAVE
                                    if(!strcmp(AlgoidCommand.Config.config.save, "true"))
                                        SaveConfig(&sysConfig, "kehops.cfg");

                                // CONFIG COMMAND FOR RESET
                                    if(!strcmp(AlgoidCommand.Config.config.reset, "true"))
                                       sysConfig.config.reset=1;

                        // Préparation des valeurs du message de réponse
                                // GET STREAM CONFIG FOR RESPONSE
                                    AlgoidResponse[valCnt].CONFIGresponse.stream.time=sysConfig.dataStream.time_ms;
                                    
                                    if(sysConfig.dataStream.onEvent==0) 
                                        strcpy(AlgoidResponse[valCnt].CONFIGresponse.stream.onEvent, "off");
                                    else strcpy(AlgoidResponse[valCnt].CONFIGresponse.stream.onEvent, "on");

                                    if(sysConfig.dataStream.state==0) 
                                        strcpy(AlgoidResponse[valCnt].CONFIGresponse.stream.state, "off");
                                    else strcpy(AlgoidResponse[valCnt].CONFIGresponse.stream.state, "on");

                                    if(sysConfig.config.reset==1) 
                                        strcpy(AlgoidResponse[valCnt].CONFIGresponse.config.reset, "true");
                                    else strcpy(AlgoidResponse[valCnt].CONFIGresponse.config.reset, "---");
                                    
                                    if(!strcmp(AlgoidCommand.Config.config.save, "true")) 
                                        strcpy(AlgoidResponse[valCnt].CONFIGresponse.config.save, "true");
                                    else strcpy(AlgoidResponse[valCnt].CONFIGresponse.config.save, "---");
                                    
                                    AlgoidResponse[valCnt].responseType = RESP_STD_MESSAGE;
                                } 
                                
                                //AlgoidResponse[0].responseType = RESP_STD_MESSAGE;                     
                                // Retourne en r�ponse le message v�rifi�
                                sendResponse(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom, RESPONSE, CONFIG, AlgoidCommand.msgValueCnt);

                                break;
                                
            case SYSTEM :       
                                // RECHERCHE DES MISE A JOURS
                                if(!strcmp(AlgoidCommand.System.application, "check")){
                                    AlgoidResponse[0].responseType=EVENT_ACTION_BEGIN;                                            
                                    updateResult = runUpdateCommand(0);
                                    
                                    char message[100];
                                    
                                    switch(updateResult){
                                        case 1 :   strcpy(AlgoidResponse[0].SYSCMDresponse.application, "connection error"); break;
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
                        robot.motor[i].velocity=AlgoidCommand.DCmotor[ptrData].velocity;
                        robot.motor[i].accel=AlgoidCommand.DCmotor[ptrData].accel;
                        robot.motor[i].decel=AlgoidCommand.DCmotor[ptrData].decel;
                        robot.motor[i].cm=AlgoidCommand.DCmotor[ptrData].cm;
                        robot.motor[i].time=AlgoidCommand.DCmotor[ptrData].time;
            }
        }

        // Au moin une action � effectuer
        if(actionCount>0){
            
            // Retoure un message EVENT de type BEGIN 
            AlgoidResponse[0].responseType = EVENT_ACTION_BEGIN;
            // Retourne un message event ALGOID 
            sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom,  EVENT, MOTORS, 1);
                    
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
                            if(robot.motor[ID].accel!=0 || robot.motor[ID].decel)
                                setMotorAccelDecel(ID, robot.motor[ID].accel, robot.motor[ID].decel);
                            
                            // Effectue l'action sur la roue
                            if(robot.motor[ID].cm <=0 && robot.motor[ID].time<=0){                                
                                sprintf(reportBuffer, "ATTENTION: Action infinie, aucun parametre defini \"time\" ou \"cm\" pour l'action sur le moteur %d\n", ID);

                                printf(reportBuffer);                                                             // Affichage du message dans le shell
                                sendMqttReport(AlgoidCommand.msgID, reportBuffer);				      // Envoie le message sur le canal MQTT "Report"     
                                setAsyncMotorAction(myTaskId, ID, robot.motor[ID].velocity, INFINITE, NULL);

                                // Défini l'état de laction comme "en cours" pour message de réponse
                                AlgoidResponse[0].responseType = EVENT_ACTION_RUN;
                                // Retourne un message event ALGOID 
                                sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom,  EVENT, MOTORS, 1);
                            }else
                            {
                                if(robot.motor[ID].cm > 0)
                                        setAsyncMotorAction(myTaskId, ID, robot.motor[ID].velocity, CENTIMETER, robot.motor[ID].cm);
                                else{
                                        setAsyncMotorAction(myTaskId, ID, robot.motor[ID].velocity, MILLISECOND, robot.motor[ID].time);
                                }
                            }
                        }
                        action++;
                    }
                    return 0;
            }
            else
                return 1;
        }
        // Aucun param�tre trouv� ou moteur inexistant
        else{
            
            AlgoidResponse[0].responseType = EVENT_ACTION_ERROR;
            sendResponse(myTaskId, AlgoidMessageRX.msgFrom, EVENT, MOTORS, 1);               // Envoie un message EVENT error
            sprintf(reportBuffer, "ERREUR: Aucun moteur defini ou inexistant pour le message #%d\n", AlgoidCommand.msgID);
            printf(reportBuffer);                                                             // Affichage du message dans le shell
            sendMqttReport(AlgoidCommand.msgID, reportBuffer);				      // Envoie le message sur le canal MQTT "Report"
        }
}

// -------------------------------------------------------------------
// runStepperAction
// Effectue une action avec les paramètre recus: MOTEUR, VELOCITE, PAS, ROTATION
// -------------------------------------------------------------------
int runStepperAction(void){
	int ptrData;
	int myTaskId;
	unsigned char actionCount=0;
	int action=0;
        int i;
        int ID;

	// Comptabilise le nombre de paramètre (moteur pas à pas) recu dans le message
	// 
        for(i=0;i<NBSTEPPER;i++){
            ptrData=getStepperValue(i);
            if(ptrData>=0){
                actionCount++;
                        robot.stepper[i].speed=AlgoidCommand.StepperMotor[ptrData].velocity;
                        robot.stepper[i].time=AlgoidCommand.StepperMotor[ptrData].time;
                        robot.stepper[i].step=AlgoidCommand.StepperMotor[ptrData].step;
                        robot.stepper[i].angle=AlgoidCommand.StepperMotor[ptrData].angle;
                        robot.stepper[i].rotation=AlgoidCommand.StepperMotor[ptrData].rotation;
            }
        }

        // Au moin une action à effectuer
        if(actionCount>0){
            
            // Retoure un message EVENT de type BEGIN 
            AlgoidResponse[0].responseType = EVENT_ACTION_BEGIN;
            // Retourne un message event ALGOID 
            sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom,  EVENT, STEPPER, 1);
                    
            // Ouverture d'une tâche pour les toutes les actions du message algoid � effectuer
            // Recois un numéro de tache en retour
            myTaskId=createBuggyTask(AlgoidCommand.msgID, actionCount);			// 2 actions pour mouvement 2WD

            // Démarrage des actions
            if(myTaskId>0){
                    printf("Creation de tache STEPPER MOTOR: #%d avec %d actions\n", myTaskId, actionCount);

                    // Sauvegarde du nom de l'emetteur et du ID du message pour la r�ponse
                    // en fin d'�venement
                    saveSenderOfMsgId(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom);

                    for(ptrData=0; action < actionCount && ptrData<10; ptrData++){
                        ID = AlgoidCommand.StepperMotor[ptrData].motor;
                        if(ID >= 0){
                            
                            // Effectue l'action sur le moteur pas à pas
                            if(robot.stepper[ID].time<=0 && robot.stepper[ID].step <=0 && robot.stepper[ID].rotation<=0 && robot.stepper[ID].angle<=0){                                
                                sprintf(reportBuffer, "ATTENTION: Action infinie, aucun parametre defini \"time\" ou \"step\" ou \"rotation\" ou \"angle\"pour l'action sur le moteur pas à pas %d\n", ID);

                                printf(reportBuffer);                                                             // Affichage du message dans le shell
                                sendMqttReport(AlgoidCommand.msgID, reportBuffer);				      // Envoie le message sur le canal MQTT "Report"     
                                setAsyncStepperAction(myTaskId, ID, robot.stepper[ID].speed, INFINITE, NULL);

                                // Défini l'état de laction comme "en cours" pour message de réponse
                                AlgoidResponse[0].responseType = EVENT_ACTION_RUN;
                                // Retourne un message event ALGOID 
                                sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom,  EVENT, STEPPER, 1);
                            }else
                            {
                                if(robot.stepper[ID].step > 0){
                                    setAsyncStepperAction(myTaskId, ID, robot.stepper[ID].speed, STEP, robot.stepper[ID].step);
                                }
                                else{
                                    if(robot.stepper[ID].angle > 0){
                                       setAsyncStepperAction(myTaskId, ID, robot.stepper[ID].speed, ANGLE, robot.stepper[ID].angle);
                                    }else
                                    {
                                        if(robot.stepper[ID].rotation > 0){
                                            setAsyncStepperAction(myTaskId, ID, robot.stepper[ID].speed, ROTATION, robot.stepper[ID].rotation);
                                        }else{
                                            if(robot.stepper[ID].time > 0){
                                                setAsyncStepperAction(myTaskId, ID, robot.stepper[ID].speed, MILLISECOND, robot.stepper[ID].time);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        action++;
                    }
                    return 0;
            }
            else
                return 1;
        }
        // Aucun param�tre trouv� ou moteur inexistant
        else{
            
            AlgoidResponse[0].responseType = EVENT_ACTION_ERROR;
            sendResponse(myTaskId, AlgoidMessageRX.msgFrom, EVENT, MOTORS, 1);               // Envoie un message EVENT error
            sprintf(reportBuffer, "ERREUR: Aucun moteur défini ou inexistant pour le message #%d\n", AlgoidCommand.msgID);
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
                    robot.led[i].state=OFF;
                if(!strcmp(AlgoidCommand.LEDarray[ptrData].state,"on"))
                    robot.led[i].state=ON;
                if(!strcmp(AlgoidCommand.LEDarray[ptrData].state,"blink"))
                    robot.led[i].state=BLINK;
                
                // R�cup�ration des consignes dans le message (si disponible)
                if(AlgoidCommand.LEDarray[ptrData].powerPercent > 0)
                    robot.led[i].power=AlgoidCommand.LEDarray[ptrData].powerPercent;
                
                if(AlgoidCommand.LEDarray[ptrData].time > 0)
                    robot.led[i].blinkTime=AlgoidCommand.LEDarray[ptrData].time;
                
                if(AlgoidCommand.LEDarray[ptrData].blinkCount > 0)
                    robot.led[i].blinkCount=AlgoidCommand.LEDarray[ptrData].blinkCount;
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

                    AlgoidResponse[0].responseType=EVENT_ACTION_BEGIN;
                    sendResponse(myTaskId, AlgoidMessageRX.msgFrom, EVENT, pLED, 1);                         // Envoie un message ALGOID de fin de t�che pour l'action �cras�
                    
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
                                    if(robot.led[ID].state==BLINK){
                                        
                                        // Verifie la presence de parametres de type "time" et "count", sinon applique des
                                        // valeurs par defaut
/*                                        if(time<=0){
 */ 
                                        if(time<=0 && Count<=0){
                                            time=500;
                                            Count=1;
                                            sprintf(reportBuffer, "ATTENTION: Action infinie, aucun parametre defini \"time\" ou \"count\" pour l'action sur la LED %d\n", ID);
                                            printf(reportBuffer);                                                             // Affichage du message dans le shell
                                            sendMqttReport(AlgoidCommand.msgID, reportBuffer);	
                                            
                                            AlgoidResponse[0].responseType=EVENT_ACTION_RUN;
                                            sendResponse(myTaskId, AlgoidMessageRX.msgFrom, EVENT, pLED, 1);                         // Envoie un message ALGOID de fin de t�che pour l'action �cras�
                                        }
            /*   BUG SEGFAULT
                                        if(Count<=0){
                                            Count=1;
                                            sprintf(reportBuffer, "ATTENTION: Action infinie, aucun parametre defini \"count\"  pour l'action sur la LED %d\n", ID);
                                            printf(reportBuffer);                                                             // Affichage du message dans le shell
                                            sendMqttReport(AlgoidCommand.msgID, reportBuffer);				      // Envoie le message sur le canal MQTT "Report"     
                                        }
            */
                                        // Creation d'un timer effectu� sans erreur, ni ecrasement d'une ancienne action
                                         setAsyncLedAction(myTaskId, ID, BLINK, time, Count);
                                     }

                                    // Mode on ou off
                                    else{
                                            if(robot.led[ID].state==OFF)
                                                setAsyncLedAction(myTaskId, ID, OFF, NULL, NULL);

                                            if(robot.led[ID].state==ON)
                                                setAsyncLedAction(myTaskId, ID, ON, NULL, NULL);
                                    }

                                    action++;
                            }
                    }
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
                    robot.pwm[i].state=OFF;
                if(!strcmp(AlgoidCommand.PWMarray[ptrData].state,"on"))
                    robot.pwm[i].state=ON;
                

                // Blink mode not available in SERVO MODE
                if(!AlgoidCommand.PWMarray[ptrData].isServoMode){
                    if(!strcmp(AlgoidCommand.PWMarray[ptrData].state,"blink"))
                        robot.pwm[i].state=BLINK;
                    if(AlgoidCommand.PWMarray[ptrData].time > 0)
                        robot.pwm[i].blinkTime=AlgoidCommand.PWMarray[ptrData].time;
                    if(AlgoidCommand.PWMarray[ptrData].blinkCount > 0)
                        robot.pwm[i].blinkCount=AlgoidCommand.PWMarray[ptrData].blinkCount;
                }
                else{
                    
                }
                
                // Recuperation des consignes dans le message (si disponible)
                if(AlgoidCommand.PWMarray[ptrData].powerPercent >= 0)
                    robot.pwm[i].power=AlgoidCommand.PWMarray[ptrData].powerPercent;
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
                                        if(robot.pwm[ID].state==BLINK){
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
                                            if(robot.pwm[ID].state==OFF)
                                                setAsyncPwmAction(myTaskId, ID, OFF, NULL, NULL);

                                            if(robot.pwm[ID].state==ON)
                                                setAsyncPwmAction(myTaskId, ID, ON, NULL, NULL);
                                            }
                                    }
                                    else
                                        
                                    {
                                            if(robot.pwm[ID].state==OFF)
                                                setAsyncServoAction(myTaskId, ID, OFF, NULL);

                                            if(robot.pwm[ID].state==ON)
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
// GETSTEPPERVALUE
// Recherche dans le message les param�tres
// Retourne un pointeur sur le champs de param�tre correspondant au moteur sp�cifi�
// -------------------------------------------------------------------
int getStepperValue(int motorName){
	int i;
	int searchPtr = -1;

	// V�rifie que le moteur est existant...
		// Recherche dans les donn�e recues la valeur correspondante au param�tre "wheelName"
		for(i=0;i<AlgoidCommand.msgValueCnt;i++){
			if(motorName == AlgoidCommand.StepperMotor[i].motor)
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

	sprintf(reportBuffer, "ERREUR: Table de tâches pleine\n");
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
        AlgoidResponse[ptrData].SYSresponse.battVoltage=robot.battery[0].value;
        AlgoidResponse[ptrData].SYSresponse.battPercent=robot.battery[0].capacity;
        ptrData++;
        
	for(i=0;i<NBDIN;i++){
		AlgoidResponse[ptrData].DINresponse.id=i;
		AlgoidResponse[ptrData].value=robot.proximity[i].state;
                
                if(robot.proximity[i].event_enable) strcpy(AlgoidResponse[ptrData].DINresponse.event_state, "on");
                else strcpy(AlgoidResponse[ptrData].DINresponse.event_state, "off");                
		ptrData++;
	}
                
        for(i=0;i<NBBTN;i++){
                AlgoidResponse[ptrData].BTNresponse.id=i;
                AlgoidResponse[ptrData].value=robot.button[i].state;
                
                if(robot.button[i].event_enable) strcpy(AlgoidResponse[ptrData].BTNresponse.event_state, "on");
                else strcpy(AlgoidResponse[ptrData].BTNresponse.event_state, "off");
                ptrData++;
	}

	for(i=0;i<NBMOTOR;i++){
		AlgoidResponse[ptrData].MOTresponse.motor=i;
		AlgoidResponse[ptrData].MOTresponse.speed=robot.motor[i].speed_rpm;
		AlgoidResponse[ptrData].MOTresponse.cm = rpmToPercent(i, robot.motor[i].speed_rpm);
                AlgoidResponse[ptrData].MOTresponse.velocity = rpmToPercent(0,sysConfig.motor[0].minRPM) + robot.motor[i].velocity;;
		ptrData++;
	}

        for(i=0;i<NBSONAR;i++){
                AlgoidResponse[ptrData].DISTresponse.id=i;
                AlgoidResponse[ptrData].value=robot.distance[i].value;
                
                if(robot.distance[i].event_enable) strcpy(AlgoidResponse[ptrData].DISTresponse.event_state, "on");
                else strcpy(AlgoidResponse[ptrData].DISTresponse.event_state, "off");
                ptrData++;
	}

        for(i=0;i<NBRGBC;i++){
		AlgoidResponse[ptrData].RGBresponse.id=i;
		AlgoidResponse[ptrData].RGBresponse.red.value=robot.rgb[i].red.value;
                AlgoidResponse[ptrData].RGBresponse.green.value=robot.rgb[i].green.value;
                AlgoidResponse[ptrData].RGBresponse.blue.value=robot.rgb[i].blue.value;
                AlgoidResponse[ptrData].RGBresponse.clear.value=robot.rgb[i].clear.value;
                
                if(robot.rgb[i].event_enable) strcpy(AlgoidResponse[ptrData].RGBresponse.event_state, "on");
                else strcpy(AlgoidResponse[ptrData].RGBresponse.event_state, "off");
                
		ptrData++;
	}

        for(i=0;i<NBLED;i++){
		AlgoidResponse[ptrData].LEDresponse.id=i;
		AlgoidResponse[ptrData].value=robot.led[i].state;
                AlgoidResponse[ptrData].LEDresponse.powerPercent=robot.led[i].power;
		ptrData++;
	}

        for(i=0;i<NBPWM;i++){
		AlgoidResponse[ptrData].PWMresponse.id=i;
		AlgoidResponse[ptrData].value=robot.pwm[i].state;
                AlgoidResponse[ptrData].PWMresponse.powerPercent=robot.pwm[i].power;
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
				if(!strcmp(AlgoidCommand.DINsens[i].event_state, "on"))	robot.proximity[AlgoidCommand.DINsens[i].id].event_enable=1; 			// Activation de l'envoie de messages sur �venements
				else if(!strcmp(AlgoidCommand.DINsens[i].event_state, "off"))	robot.proximity[AlgoidCommand.DINsens[i].id].event_enable=0;    // D�sactivation de l'envoie de messages sur �venements
			} else
				AlgoidResponse[i].value = -1;
		};

	// RETOURNE EN REPONSE LES PARAMETRES ENREGISTRES ---
	for(i=0;i<AlgoidCommand.msgValueCnt;i++){
		int temp = AlgoidResponse[i].DINresponse.id;

		// Contr�le que le capteur soit pris en charge
		if(AlgoidCommand.DINsens[i].id < NBDIN){
			AlgoidResponse[i].value = robot.proximity[temp].state;
			if(robot.proximity[temp].event_enable) strcpy(AlgoidResponse[i].DINresponse.event_state, "on");
				else strcpy(AlgoidResponse[i].DINresponse.event_state, "off");
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
				if(!strcmp(AlgoidCommand.BTNsens[i].event_state, "on"))	robot.button[AlgoidCommand.BTNsens[i].id].event_enable=1; 			// Activation de l'envoie de messages sur �venements
				else if(!strcmp(AlgoidCommand.BTNsens[i].event_state, "off"))	robot.button[AlgoidCommand.BTNsens[i].id].event_enable=0;    // D�sactivation de l'envoie de messages sur �venements
			} else
				AlgoidResponse[i].value = -1;
		};

	// RETOURNE EN REPONSE LES PARAMETRES ENREGISTRES ---
	for(i=0;i<AlgoidCommand.msgValueCnt;i++){
		int temp = AlgoidResponse[i].BTNresponse.id;

		// Contr�le que le capteur soit pris en charge
		if(AlgoidCommand.BTNsens[i].id < NBBTN){
			AlgoidResponse[i].value = robot.button[temp].state;
			if(robot.button[temp].event_enable) strcpy(AlgoidResponse[i].BTNresponse.event_state, "on");
                        else strcpy(AlgoidResponse[i].BTNresponse.event_state, "off");
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
							robot.distance[AlgoidCommand.DISTsens[i].id].event_enable=1;
							saveSenderOfMsgId(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom);
					}
					else if(!strcmp(AlgoidCommand.DISTsens[i].event_state, "off")){
						robot.distance[AlgoidCommand.DISTsens[i].id].event_enable=0;
						removeSenderOfMsgId(AlgoidCommand.msgID);
					}

					// Evemenent haut
					if(AlgoidCommand.DISTsens[i].event_high!=0)
						robot.distance[AlgoidCommand.DISTsens[i].id].event_high=AlgoidCommand.DISTsens[i].event_high;
					// Evemenent bas
					if(AlgoidCommand.DISTsens[i].event_low!=0)
                                            robot.distance[AlgoidCommand.DISTsens[i].id].event_low=AlgoidCommand.DISTsens[i].event_low;
				} else
					AlgoidResponse[i].value = -1;
			};

	for(i=0;i<AlgoidCommand.msgValueCnt; i++){
		// RETOURNE EN REPONSE LES PARAMETRES ENREGISTRES
		// R�cup�ration des param�tres actuels et chargement du buffer de r�ponse
		int temp = AlgoidResponse[i].DISTresponse.id;

		if(AlgoidCommand.DISTsens[i].id <NBSONAR){
			AlgoidResponse[i].value=robot.distance[temp].value;
			//AlgoidResponse[i].DISTresponse.angle=angle[AlgoidCommand.DISTsens[i].angle];

			if(robot.distance[temp].event_enable)strcpy(AlgoidResponse[i].DISTresponse.event_state, "on");
			else strcpy(AlgoidResponse[i].DISTresponse.event_state, "off");
			AlgoidResponse[i].DISTresponse.event_high=robot.distance[temp].event_high;
			AlgoidResponse[i].DISTresponse.event_low=robot.distance[temp].event_low;
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
							robot.rgb[AlgoidCommand.RGBsens[i].id].event_enable=1;
							saveSenderOfMsgId(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom);
					}
					else if(!strcmp(AlgoidCommand.RGBsens[i].event_state, "off")){
						robot.rgb[AlgoidCommand.RGBsens[i].id].event_enable=0;
						removeSenderOfMsgId(AlgoidCommand.msgID);
					}

                                        // Param�tre capteur ROUGE
					// Evemenent haut
					if(AlgoidCommand.RGBsens[i].red.event_high!=0)
						robot.rgb[AlgoidCommand.RGBsens[i].id].red.event_high=AlgoidCommand.RGBsens[i].red.event_high;
					// Evemenent bas
					if(AlgoidCommand.RGBsens[i].red.event_low!=0)
						robot.rgb[AlgoidCommand.RGBsens[i].id].red.event_low=AlgoidCommand.RGBsens[i].red.event_low;
                                        
                                        // Param�tre capteur VERT
                                        // Evemenent haut
					if(AlgoidCommand.RGBsens[i].green.event_high!=0)
                                            robot.rgb[AlgoidCommand.RGBsens[i].id].green.event_high=AlgoidCommand.RGBsens[i].green.event_high;
					// Evemenent bas
					if(AlgoidCommand.RGBsens[i].green.event_low!=0)
						robot.rgb[AlgoidCommand.RGBsens[i].id].green.event_low=AlgoidCommand.RGBsens[i].green.event_low;
                                        
                                        // Param�tre capteur BLEU
                                        // Evemenent haut
					if(AlgoidCommand.RGBsens[i].blue.event_high!=0)
						robot.rgb[AlgoidCommand.RGBsens[i].id].blue.event_high=AlgoidCommand.RGBsens[i].blue.event_high;
					// Evemenent bas
					if(AlgoidCommand.RGBsens[i].blue.event_low!=0)
                                                robot.rgb[AlgoidCommand.RGBsens[i].id].blue.event_low=AlgoidCommand.RGBsens[i].blue.event_low;

                                        // Param�tre capteur CLEAR
                                        // Evemenent haut
					if(AlgoidCommand.RGBsens[i].clear.event_high!=0)
						robot.rgb[AlgoidCommand.RGBsens[i].id].clear.event_high=AlgoidCommand.RGBsens[i].clear.event_high;
					// Evemenent bas
					if(AlgoidCommand.RGBsens[i].clear.event_low!=0)
						robot.rgb[AlgoidCommand.RGBsens[i].id].clear.event_low=AlgoidCommand.RGBsens[i].clear.event_low;
				} else
					AlgoidResponse[i].value = -1;
			};

	for(i=0;i<AlgoidCommand.msgValueCnt; i++){
		// RETOURNE EN REPONSE LES PARAMETRES ENREGISTRES
		// R�cup�ration des param�tres actuels et chargement du buffer de r�ponse
		int temp = AlgoidResponse[i].RGBresponse.id;

		if(AlgoidCommand.RGBsens[i].id <NBRGBC){
			AlgoidResponse[i].RGBresponse.red.value=robot.rgb[temp].red.value;
                        AlgoidResponse[i].RGBresponse.green.value=robot.rgb[temp].green.value;
                        AlgoidResponse[i].RGBresponse.blue.value=robot.rgb[temp].blue.value;
                        AlgoidResponse[i].RGBresponse.clear.value=robot.rgb[temp].clear.value;

                        // Copie de l'etat de l'evenement
			if(robot.rgb[temp].event_enable)strcpy(AlgoidResponse[i].RGBresponse.event_state, "on");
			else strcpy(AlgoidResponse[i].RGBresponse.event_state, "off");
                        
                        // Copie des param�tres �venements haut/bas pour le ROUGE
			AlgoidResponse[i].RGBresponse.red.event_high=robot.rgb[temp].red.event_high;
			AlgoidResponse[i].RGBresponse.red.event_low=robot.rgb[temp].red.event_low;

                        // Copie des param�tres �venements haut/bas pour le VERT
			AlgoidResponse[i].RGBresponse.green.event_high=robot.rgb[temp].green.event_high;
			AlgoidResponse[i].RGBresponse.green.event_low=robot.rgb[temp].green.event_low;
                        
                        // Copie des param�tres �venements haut/bas pour le BLEU
			AlgoidResponse[i].RGBresponse.blue.event_high=robot.rgb[temp].blue.event_high;
			AlgoidResponse[i].RGBresponse.blue.event_low=robot.rgb[temp].blue.event_low;
                        
                        // Copie des param�tres �venements haut/bas pour le CLEAR
			AlgoidResponse[i].RGBresponse.clear.event_high=robot.rgb[temp].clear.event_high;
			AlgoidResponse[i].RGBresponse.clear.event_low=robot.rgb[temp].clear.event_low;
                        
                        
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
						robot.battery[AlgoidCommand.BATTsens[i].id].event_enable=1;
						saveSenderOfMsgId(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom);
					}
					else if(!strcmp(AlgoidCommand.BATTsens[i].event_state, "off")){
						robot.battery[AlgoidCommand.BATTsens[i].id].event_enable=0;
						removeSenderOfMsgId(AlgoidCommand.msgID);
					}
					// Evemenent haut
					if(AlgoidCommand.BATTsens[i].event_high!=0) robot.battery[AlgoidCommand.BATTsens[i].id].event_high=AlgoidCommand.BATTsens[i].event_high;
					if(AlgoidCommand.BATTsens[i].event_high!=0) robot.battery[AlgoidCommand.BATTsens[i].id].event_low=AlgoidCommand.BATTsens[i].event_low;
				}else
					AlgoidResponse[i].value = -1;
			};

	for(i=0;i<AlgoidCommand.msgValueCnt; i++){
		// RETOURNE EN REPONSE LES PARAMETRES ENREGISTRES
		int temp = AlgoidResponse[i].BATTesponse.id;

		if(AlgoidCommand.BATTsens[i].id <NBAIN){
			AlgoidResponse[i].value=robot.battery[temp].value;

			if(robot.battery[temp].event_enable){
				strcpy(AlgoidResponse[i].BATTesponse.event_state, "on");
				saveSenderOfMsgId(AlgoidCommand.msgID, AlgoidMessageRX.msgFrom);
			}
			else{
				strcpy(AlgoidResponse[i].BATTesponse.event_state, "off");
				removeSenderOfMsgId(AlgoidCommand.msgID);
			}
			AlgoidResponse[i].BATTesponse.event_high=robot.battery[temp].event_high;
			AlgoidResponse[i].BATTesponse.event_low=robot.battery[temp].event_low;
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
                    
			AlgoidResponse[i].MOTresponse.velocity = robot.motor[temp].cm;
                        AlgoidResponse[i].MOTresponse.velocity = robot.motor[temp].velocity;
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
		if(robot.distance[i].event_enable){

			int event_low_disable, event_high_disable, distLowDetected, distHighDetected;

			// Contr�le l' individuelle des evenements ( = si valeur < 0)
			if(robot.distance[i].event_low < 0) event_low_disable = 1;
			else event_low_disable = 0;

			if(robot.distance[i].event_high < 0) event_high_disable = 1;
			else event_high_disable = 0;

			// Detection des seuils d'alarme
			if(robot.distance[i].value < robot.distance[i].event_low) distLowDetected = 1;
			else distLowDetected = 0;

			if(robot.distance[i].value > robot.distance[i].event_high) distHighDetected = 1;
			else distHighDetected = 0;

			// Evaluation des alarmes � envoyer
			if((distLowDetected && !event_low_disable) || (distHighDetected && !event_high_disable)){		// Mesure de distance hors plage
				if(distWarningSended[i]==0){													// N'envoie l' event qu'une seule fois
					AlgoidResponse[i].DISTresponse.id=i;
					AlgoidResponse[i].value=robot.distance[i].value;

                                        if(robot.distance[i].event_enable) strcpy(AlgoidResponse[i].DISTresponse.event_state, "on");
                                        else strcpy(AlgoidResponse[i].DISTresponse.event_state, "off");
                                        
					sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, DISTANCE, NBSONAR);
					distWarningSended[i]=1;
                                        
                                        // Si evenement pour stream activ�, envoie une trame de type status
                                        if(sysConfig.dataStream.onEvent==1)
                                            makeStatusRequest(DATAFLOW);
//                                        printf("CHANGEMENT SONAR%d, VALUE:%d\n", i, robot.distance[i].value);
				}
			}
			else if (distWarningSended[i]==1){													// Mesure de distance revenu dans la plage
					AlgoidResponse[i].DISTresponse.id=i;							// Et n'envoie qu'une seule fois le message
					AlgoidResponse[i].value=robot.distance[i].value;

                                        if(robot.distance[i].event_enable) strcpy(AlgoidResponse[i].DISTresponse.event_state, "on");
                                        else strcpy(AlgoidResponse[i].DISTresponse.event_state, "off");
                                        
					sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, DISTANCE, NBSONAR);
					distWarningSended[i]=0;
                                        
                                        // Si evenement pour stream activ�, envoie une trame de type status
                                        if(sysConfig.dataStream.onEvent==1)
                                            makeStatusRequest(DATAFLOW); 
//                                        printf("CHANGEMENT SONAR%d, VALUE:%d\n", i, robot.distance[i].value);
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
		if(robot.battery[i].event_enable){

			int event_low_disable, event_high_disable, battLowDetected, battHighDetected;

			// Contr�le l' individuelle des evenements ( = si valeur < 0)
			if(robot.battery[i].event_low < 0) event_low_disable = 1;
			else event_low_disable = 0;

			if(robot.battery[i].event_high < 0) event_high_disable = 1;
			else event_high_disable = 0;

			// Detection des seuils d'alarme
			if(robot.battery[i].value < robot.battery[i].event_low) battLowDetected = 1;
			else battLowDetected = 0;

			if(robot.battery[i].value > robot.battery[i].event_high) battHighDetected = 1;
			else battHighDetected = 0;

			// Evaluation des alarmes � envoyer
			if((battLowDetected && !event_low_disable) || (battHighDetected && !event_high_disable)){				// Mesure tension hors plage
				if(battWarningSended[i]==0){														// N'envoie qu'une seule fois l'EVENT
					AlgoidResponse[i].BATTesponse.id=i;
					AlgoidResponse[i].value=robot.battery[i].value;
					sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, BATTERY, 1);
					battWarningSended[i]=1;
                                        
                                        // Si evenement pour stream activ�, envoie une trame de type status
                                        if(sysConfig.dataStream.onEvent==1)
                                            makeStatusRequest(DATAFLOW);                                        
				}
			}
			// Envoie un �venement Fin de niveau bas (+50mV Hysterese)
			else if (battWarningSended[i]==1 && robot.battery[i].value > (robot.battery[i].event_low + robot.battery[i].event_hysteresis)){				// Mesure tension dans la plage
					AlgoidResponse[i].BATTesponse.id=i;											// n'envoie qu'une seule fois apr�s
					AlgoidResponse[i].value=robot.battery[i].value;
                                        // une hysterese de 50mV
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
		oldDinValue[i]=robot.proximity[i].state;
		robot.proximity[i].state = getDigitalInput(i);

		// V�rifie si un changement a eu lieu sur les entrees et transmet un message
		// "event" listant les modifications
		if(robot.proximity[i].event_enable && (oldDinValue[i] != robot.proximity[i].state)){
			AlgoidResponse[ptrBuff].DINresponse.id=i;
			AlgoidResponse[ptrBuff].value=robot.proximity[i].state;

                        if(robot.proximity[i].event_enable) strcpy(AlgoidResponse[ptrBuff].DINresponse.event_state, "on");
                        else strcpy(AlgoidResponse[ptrBuff].DINresponse.event_state, "off");     
                        
			ptrBuff++;
			//printf("CHANGEMENT DIN%d, ETAT:%d\n", i, robot.proximity[i].state);
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
		oldBtnValue[i]=robot.button[i].state;
		robot.button[i].state = getButtonInput(i);

		// V�rifie si un changement a eu lieu sur les entrees et transmet un message
		// "event" listant les modifications
		if(robot.button[i].event_enable && (oldBtnValue[i] != robot.button[i].state)){
			AlgoidResponse[ptrBuff].BTNresponse.id=i;
			AlgoidResponse[ptrBuff].value=robot.button[i].state;

                        if(robot.button[i].event_enable) strcpy(AlgoidResponse[ptrBuff].BTNresponse.event_state, "on");
                        else strcpy(AlgoidResponse[ptrBuff].BTNresponse.event_state, "off");
                        
			ptrBuff++;
//			printf("CHANGEMENT BOUTON %d, ETAT:%d\n", i, robot.button[i].state);
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
		if(robot.rgb[i].event_enable){

			int red_event_low_disable, red_event_high_disable;                     
                        int redLowDetected, redHighDetected;
                        
                        int green_event_low_disable, green_event_high_disable;                     
                        int greenLowDetected, greenHighDetected;
                        
                        int blue_event_low_disable, blue_event_high_disable;                     
                        int blueLowDetected, blueHighDetected;

			// Contr�le l' individuelle des evenements sur changement de couleur [ROUGE]
			if(robot.rgb[i].red.event_low < 0) red_event_low_disable = 1;
			else red_event_low_disable = 0;

			if(robot.rgb[i].red.event_high < 0) red_event_high_disable = 1;
			else red_event_high_disable = 0;

			// Detection des seuils d'alarme
			if(robot.rgb[i].red.value < robot.rgb[i].red.event_low) redLowDetected = 1;
			else redLowDetected = 0;

			if(robot.rgb[i].red.value > robot.rgb[i].red.event_high) redHighDetected = 1;
			else redHighDetected = 0;
                        
			// Evaluation des alarmes � envoyer
			if((redLowDetected && !red_event_low_disable) || (redHighDetected && !red_event_high_disable)){				// Mesure tension hors plage
				if(RGB_red_WarningSended[i]==0){														// N'envoie qu'une seule fois l'EVENT
					AlgoidResponse[ptrBuff].RGBresponse.id=i;
					AlgoidResponse[ptrBuff].RGBresponse.red.value=robot.rgb[i].red.value;
					ptrBuff++;
                                        RGBevent++;
                                        //sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, COLORS, 1);
					RGB_red_WarningSended[i]=1;
                                        
                                        // Si evenement pour stream activ�, envoie une trame de type status
                                        if(sysConfig.dataStream.onEvent==1)
                                            makeStatusRequest(DATAFLOW);
//                                        printf("CHANGEMENT ROUGE RGB %d, VALUE:%d\n", i, robot.rgb[i].red.value);
				}
			}
                        
			// Envoie un �venement Fin de niveau bas (+50mV Hysterese)
			else if (RGB_red_WarningSended[i]==1 && robot.rgb[i].red.value > (robot.rgb[i].red.event_low + robot.rgb[i].red.event_hysteresis)){				// Mesure tension dans la plage
					AlgoidResponse[ptrBuff].RGBresponse.id=i;											// n'envoie qu'une seule fois apr�s
					AlgoidResponse[ptrBuff].RGBresponse.red.value=robot.rgb[i].red.value;											// une hysterese de 50mV
                                        ptrBuff++;
                                        RGBevent++;
					//sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, COLORS, 1);
					RGB_red_WarningSended[i]=0;
                                        // Si evenement pour stream activ�, envoie une trame de type status
                                        if(sysConfig.dataStream.onEvent==1)
                                            makeStatusRequest(DATAFLOW);
 //                                        printf("- CHANGEMENT ROUGE RGB %d, VALUE:%d\n", i, robot.rgb[i].red.value);
			}
                        
                        // Contr�le l' individuelle des evenements sur changement de couleur [VERT]
			if(robot.rgb[i].green.event_low < 0) green_event_low_disable = 1;
			else green_event_low_disable = 0;

			if(robot.rgb[i].green.event_high < 0) green_event_high_disable = 1;
			else green_event_high_disable = 0;

			// Detection des seuils d'alarme
			if(robot.rgb[i].green.value < robot.rgb[i].green.event_low) greenLowDetected = 1;
			else greenLowDetected = 0;

			if(robot.rgb[i].green.value > robot.rgb[i].green.event_high) greenHighDetected = 1;
			else greenHighDetected = 0;

			// Evaluation des alarmes � envoyer
			if((greenLowDetected && !green_event_low_disable) || (greenHighDetected && !green_event_high_disable)){				// Mesure tension hors plage
				if(RGB_green_WarningSended[i]==0){														// N'envoie qu'une seule fois l'EVENT
					AlgoidResponse[ptrBuff].RGBresponse.id=i;
					AlgoidResponse[ptrBuff].RGBresponse.green.value=robot.rgb[i].green.value;
                                        ptrBuff++;
                                        RGBevent++;
					//sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, COLORS, 1);
					RGB_green_WarningSended[i]=1;
                                        
                                        // Si evenement pour stream activ�, envoie une trame de type status
                                        if(sysConfig.dataStream.onEvent==1)
                                            makeStatusRequest(DATAFLOW);
 //                                        printf("CHANGEMENT VERT RGB %d, VALUE:%d\n", i, robot.rgb[i].green.value);
				}
			}
                        
			// Envoie un �venement Fin de niveau bas (+50mV Hysterese)
			else if (RGB_green_WarningSended[i]==1 && robot.rgb[i].green.value > (robot.rgb[i].green.event_low + robot.rgb[i].green.event_hysteresis)){				// Mesure tension dans la plage
					AlgoidResponse[ptrBuff].RGBresponse.id=i;											// n'envoie qu'une seule fois apr�s
					AlgoidResponse[ptrBuff].RGBresponse.green.value=robot.rgb[i].green.value;											// une hysterese de 50mV
                                        ptrBuff++;
                                        RGBevent++;
					//sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, COLORS, 1);
					RGB_green_WarningSended[i]=0;
                                        // Si evenement pour stream activ�, envoie une trame de type status
                                        if(sysConfig.dataStream.onEvent==1)
                                            makeStatusRequest(DATAFLOW);
 //                                       printf("-CHANGEMENT VERT RGB %d, VALUE:%d\n", i, robot.rgb[i].green.value);
			}
                        
                        
                        // Contr�le l' individuelle des evenements sur changement de couleur [BLEU]
			if(robot.rgb[i].blue.event_low < 0) blue_event_low_disable = 1;
			else blue_event_low_disable = 0;

			if(robot.rgb[i].blue.event_high < 0) blue_event_high_disable = 1;
			else blue_event_high_disable = 0;

			// Detection des seuils d'alarme
			if(robot.rgb[i].blue.value < robot.rgb[i].blue.event_low) blueLowDetected = 1;
			else blueLowDetected = 0;

			if(robot.rgb[i].blue.value > robot.rgb[i].blue.event_high) blueHighDetected = 1;
			else blueHighDetected = 0;

			// Evaluation des alarmes � envoyer
			if((blueLowDetected && !blue_event_low_disable) || (blueHighDetected && !blue_event_high_disable)){				// Mesure tension hors plage
				if(RGB_blue_WarningSended[i]==0){														// N'envoie qu'une seule fois l'EVENT
					AlgoidResponse[ptrBuff].RGBresponse.id=i;
					AlgoidResponse[ptrBuff].RGBresponse.blue.value=robot.rgb[i].blue.value;
                                        ptrBuff++;
                                        RGBevent++;
					//sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, COLORS, 1);
					RGB_blue_WarningSended[i]=1;
                                        
                                        // Si evenement pour stream activ�, envoie une trame de type status
                                        if(sysConfig.dataStream.onEvent==1)
                                            makeStatusRequest(DATAFLOW);
//                                        printf("CHANGEMENT BLEU RGB %d, VALUE:%d\n", i, robot.rgb[i].blue.value);
				}
			}
                        
			// Envoie un �venement Fin de niveau bas (+50mV Hysterese)
			else if (RGB_blue_WarningSended[i]==1 && robot.rgb[i].blue.value > (robot.rgb[i].blue.event_low + robot.rgb[i].blue.event_hysteresis)){				// Mesure tension dans la plage
					AlgoidResponse[ptrBuff].RGBresponse.id=i;											// n'envoie qu'une seule fois apr�s
					AlgoidResponse[ptrBuff].RGBresponse.blue.value=robot.rgb[i].blue.value;											// une hysterese de 50mV
                                        ptrBuff++;
                                        RGBevent++;
					//sendResponse(AlgoidCommand.msgID, AlgoidCommand.msgFrom, EVENT, COLORS, 1);
					RGB_blue_WarningSended[i]=0;
                                        // Si evenement pour stream activ�, envoie une trame de type status
                                        if(sysConfig.dataStream.onEvent==1)
                                            makeStatusRequest(DATAFLOW);
 //                                       printf("-CHANGEMENT BLEU RGB %d, VALUE:%d\n", i, robot.rgb[i].blue.value);
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

int runCloudTestCommand(void){
    int status=0;
        sendMqttReport(AlgoidCommand.msgID, "Try to ping cloud server on vps596769.ovh.net...");// Envoie le message sur le canal MQTT "Report"   
        status=system("ping -q -c 2 -t 1000 vps596769.ovh.net");
        printf("------------- ping status on vps596769.ovh.net: %d\n", status);
        if(status != 0)
            sysInfo.wan_online = 0;
        else sysInfo.wan_online = 1;
   
    return sysInfo.wan_online;
}

void resetConfig(void){
    int i;
      
    	// Init robot membre
	for(i=0;i<NBAIN;i++){
		robot.battery[i].event_enable=DEFAULT_EVENT_STATE;
		robot.battery[i].event_high=65535;
		robot.battery[i].event_low=0;
	}
    

	for(i=0;i<NBDIN;i++){
		robot.proximity[i].event_enable=DEFAULT_EVENT_STATE;
	}

    
        for(i=0;i<NBBTN;i++){
		robot.button[i].event_enable=DEFAULT_EVENT_STATE;
	}
    
        for(i=0;i<NBMOTOR;i++){
            
		robot.motor[i].accel=100;        
                robot.motor[i].decel=100;
                robot.motor[i].distance_cm=0;           // ATTENTION, BUG SEGFAULT !!!!!
                robot.motor[i].velocity=0;
                robot.motor[i].time=0;
                robot.motor[i].cm=0;
            
                sysConfig.motor[i].inverted=0;
                sysConfig.motor[i].minRPM=20;
                sysConfig.motor[i].maxRPM=200;
                sysConfig.motor[i].rpmRegulator.PIDstate=0;
                sysConfig.motor[i].rpmRegulator.PID_Kp=0.0;
                sysConfig.motor[i].rpmRegulator.PID_Ki=0.0;
                sysConfig.motor[i].rpmRegulator.PID_Kd=0.0;
	}  

        for(i=0;i<NBSTEPPER;i++){
            
		robot.stepper[i].angle=0;        // ATTENTION, BUG SEGFAULT !!!!!
		robot.stepper[i].rotation=0;
           	robot.stepper[i].step=0;
		robot.stepper[i].speed=0;
		robot.stepper[i].time=0;
               
                sysConfig.stepper[i].inverted=0;
                sysConfig.stepper[i].ratio=64;
                sysConfig.stepper[i].stepPerRot=32;
	}

        for(i=0;i<NBSONAR;i++){
		robot.distance[i].event_enable=DEFAULT_EVENT_STATE;
                robot.distance[i].event_high=100;
                robot.distance[i].event_low=15;
                robot.distance[i].event_hysteresis=0;
                robot.distance[i].value=-1;
	}
        
      
        
        for(i=0;i<NBRGBC;i++){
                robot.rgb[i].event_enable=DEFAULT_EVENT_STATE;
                
		robot.rgb[i].red.value=-1;
		robot.rgb[i].red.event_low=0;
		robot.rgb[i].red.event_high=65535;
                
                robot.rgb[i].green.value=-1;
		robot.rgb[i].green.event_low=0;
		robot.rgb[i].green.event_high=65535;
                
                robot.rgb[i].blue.value=-1;
		robot.rgb[i].blue.event_low=0;
		robot.rgb[i].blue.event_high=65535;
                
                robot.rgb[i].clear.value=-1;
		robot.rgb[i].clear.event_low=0;
		robot.rgb[i].clear.event_high=65535;
	}

        
        
        // ------------ Initialisation de la configuration systeme
        
        // Initialisation configuration de flux de donn�es periodique
        sysConfig.dataStream.state=ON;
        sysConfig.dataStream.time_ms=500;
        sysConfig.config.reset=0;
        
                
        // Load config data
        int configStatus = LoadConfig(&sysConfig, "kehops.cfg");
        if(configStatus<0){
            printf("#[CORE] Load configuration file from \"kehops.cfg\": ERROR\n");
        }else
            printf("#[CORE] Load configuration file from \"kehops.cfg\": OK\n");
            
        sysInfo.startUpTime=0;
}

int getStartupArg(int count, char *arg[]){
    unsigned char i;
    
    for(i=0;i<count;i++){
        //printf("ARG #%d : %s\n", count, arg[i]);
        
        if(!strcmp(arg[i], "-n"))
            sprintf(&ClientID, "%s", arg[i+1]);
        
        if(!strcmp(arg[i], "-a"))
            sprintf(&ADDRESS, "%s", arg[i+1]);
    }
}
