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
#include "hwManager.h"
#include <math.h>
#include <../tools.h>

char reportBuffer[256];

//unsigned char motorDCactualPower[2];				// Valeur de la puissance moteur
//unsigned char motorDCtargetPower[2]; 				// Valuer de consigne pour la puissance moteur
//unsigned char motorDCaccelValue[2]={25,25};			// Valeur d'acceleration des moteurs
//unsigned char motorDCdecelValue[2]={25,25};			// Valeur d'acceleration des moteurs

int setAsyncMotorAction(int actionNumber, int motorNb, int veloc, char unit, int value);
int endWheelAction(int actionNumber, int motorNb);
int checkMotorEncoder(int actionNumber, int encoderName);
int dummyMotorAction(int actionNumber, int encoderName);

float rpmToPercent(int motorName, int rpm);
void checkDCmotorPower(void);				// Fonction temporaire pour rampe d'acceleration
//int motorSpeedSetpoint(int motorName, int ratio);  // Applique la consigne de vélocité pour un moteur donné
void setMotorAccelDecel(unsigned char motorNo, char accelPercent, char decelPercent);		// D�fini l'acc�leration/deceleration d'un moteur
// -------------------------------------------------------------------
// SETASYNCMOTORACTION
// Effectue l'action sur une roue sp�cifi�e
// - D�marrage du timer avec definition de fonction call-back, et no d'action
// - D�marrage du mouvement de la roue sp�cifi�e
// - V�locit� entre -100 et +100 qui d�fini le sens de rotation du moteur
// -------------------------------------------------------------------

int setAsyncMotorAction(int actionNumber, int motorNb, int veloc, char unit, int value){
	int setTimerResult;
	int endOfTask;  
        int motorPWM;
        
	if(veloc == 0){
		robot.motor[motorNb].direction = BUGGY_STOP;
	}else
        {
            if(veloc < 0){
                // Check if motor inversion requiered and modify if necessary
                if(!sysConfig.motor[motorNb].inverted)
                    robot.motor[motorNb].direction=BUGGY_FORWARD;
                else 
                    robot.motor[motorNb].direction=BUGGY_BACK;
                robot.motor[motorNb].velocity *=-1;
            }else{      
                // Check if motor inversion requiered and modify if necessary
                if(!sysConfig.motor[motorNb].inverted) 
                    robot.motor[motorNb].direction=BUGGY_BACK;
                else
                    robot.motor[motorNb].direction=BUGGY_FORWARD;

            }            
        }


	// D�marre de timer d'action sur la roue et sp�cifie la fonction call back � appeler en time-out
	// Valeur en retour >0 signifie que l'action "en retour" � �t� �cras�e
	switch(unit){
		case  MILLISECOND:  setTimerResult=setTimer(value, &endWheelAction, actionNumber, motorNb, MOTOR); break;
		case  CENTIMETER:   //motorNb = getOrganNumber(motorNb);
                                    robot.encoder[motorNb].startEncoderValue=getMotorPulses(motorNb)*(sysConfig.wheel[motorNb]._MMPP/10); // (/10 = Convert millimeter per pulse to centimeter per pulse)
                                    robot.encoder[motorNb].stopEncoderValue = robot.encoder[motorNb].startEncoderValue+ value;
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
                
		// Défini le "nouveau" sens de rotation à applique au moteur ainsi que la consigne de vitesse
		if(setMotorDirection(motorNb, robot.motor[motorNb].direction)){                                                            // Sens de rotation
                    printf("\n----  SET DIRECTION: %d  \n", robot.motor[motorNb].direction);
                        //motorPWM = RPMToPercent(motorNb, veloc);                                                   // Mise à l'échelle d'un % "utilisateur" en PWM % utilisable par le moteur
                        //motorSpeedSetpoint(motorNb, motorPWM);                                                          // Vitesse
                        
                        //printf("\n[setAsyncMotorAction()] New PWM Setpoint: %d\n", motorPWM );  


			// Envoie de message ALGOID et SHELL
			sprintf(reportBuffer, "Start wheel %d with power %d for time %d\n", motorNb, veloc, value);
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
	//setMotorSpeed(motorNb, 0);
        //motorSpeedSetpoint(motorNb, 0);
        robot.motor[motorNb].velocity=0;
        robot.motor[motorNb].direction = BUGGY_STOP;
        
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

	distance = getMotorPulses(encoderName);

	if(distance >=0){
		distance = distance * (sysConfig.wheel[encoderName]._MMPP/10);
    	usleep(2200);
	}else  printf("\n ERROR: I2CBUS READ\n");
	//printf("\n Encodeur #%d -> START %.2f cm  STOP %.2f cm", encoderNumber, startEncoderValue[encoderNumber], stopEncoderValue[encoderNumber]);

	if(distance >= robot.encoder[encoderName].stopEncoderValue){
		endWheelAction(actionNumber, encoderName);
        }
	else{
		setTimer(50, &checkMotorEncoder, actionNumber, encoderName, MOTOR);
        }

	return 0;
}

int dummyMotorAction(int actionNumber, int encoderName){
        setTimer(0, &dummyMotorAction, actionNumber, encoderName, MOTOR);
    return 0;
}



// ----------------------------------------------------------------------
// RPMToPercent
// Redefini l'echelle donnée en % en une échelle utilisable
// selon les caractéristique du motor (par ex. PWM minimum pour démarrage moteur)
// donnés dans le fichier de configuration
// -----------------------------------------------------------------------

float rpmToPercent(int motorName, int rpm){
        float RPMpercent = 0;                 // % minimum pour fonctionnement du moteur 
        int newRatio;
        
        RPMpercent = (rpm * 100) / (float)(sysConfig.motor[motorName].maxRPM);
           
        return RPMpercent;
}

// ------------------------------------------------------------------------------------
// CHECKDCMOTORPOWER:
// Fonction appel�e periodiquement pour la gestion de l'acceleration
// D�cel�ration du moteur.
// Elle va augmenter ou diminuer la velocite du moteur jusqu'a atteindre la consigne
// ------------------------------------------------------------------------------------
void checkDCmotorPower(void){
        static float oldSetpoint;
	unsigned char i;
        float userSetpoint;
        float normalizeSetpoint;
        float pidSetpoint;
        float actualRpmInPercent;
        int fixedSetpoint;
        float newSetpoint;
        
	//unsigned char PowerToSet;

	// Contr�le successivement la puissance sur chaque moteur et effectue une rampe d'acc�l�ration ou d�c�leration
	for(i=0;i<NBMOTOR;i++){
            
            // Converti la consigne donnée en % en consigne  CM/SEC
            
            userSetpoint=(float)(robot.motor[i].velocity);
            
            actualRpmInPercent = rpmToPercent(i, robot.motor[i].speed_rpm);
            
            if(userSetpoint <= 0){   
                setMotorSpeed(i, 0);
            }else{
                
                if(sysConfig.motor[i].rpmRegulator.PIDstate > 0){
                //normalizeSetpoint = sysConfig.motor[0].minRPM + userSetpoint + ((float)(sysConfig.motor[0].maxRPM - sysConfig.motor[0].minRPM)/100);
                normalizeSetpoint = rpmToPercent(i,sysConfig.motor[i].minRPM) + userSetpoint;
                                
                //printf("\nnormalizeSetpoint: %.2f", normalizeSetpoint);
                    pidSetpoint = PID_speedControl(i, actualRpmInPercent, normalizeSetpoint);
                    
                    if(pidSetpoint < sysConfig.motor[i].minPWM)
                        pidSetpoint = sysConfig.motor[i].minPWM;
                    newSetpoint = pidSetpoint;  
              
                    
                }else{                    
                    
                    // Crée une consigne avec le PWM minimum pour entretenir le moteur et ajoute la consigne utilisateur
                    fixedSetpoint = sysConfig.motor[i].minPWM + ((100 - (float)sysConfig.motor[i].minPWM) / 100) * userSetpoint;
                    
                    //Ajoute 30% de consigne pour le démarrage moteur si RPM trop bas
                    if(actualRpmInPercent < sysConfig.motor[i].minRPM){
                        fixedSetpoint += ((float)sysConfig.motor[i].minPWM / 100) * 30;
                    }
                        
                    //printf("\n*** MOTOR #%d  USER setpoint [pc]:   %d     Normalized setpoint [pc]:   %.2f       \n",0, robot.motor[0].velocity, userSetpoint);
                    if(fixedSetpoint < sysConfig.motor[i].minPWM){
                        newSetpoint = sysConfig.motor[i].minPWM;
                    }else
                        newSetpoint = fixedSetpoint;
                    
                    //newSetpoint=userSetpoint;
                }
                
                //Ajoute 30% de consigne pour le démarrage moteur si RPM trop bas (20% < RPMmin)
                if(robot.motor[i].speed_rpm < sysConfig.motor[i].minRPM - (sysConfig.motor[i].minRPM / 100 * 50)){
                    newSetpoint += ((float)sysConfig.motor[i].minPWM / 100) * 25;
                    printf("\n !!!!!!! KICK START !!!!!\n");
                }
                
                if((newSetpoint != oldSetpoint)){
                    setMotorSpeed(i, newSetpoint); 

                }
                
                printf("\n* MOTOR #%d  USER setpoint [pc]: %.2f - Actual speed [pc] : %.2f - New SETPOINT [pc]: %.2f\n",i, normalizeSetpoint, actualRpmInPercent, newSetpoint);
                                    
                oldSetpoint = newSetpoint;
            }

            //printf("\n--- MOTOR #%d  ACTUAL RPM: %.1f  ACTUAL RPM [percent]: %.1f   USER SETPOINT: %.1f\n",0 , robot.motor[0].speed_rpm, actualSpeedPercent, userSetpoint);
            //pidSetpoint = PID_speedControl(0, actualSpeedPercent, robot.motor[0].velocity);           
            //setpoint = sysConfig.motor[0].minRPM + RPMToPercent(0, setpoint);
            
            

            
            /*
            //printf("Motor Nb: %d Adr: %2x ActualPower: %d   TargetPower: %d  \n",i, motorDCadr[i], motorDCactualPower[i], motorDCtargetPower[i]);
            if(motorDCactualPower[i] < motorDCtargetPower[i]){
                    //PowerToSet=motorDCactualPower[i] + ((motorDCtargetPower[i]-motorDCactualPower[i])/100)*motorDCaccelValue[i];
                    //printf("Power to set: %d %",PowerToSet);

                    if(motorDCactualPower[i]+motorDCaccelValue[i]<=motorDCtargetPower[i])		// Contr�le que puissance apr�s acceleration ne d�passe pas la consigne
                            motorDCactualPower[i]+=motorDCaccelValue[i];						// Augmente la puissance moteur
                    else motorDCactualPower[i]=motorDCtargetPower[i];						// Attribue la puissance de consigne

                    setMotorSpeed(i, motorDCactualPower[i]);
                    //set_i2c_command_queue(&PCA9685_DCmotorSetSpeed, motorDCadr[i], motorDCactualPower[i]);
                    //PCA9685_DCmotorSetSpeed(motorDCadr[i], motorDCactualPower[i]);
            }

            if(motorDCactualPower[i]>motorDCtargetPower[i]){
                    if(motorDCactualPower[i]-motorDCdecelValue[i]>=motorDCtargetPower[i])		// Contr�le que puissance apr�s acceleration ne d�passe pas la consigne
                            motorDCactualPower[i]-=motorDCdecelValue[i];						// Diminue la puissance moteur
                    else motorDCactualPower[i]=motorDCtargetPower[i];						// Attribue la puissance de consigne

                    setMotorSpeed(i, motorDCactualPower[i]);
                    //set_i2c_command_queue(&PCA9685_DCmotorSetSpeed, motorDCadr[i], motorDCactualPower[i]);
                    //PCA9685_DCmotorSetSpeed(motorDCadr[i], motorDCactualPower[i]);

                    // Ouvre le pont en h de commande moteur
                    if(motorDCactualPower[i]==0)
                            setMotorDirection(i, BUGGY_STOP);
            }
                */
	}
}

/*
// ---------------------------------------------------------------------------
// motorSpeedSetpoint
// Applique la consigne de v�locit� pour un moteur donn�
// Cette consigne n'est pas appliqu�e directement sur les moteur mais sera progressivement
// approchée par le gestionnaire d'acceleration.
// ---------------------------------------------------------------------------
int motorSpeedSetpoint(int motorName, int ratio){
	if(motorName >= 0)
		//motorDCtargetPower[motorName]=ratio;
	else
		printf("\n function [motorSpeedSetpoint] : undefine motor #%d", motorName);
        
        return 0;
}

*/



// -------------------------------------------------------------------
// setMotorAccelDecel
// D�fini les valeurs d'acceleration et decelaration du moteur
// Valeur donn�e en % de ce qu'il reste pour atteindre la consigne
// -------------------------------------------------------------------
void setMotorAccelDecel(unsigned char motorNo, char accelPercent, char decelPercent){
/*
	//unsigned char motorSlot;
	//motorSlot = getOrganNumber(motorNo);

	// R�cup�ration de la valeur absolue de l'acceleration
	if(accelPercent<0) accelPercent*=-1;
	// D�fini un maximum de 100% d'acceleration
	if(accelPercent>100)
		accelPercent=100;

	// R�cup�ration de la valeur absolue de la deceleration
	if(decelPercent<0) decelPercent*=-1;
	// D�fini un maximum de 100% de deceleration
	if(decelPercent>100)
		decelPercent=100;

	// Ne modifie les valeurs d'acceleration et deceleration uniquement si "valable" (=>0)
	if(accelPercent>0)
		motorDCaccelValue[motorNo] = accelPercent;
	if(decelPercent>0)
		motorDCdecelValue[motorNo] = decelPercent;
 */
}
 