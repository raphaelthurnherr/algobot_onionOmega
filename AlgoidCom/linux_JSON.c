/*
 ============================================================================
 Name        : linux_JSON.c
 Author      : Raph
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */



#define KEY_TO "{'MsgTo'"
#define KEY_FROM "{'MsgFrom'"
#define KEY_MSGID "{'MsgID'"
#define KEY_MESSAGE ""
#define KEY_MESSAGE_TYPE "{'MsgData'{'MsgType'"
#define KEY_MESSAGE_PARAM "{'MsgData'{'MsgParam'"
#define KEY_MESSAGE_VALUE "{'MsgData'{'MsgValue'"

#define KEY_MESSAGE_VALUE_DIN "{'MsgData'{'MsgValue'[*{'din'"
#define KEY_MESSAGE_VALUE_BTN "{'MsgData'{'MsgValue'[*{'btn'"
#define KEY_MESSAGE_VALUE_SONAR "{'MsgData'{'MsgValue'[*{'sonar'"
#define KEY_MESSAGE_VALUE_ANGLE "{'MsgData'{'MsgValue'[*{'angle'"
#define KEY_MESSAGE_VALUE_BATT "{'MsgData'{'MsgValue'[*{'battery'"
#define KEY_MESSAGE_VALUE_RGB "{'MsgData'{'MsgValue'[*{'rgb'"

#define KEY_MESSAGE_VALUE_PWM "{'MsgData'{'MsgValue'[*{'pwm'"
#define KEY_MESSAGE_VALUE_LED "{'MsgData'{'MsgValue'[*{'led'"
#define KEY_MESSAGE_VALUE_POWER "{'MsgData'{'MsgValue'[*{'power'"
#define KEY_MESSAGE_VALUE_STATE "{'MsgData'{'MsgValue'[*{'state'"
#define KEY_MESSAGE_VALUE_COUNT "{'MsgData'{'MsgValue'[*{'count'"
#define KEY_MESSAGE_VALUE_POSPERCENT "{'MsgData'{'MsgValue'[*{'position'"


#define KEY_MESSAGE_VALUE_EVENT_STATE "{'MsgData'{'MsgValue'[*{'event'"
#define KEY_MESSAGE_VALUE_EVENT_LOWER "{'MsgData'{'MsgValue'[*{'event_lower'"
#define KEY_MESSAGE_VALUE_EVENT_HIGHER "{'MsgData'{'MsgValue'[*{'event_higher'"
#define KEY_MESSAGE_VALUE_EVENT_RED_LOWER "{'MsgData'{'MsgValue'[*{'red'{'event_lower'"
#define KEY_MESSAGE_VALUE_EVENT_RED_HIGHER "{'MsgData'{'MsgValue'[*{'red'{'event_higher'"
#define KEY_MESSAGE_VALUE_EVENT_GREEN_LOWER "{'MsgData'{'MsgValue'[*{'green'{'event_lower'"
#define KEY_MESSAGE_VALUE_EVENT_GREEN_HIGHER "{'MsgData'{'MsgValue'[*{'green'{'event_higher'"
#define KEY_MESSAGE_VALUE_EVENT_BLUE_LOWER "{'MsgData'{'MsgValue'[*{'blue'{'event_lower'"
#define KEY_MESSAGE_VALUE_EVENT_BLUE_HIGHER "{'MsgData'{'MsgValue'[*{'blue'{'event_higher'"
#define KEY_MESSAGE_VALUE_EVENT_CLEAR_LOWER "{'MsgData'{'MsgValue'[*{'clear'{'event_lower'"
#define KEY_MESSAGE_VALUE_EVENT_CLEAR_HIGHER "{'MsgData'{'MsgValue'[*{'clear'{'event_higher'"

#define KEY_MESSAGE_VALUE_MOTOR "{'MsgData'{'MsgValue'[*{'motor'"
#define KEY_MESSAGE_VALUE_VELOCITY "{'MsgData'{'MsgValue'[*{'velocity'"
#define KEY_MESSAGE_VALUE_TIME "{'MsgData'{'MsgValue'[*{'time'"
#define KEY_MESSAGE_VALUE_CM "{'MsgData'{'MsgValue'[*{'cm'"
#define KEY_MESSAGE_VALUE_STEP "{'MsgData'{'MsgValue'[*{'step'"
#define KEY_MESSAGE_VALUE_ROTATION "{'MsgData'{'MsgValue'[*{'rotation'"

#define KEY_MESSAGE_VALUE_CFG_RESET "{'MsgData'{'MsgValue'[*{'config'{'reset'"
#define KEY_MESSAGE_VALUE_CFG_SAVE "{'MsgData'{'MsgValue'[*{'config'{'save'"

#define KEY_MESSAGE_VALUE_CFG_STREAM_STATE "{'MsgData'{'MsgValue'[*{'stream'{'state'"
#define KEY_MESSAGE_VALUE_CFG_STREAM_TIME "{'MsgData'{'MsgValue'[*{'stream'{'time'"
#define KEY_MESSAGE_VALUE_CFG_STREAM_ONEVENT "{'MsgData'{'MsgValue'[*{'stream'{'onEvent'"

#define KEY_MESSAGE_VALUE_CFG_MOTOR "{'MsgData'{'MsgValue'[*{'motor'"
#define KEY_MESSAGE_VALUE_CFG_MOTOR_ID "{'MsgData'{'MsgValue'[{'motor'[*{'motor'"
#define KEY_MESSAGE_VALUE_CFG_MOTOR_INVERT "{'MsgData'{'MsgValue'[{'motor'[*{'inverted'"
#define KEY_MESSAGE_VALUE_CFG_MOTOR_MINPWM "{'MsgData'{'MsgValue'[{'motor'[*{'pwmMin'"
#define KEY_MESSAGE_VALUE_CFG_MOTOR_MINRPM "{'MsgData'{'MsgValue'[{'motor'[*{'rpmMin'"
#define KEY_MESSAGE_VALUE_CFG_MOTOR_MAXRPM "{'MsgData'{'MsgValue'[{'motor'[*{'rpmMax'"
#define KEY_MESSAGE_VALUE_CFG_MOTOR_PIDREG "{'MsgData'{'MsgValue'[{'motor'[*{'rpmRegulator'{'state'"
#define KEY_MESSAGE_VALUE_CFG_MOTOR_PIDKP "{'MsgData'{'MsgValue'[{'motor'[*{'rpmRegulator'{'PID_Kp'"
#define KEY_MESSAGE_VALUE_CFG_MOTOR_PIDKI "{'MsgData'{'MsgValue'[{'motor'[*{'rpmRegulator'{'PID_Ki'"
#define KEY_MESSAGE_VALUE_CFG_MOTOR_PIDKD "{'MsgData'{'MsgValue'[{'motor'[*{'rpmRegulator'{'PID_Kd'"

#define KEY_MESSAGE_VALUE_CFG_WHEEL "{'MsgData'{'MsgValue'[*{'wheel'"
#define KEY_MESSAGE_VALUE_CFG_WHEEL_ID "{'MsgData'{'MsgValue'[{'wheel'[*{'wheel'"
#define KEY_MESSAGE_VALUE_CFG_WHEEL_DIAMETER "{'MsgData'{'MsgValue'[{'wheel'[*{'diameter'"
#define KEY_MESSAGE_VALUE_CFG_WHEEL_PULSES "{'MsgData'{'MsgValue'[{'wheel'[*{'pulses'"

#define KEY_MESSAGE_VALUE_CFG_STEPPER "{'MsgData'{'MsgValue'[*{'stepper'"
#define KEY_MESSAGE_VALUE_CFG_STEPPER_ID "{'MsgData'{'MsgValue'[{'stepper'[*{'motor'"
#define KEY_MESSAGE_VALUE_CFG_STEPPER_INVERT "{'MsgData'{'MsgValue'[{'stepper'[*{'inverted'"
#define KEY_MESSAGE_VALUE_CFG_STEPPER_RATIO "{'MsgData'{'MsgValue'[{'stepper'[*{'ratio'"
#define KEY_MESSAGE_VALUE_CFG_STEPPER_STEPS "{'MsgData'{'MsgValue'[{'stepper'[*{'steps'"

#define KEY_MESSAGE_VALUE_CFG_LED "{'MsgData'{'MsgValue'[*{'led'"
#define KEY_MESSAGE_VALUE_CFG_LED_ID "{'MsgData'{'MsgValue'[{'led'[*{'led'"
#define KEY_MESSAGE_VALUE_CFG_LED_STATE "{'MsgData'{'MsgValue'[{'led'[*{'state'"
#define KEY_MESSAGE_VALUE_CFG_LED_POWER "{'MsgData'{'MsgValue'[{'led'[*{'power'"

#define KEY_MESSAGE_VALUE_SYS_APP "{'MsgData'{'MsgValue'[*{'application'"
#define KEY_MESSAGE_VALUE_SYS_FIRMWARE "{'MsgData'{'MsgValue'[*{'firmware'"
#define KEY_MESSAGE_VALUE_SYS_WEBAPP "{'MsgData'{'MsgValue'[*{'webAppUpdate'"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "type.h"
#include "../buggy_descriptor.h"
#include "linux_json.h"
#include "libs/lib_json/jRead.h"
#include "libs/lib_json/jWrite.h"

void ackToJSON(char * buffer, int msgId, char* to, char * from, char * msgType,char * msgParam, unsigned char orgType, unsigned char count);
char GetAlgoidMsg(ALGOID destMessage, char *srcDataBuffer);

ALGOID myReplyMessage;

// -----------------------------------------------------------------------------
// MAIN, APPLICATION PRINCIPALE-------------------------------------------------
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// GetAlgoidMsg
// Get message from buffer and set in the message structure
// -----------------------------------------------------------------------------

char GetAlgoidMsg(ALGOID destMessage, char *srcBuffer){
	struct jReadElement element, cfg_device_list;
	int i;

	// ENTETE DE MESSAGE
		jRead_string((char *)srcBuffer, KEY_TO, AlgoidMessageRX.msgTo, 15, NULL );
		jRead_string((char *)srcBuffer, KEY_FROM, AlgoidMessageRX.msgFrom, 15, NULL );
		AlgoidMessageRX.msgID= jRead_int((char *)srcBuffer,  KEY_MSGID, NULL);

	// MESSAGE TYPE
				char myDataString[20];
				// Clear string
				for(i=0;i<20;i++) myDataString[i]=0;

				jRead_string((char *)srcBuffer,  KEY_MESSAGE_TYPE,myDataString,15, NULL);

				AlgoidMessageRX.msgType= ERR_TYPE;
				if(!strcmp(myDataString, "command")) AlgoidMessageRX.msgType = COMMAND;
				if(!strcmp(myDataString, "request")) AlgoidMessageRX.msgType = REQUEST;
				if(!strcmp(myDataString, "ack")) AlgoidMessageRX.msgType = ACK;
				if(!strcmp(myDataString, "response")) AlgoidMessageRX.msgType = RESPONSE;
				if(!strcmp(myDataString, "event")) AlgoidMessageRX.msgType = EVENT;
				if(!strcmp(myDataString, "error")) AlgoidMessageRX.msgType = ERR_TYPE;

	// MESSAGE PARAM
				// Clear string
				for(i=0;i<20;i++) myDataString[i]=0;
				jRead_string((char *)srcBuffer,  KEY_MESSAGE_PARAM,myDataString,15, NULL);

				AlgoidMessageRX.msgParam=ERR_PARAM;
					if(!strcmp(myDataString, "stop")) AlgoidMessageRX.msgParam = STOP;
					if(!strcmp(myDataString, "motor")) AlgoidMessageRX.msgParam = MOTORS;
					if(!strcmp(myDataString, "pwm")) AlgoidMessageRX.msgParam = pPWM;
					if(!strcmp(myDataString, "led")) AlgoidMessageRX.msgParam = pLED;
                                        if(!strcmp(myDataString, "servo")) AlgoidMessageRX.msgParam = pSERVO;
                                        if(!strcmp(myDataString, "button")) AlgoidMessageRX.msgParam = BUTTON;
					if(!strcmp(myDataString, "distance")) AlgoidMessageRX.msgParam = DISTANCE;
					if(!strcmp(myDataString, "battery")) AlgoidMessageRX.msgParam = BATTERY;
					if(!strcmp(myDataString, "din")) AlgoidMessageRX.msgParam = DINPUT;
					if(!strcmp(myDataString, "status")) AlgoidMessageRX.msgParam = STATUS;
                                        if(!strcmp(myDataString, "rgb")) AlgoidMessageRX.msgParam = COLORS;
                                        if(!strcmp(myDataString, "config")) AlgoidMessageRX.msgParam = CONFIG;
                                        if(!strcmp(myDataString, "system")) AlgoidMessageRX.msgParam = SYSTEM;
                                        if(!strcmp(myDataString, "stepper")) AlgoidMessageRX.msgParam = STEPPER;

				  jRead((char *)srcBuffer, KEY_MESSAGE_VALUE, &element );

                                    // RECHERCHE DATA ARRAY
				  if(element.dataType == JREAD_ARRAY ){
                                    AlgoidMessageRX.msgValueCnt=element.elements;

				      for(i=0; i<element.elements; i++ )    // loop for no. of elements in JSON
				      {
				    	  if(AlgoidMessageRX.msgParam == MOTORS){
				    		  AlgoidMessageRX.DCmotor[i].motor=UNKNOWN;	// Initialisation roue inconnue
				    		  //jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_MOTOR, myDataString, 15, &i );
                                                  AlgoidMessageRX.DCmotor[i].motor= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_MOTOR, &i);
                                                 
					    	  AlgoidMessageRX.DCmotor[i].velocity= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_VELOCITY, &i);
					    	  AlgoidMessageRX.DCmotor[i].time= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_TIME, &i);
					    	  AlgoidMessageRX.DCmotor[i].cm= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_CM, &i);
				    	  }
                                          
                                            if(AlgoidMessageRX.msgParam == STEPPER){
                                                AlgoidMessageRX.StepperMotor[i].motor=UNKNOWN;	// Initialisation roue inconnue
                                                //jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_MOTOR, myDataString, 15, &i );
                                                AlgoidMessageRX.StepperMotor[i].motor= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_MOTOR, &i);

                                                AlgoidMessageRX.StepperMotor[i].velocity= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_VELOCITY, &i);
                                                AlgoidMessageRX.StepperMotor[i].angle= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_ANGLE, &i);
                                                AlgoidMessageRX.StepperMotor[i].step= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_STEP, &i);
                                                AlgoidMessageRX.StepperMotor[i].rotation= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_ROTATION, &i);
                                                AlgoidMessageRX.StepperMotor[i].time= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_TIME, &i);
                                            }

				    	  if(AlgoidMessageRX.msgParam == DINPUT){
						 AlgoidMessageRX.DINsens[i].id= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_DIN, &i);
				    		 jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_STATE, AlgoidMessageRX.DINsens[i].event_state, 15, &i );
				    	  }
                                          
                                          if(AlgoidMessageRX.msgParam == BUTTON){
						 AlgoidMessageRX.BTNsens[i].id= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_BTN, &i);
				    		 jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_STATE, AlgoidMessageRX.BTNsens[i].event_state, 15, &i );
				    	  }

				    	  if(AlgoidMessageRX.msgParam == DISTANCE){
				    		  jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_STATE, AlgoidMessageRX.DISTsens[i].event_state, 15, &i );
				    		  AlgoidMessageRX.DISTsens[i].id= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_SONAR, &i);
				    		  AlgoidMessageRX.DISTsens[i].angle= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_ANGLE, &i);
				    		  AlgoidMessageRX.DISTsens[i].event_low= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_LOWER, &i);
				    		  AlgoidMessageRX.DISTsens[i].event_high= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_HIGHER, &i);
				    	  }

                                          if(AlgoidMessageRX.msgParam == COLORS){
				    		  jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_STATE, AlgoidMessageRX.RGBsens[i].event_state, 15, &i );
				    		  AlgoidMessageRX.RGBsens[i].id= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_RGB, &i);
				    		  AlgoidMessageRX.RGBsens[i].red.event_low= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_RED_LOWER, &i);
				    		  AlgoidMessageRX.RGBsens[i].red.event_high= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_RED_HIGHER, &i);
                                                  AlgoidMessageRX.RGBsens[i].green.event_low= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_GREEN_LOWER, &i);
				    		  AlgoidMessageRX.RGBsens[i].green.event_high= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_GREEN_HIGHER, &i);
                                                  AlgoidMessageRX.RGBsens[i].blue.event_low= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_BLUE_LOWER, &i);
				    		  AlgoidMessageRX.RGBsens[i].blue.event_high= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_BLUE_HIGHER, &i);
                                                  AlgoidMessageRX.RGBsens[i].clear.event_low= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_CLEAR_LOWER, &i);
				    		  AlgoidMessageRX.RGBsens[i].clear.event_high= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_CLEAR_HIGHER, &i);
				    	  }

                                          // BATTERY
				    	  if(AlgoidMessageRX.msgParam == BATTERY){
				    		  jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_STATE, AlgoidMessageRX.BATTsens[i].event_state, 15, &i );
				    		  AlgoidMessageRX.BATTsens[i].id= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_BATT, &i);
				    		  AlgoidMessageRX.BATTsens[i].event_low= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_LOWER, &i);
				    		  AlgoidMessageRX.BATTsens[i].event_high= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_HIGHER, &i);
				    	  }
                                          
                                          // LED
				    	  if(AlgoidMessageRX.msgParam == pLED){
                                                  AlgoidMessageRX.LEDarray[i].time=-1;
                                                  AlgoidMessageRX.LEDarray[i].powerPercent=-1;
                                                  strcpy(AlgoidMessageRX.LEDarray[i].state,"null");
                                                  AlgoidMessageRX.LEDarray[i].blinkCount=-1;
                                                  
				    		  jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_STATE, AlgoidMessageRX.LEDarray[i].state, 15, &i );
				    		  int ledId=jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_LED, &i);
				    		  AlgoidMessageRX.LEDarray[i].id=ledId;
				    		  AlgoidMessageRX.LEDarray[i].powerPercent= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_POWER, &i);                                                  
                                                  AlgoidMessageRX.LEDarray[i].time= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_TIME, &i);
                                                  AlgoidMessageRX.LEDarray[i].blinkCount= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_COUNT, &i);
				    	  }
                                          
                                          // PWM
				    	  if(AlgoidMessageRX.msgParam == pPWM){
                                                  AlgoidMessageRX.PWMarray[i].time=-1;
                                                  AlgoidMessageRX.PWMarray[i].powerPercent=-1;
                                                  strcpy(AlgoidMessageRX.PWMarray[i].state,"null");
                                                  AlgoidMessageRX.PWMarray[i].blinkCount=-1;
				    		  
                                                  jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_STATE, AlgoidMessageRX.PWMarray[i].state, 15, &i );
				    		  int pwmId=jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_PWM, &i);
				    		  AlgoidMessageRX.PWMarray[i].id=pwmId;
				    		  AlgoidMessageRX.PWMarray[i].powerPercent= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_POWER, &i);                                                  
                                                  AlgoidMessageRX.PWMarray[i].time= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_TIME, &i);
                                                  AlgoidMessageRX.PWMarray[i].blinkCount= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_COUNT, &i);
				    	  }
                                          
                                          // SERVO
				    	  if(AlgoidMessageRX.msgParam == pSERVO){
                                                  AlgoidMessageRX.PWMarray[i].time=-1;
                                                  AlgoidMessageRX.PWMarray[i].powerPercent=-1;
                                                  strcpy(AlgoidMessageRX.PWMarray[i].state,"null");
                                                  AlgoidMessageRX.PWMarray[i].blinkCount=-1;
                                                  
				    		  jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_STATE, AlgoidMessageRX.PWMarray[i].state, 15, &i );
				    		  int pwmId=jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_PWM, &i);
				    		  AlgoidMessageRX.PWMarray[i].id=pwmId;
				    		  AlgoidMessageRX.PWMarray[i].powerPercent= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_POSPERCENT, &i);                                                                                                   
				    	  }

                                        // STATUS
				    	  if(AlgoidMessageRX.msgParam == STATUS){
                                                  // Nothing to get, return status of all system
				    	  }
                                          
                                        // CONFIGURATION
                                          if(AlgoidMessageRX.msgParam == CONFIG){
                                              int nbOfdeviceInConf;
                                              int i_dev;
                                              
                                              for(i_dev=0;i_dev<4;i_dev++){
                                                AlgoidMessageRX.Config.motor[i_dev].id=-1;
                                              }
                                              
                                            // Stream settings
                                                  jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_STREAM_STATE, AlgoidMessageRX.Config.stream.state, 15, &i );
                                                  AlgoidMessageRX.Config.stream.time= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_STREAM_TIME, &i);
                                                  jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_STREAM_ONEVENT, AlgoidMessageRX.Config.stream.onEvent, 15, &i );
                                                  
                                            // Motor Setting
                                                jRead((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_MOTOR, &cfg_device_list );

                                                // RECHERCHE DATA DE TYPE ARRAY
                                                if(cfg_device_list.dataType == JREAD_ARRAY ){
                                                    // Get the number of motors in array
                                                    nbOfdeviceInConf=cfg_device_list.elements;
                                                    AlgoidMessageRX.Config.motValueCnt=nbOfdeviceInConf;
                                                    
                                                    for(i_dev=0; i_dev < nbOfdeviceInConf; i_dev++){                 
                                                        AlgoidMessageRX.Config.motor[i_dev].id=jRead_int((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_MOTOR_ID, &i_dev); 
                                                        AlgoidMessageRX.Config.motor[i_dev].minRPM=jRead_int((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_MOTOR_MINRPM, &i_dev); 
                                                        AlgoidMessageRX.Config.motor[i_dev].maxRPM=jRead_int((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_MOTOR_MAXRPM, &i_dev); 
                                                        AlgoidMessageRX.Config.motor[i_dev].minPWM=jRead_int((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_MOTOR_MINPWM, &i_dev); 
                                                        jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_MOTOR_INVERT, AlgoidMessageRX.Config.motor[i_dev].inverted, 15, &i_dev ); 
                                                        jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_MOTOR_PIDREG, AlgoidMessageRX.Config.motor[i_dev].rpmRegulator.PIDstate, 15, &i_dev ); 
                                                        AlgoidMessageRX.Config.motor[i_dev].rpmRegulator.PID_Kp=jRead_double((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_MOTOR_PIDKP, &i_dev); 
                                                        AlgoidMessageRX.Config.motor[i_dev].rpmRegulator.PID_Ki=jRead_double((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_MOTOR_PIDKI, &i_dev); 
                                                        AlgoidMessageRX.Config.motor[i_dev].rpmRegulator.PID_Kd=jRead_double((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_MOTOR_PIDKD, &i_dev); 
                                                        
                                                    }
                                                }

                                            // Wheel Setting
                                                jRead((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_WHEEL, &cfg_device_list );

                                                // RECHERCHE DATA DE TYPE ARRAY
                                                if(cfg_device_list.dataType == JREAD_ARRAY ){
                                                    // Get the number of motors in array
                                                    nbOfdeviceInConf=cfg_device_list.elements;
                                                    AlgoidMessageRX.Config.wheelValueCnt=nbOfdeviceInConf;
                                                    
                                                    for(i_dev=0; i_dev < nbOfdeviceInConf; i_dev++){                 
                                                        AlgoidMessageRX.Config.wheel[i_dev].id=jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_WHEEL_ID, &i_dev); 
                                                        AlgoidMessageRX.Config.wheel[i_dev].diameter=jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_WHEEL_DIAMETER, &i_dev); 
                                                        AlgoidMessageRX.Config.wheel[i_dev].pulsesPerRot=jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_WHEEL_PULSES, &i_dev);
                                                    }
                                                }                                                
                                                
                                                // Stepper motor Setting
                                                jRead((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_STEPPER, &cfg_device_list );

                                                // RECHERCHE DATA DE TYPE ARRAY
                                                if(cfg_device_list.dataType == JREAD_ARRAY ){
                                                    // Get the number of motors in array
                                                    nbOfdeviceInConf=cfg_device_list.elements;
                                                    AlgoidMessageRX.Config.stepperValueCnt=nbOfdeviceInConf;
                                                    
                                                    for(i_dev=0; i_dev < nbOfdeviceInConf; i_dev++){                 
                                                        AlgoidMessageRX.Config.stepper[i_dev].id=jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_STEPPER_ID, &i_dev); 
                                                        jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_STEPPER_INVERT, AlgoidMessageRX.Config.stepper[i_dev].inverted, 15, &i_dev ); 
                                                        AlgoidMessageRX.Config.stepper[i_dev].ratio=jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_STEPPER_RATIO, &i_dev); 
                                                        AlgoidMessageRX.Config.stepper[i_dev].stepsPerRot=jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_STEPPER_STEPS, &i_dev); 
                                                    }
                                                }
                                                
                                            // LEDS Setting
                                                jRead((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_LED, &cfg_device_list );

                                                // RECHERCHE DATA DE TYPE ARRAY
                                                if(cfg_device_list.dataType == JREAD_ARRAY ){
                                                    // Get the number of leds in array
                                                    nbOfdeviceInConf=cfg_device_list.elements;
                                                    AlgoidMessageRX.Config.ledValueCnt=nbOfdeviceInConf;
                                                    
                                                    for(i_dev=0; i_dev < nbOfdeviceInConf; i_dev++){
                                                        AlgoidMessageRX.Config.led[i_dev].id=jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_LED_ID, &i_dev); 
                                                        AlgoidMessageRX.Config.led[i_dev].power=jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_LED_POWER, &i_dev); 
                                                        jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_LED_STATE, AlgoidMessageRX.Config.led[i_dev].state, 15, &i_dev ); 
                                                    }
                                                }                                                
                                                
                                            // Reset settings
                                                jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_RESET, AlgoidMessageRX.Config.config.reset, 15, &i );
                                            // Save settings
                                                jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_SAVE, AlgoidMessageRX.Config.config.save, 15, &i );
				    	  }
                                          
                                        // SYSTEM                                          
                                          if(AlgoidMessageRX.msgParam == SYSTEM){
                                                  jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_SYS_FIRMWARE, AlgoidMessageRX.System.firmwareUpdate, 15, &i );
                                                  jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_SYS_APP, AlgoidMessageRX.System.application, 15, &i );
				    	  }
				    }
				  }
				  else
					  AlgoidMessageRX.msgValueCnt=0;

				  if((AlgoidMessageRX.msgParam == ERR_PARAM) || (AlgoidMessageRX.msgType == ERR_TYPE)){
					  return 0;
				  }else
					  return 1;
}

// -----------------------------------------------------------------------------
// replyToHost
// convert the structure in JSON format & Send to host
// -----------------------------------------------------------------------------
void ackToJSON(char * buffer, int msgId, char* to, char* from, char* msgType, char* msgParam, unsigned char orgType, unsigned char count ){
	unsigned int buflen= MAX_MQTT_BUFF;
	unsigned char i,j;
        
// Formatage de la réponse en JSON
	jwOpen( buffer, buflen, JW_OBJECT, JW_PRETTY );		// start root object
		jwObj_string( "MsgTo", to );				// add object key:value pairs
		jwObj_string( "MsgFrom", from );				// add object key:value pairs
		jwObj_int( "MsgID", msgId );
		jwObj_object( "MsgData" );
			jwObj_string( "MsgType", msgType );				// add object key:value pairs
			if(msgParam!=0) jwObj_string( "MsgParam", msgParam );				// add object key:value pairs

			if(count>0){

				//
                                if(orgType==STATUS)
                                    jwObj_object("MsgMap");
                                else
                                    jwObj_array( "MsgValue" );
                                
				for(i=0;i<count;i++){
					//printf("Make array: %d values: %d %d\n", i, 0,9);
					if(orgType!=STATUS)
                                            jwArr_object();

						switch(orgType){
							case MOTORS :                   
                                                                            switch(AlgoidResponse[i].responseType){
                                                                                case EVENT_ACTION_ERROR : jwObj_string("action", "error"); break;
                                                                                case EVENT_ACTION_END : jwObj_string("action", "end"); break;
                                                                                case EVENT_ACTION_BEGIN : jwObj_string("action", "begin"); break;
                                                                                case EVENT_ACTION_RUN : jwObj_string("action", "run"); break;
                                                                                case EVENT_ACTION_ABORT : jwObj_string("action", "abort"); break;
                                                                                case RESP_STD_MESSAGE   :   if(AlgoidResponse[i].MOTresponse.motor>=0)
                                                                                                                jwObj_int( "motor", AlgoidResponse[i].MOTresponse.motor);
                                                                                                            else
                                                                                                                jwObj_string("motor", "unknown");
                                                                                                            jwObj_int( "cm", AlgoidResponse[i].MOTresponse.cm);				// add object key:value pairs
                                                                                                            jwObj_int( "time", AlgoidResponse[i].MOTresponse.time);				// add object key:value pairs
                                                                                                            jwObj_int("velocity", round((AlgoidResponse[i].MOTresponse.velocity)));
                                                                                                            ; break;
                                                                                default : jwObj_string("error", "unknown"); break;
                                                                            }
                                                                            break;

							case STEPPER :                   
                                                                            switch(AlgoidResponse[i].responseType){
                                                                                case EVENT_ACTION_ERROR : jwObj_string("action", "error"); break;
                                                                                case EVENT_ACTION_END : jwObj_string("action", "end"); break;
                                                                                case EVENT_ACTION_BEGIN : jwObj_string("action", "begin"); break;
                                                                                case EVENT_ACTION_RUN : jwObj_string("action", "run"); break;
                                                                                case EVENT_ACTION_ABORT : jwObj_string("action", "abort"); break;
                                                                                case RESP_STD_MESSAGE   :   if(AlgoidResponse[i].STEPPERresponse.motor>=0)
                                                                                                                jwObj_int( "motor", AlgoidResponse[i].STEPPERresponse.motor);
                                                                                                            else
                                                                                                                jwObj_string("motor", "unknown");
                                                                                                            jwObj_int( "step", AlgoidResponse[i].STEPPERresponse.step);				// add object key:value pairs
                                                                                                            jwObj_int( "rotation", AlgoidResponse[i].STEPPERresponse.rotation);				// add object key:value pairs
                                                                                                            jwObj_int("velocity", round((AlgoidResponse[i].STEPPERresponse.velocity)));
                                                                                                            ; break;
                                                                                default : jwObj_string("error", "unknown"); break;
                                                                            }
                                                                            break;                                                                            

							case DISTANCE :                 
                                                                            jwObj_int( "sonar",AlgoidResponse[i].DISTresponse.id);

                                                                            // add object key:value pairs
                                                                            if(AlgoidResponse[i].value >= 0){
                                                                                    jwObj_int("cm", round((AlgoidResponse[i].value)));					// add object key:value pairs
                                                                                    //jwObj_int("angle", AlgoidResponse[i].DISTresponse.angle);				// add object key:value pairs
                                                                                    jwObj_string("event", AlgoidResponse[i].DISTresponse.event_state);				// add object key:value pairs
                                                                                    jwObj_int("event_lower", AlgoidResponse[i].DISTresponse.event_low);				// add object key:value pairs
                                                                                    jwObj_int("event_higher", AlgoidResponse[i].DISTresponse.event_high);				// add object key:value pairs
                                                                            } else
                                                                                    jwObj_string("cm", "error");

                                                                            break;

							case COLORS :                 
                                                                            jwObj_int( "rgb",AlgoidResponse[i].RGBresponse.id);
                                                                            jwObj_string("event", AlgoidResponse[i].RGBresponse.event_state);


                                                                                jwObj_object("color");
                                                                                    jwObj_int("red", AlgoidResponse[i].RGBresponse.red.value);
                                                                                    jwObj_int("green", AlgoidResponse[i].RGBresponse.green.value);
                                                                                    jwObj_int("blue", AlgoidResponse[i].RGBresponse.blue.value);
                                                                                    jwObj_int("clear", AlgoidResponse[i].RGBresponse.clear.value);
                                                                                jwEnd();

                                                                            jwObj_object( "red" );                                                                                 
                                                                                    jwObj_int("event_lower", AlgoidResponse[i].RGBresponse.red.event_low);
                                                                                    jwObj_int("event_higher", AlgoidResponse[i].RGBresponse.red.event_high);
                                                                            jwEnd();

                                                                            jwObj_object( "green" );                                                                                 
                                                                                    jwObj_int("event_lower", AlgoidResponse[i].RGBresponse.green.event_low);
                                                                                    jwObj_int("event_higher", AlgoidResponse[i].RGBresponse.green.event_high);           
                                                                            jwEnd(); 

                                                                            jwObj_object( "blue" );                                                                                 
                                                                                    jwObj_int("event_lower", AlgoidResponse[i].RGBresponse.blue.event_low);
                                                                                    jwObj_int("event_higher", AlgoidResponse[i].RGBresponse.blue.event_high);
                                                                            jwEnd();

                                                                            jwObj_object( "clear" );                                                                                 
                                                                                    jwObj_int("event_lower", AlgoidResponse[i].RGBresponse.clear.event_low);
                                                                                    jwObj_int("event_higher", AlgoidResponse[i].RGBresponse.clear.event_high);
                                                                            jwEnd(); 

                                                                            break;                                                                                

							case BATTERY :                  
                                                                            jwObj_int( "battery",AlgoidResponse[i].BATTesponse.id);

                                                                            // add object key:value pairs
                                                                            if(AlgoidResponse[i].value >= 0){
                                                                                    jwObj_int("mV", AlgoidResponse[i].value);
                                                                                    jwObj_int("capacity", AlgoidResponse[i].BATTesponse.capacity);// add object key:value pairs
                                                                                    jwObj_string("event", AlgoidResponse[i].BATTesponse.event_state);				// add object key:value pairs
                                                                                    jwObj_int("event_lower", AlgoidResponse[i].BATTesponse.event_low);				// add object key:value pairs
                                                                                    jwObj_int("event_higher", AlgoidResponse[i].BATTesponse.event_high);				// add object key:value pairs
                                                                                    jwObj_string("mV", "error");
                                                                            }

                                                                            break;

							case DINPUT :           
                                                                            jwObj_int("din",AlgoidResponse[i].DINresponse.id);	
                                                                            if(AlgoidResponse[i].value >= 0){

                                                                                                     // add object key:value pairs
                                                                                 if(AlgoidResponse[i].value >= 0){
                                                                                         jwObj_int( "state", AlgoidResponse[i].value);				// add object key:value pairs
                                                                                         jwObj_string("event", AlgoidResponse[i].DINresponse.event_state);			// Etat des evenements DIN
                                                                                 } else
                                                                                         jwObj_string("state", "error");
                                                                             }
                                                                            else{
                                                                                 jwObj_string("state", "error");
                                                                            }
                                                                            break;
                                                                                    
							case BUTTON :                  
                                                                            jwObj_int("btn",AlgoidResponse[i].BTNresponse.id);	
                                                                               if(AlgoidResponse[i].value >= 0){

                                                                                                        // add object key:value pairs
                                                                                    if(AlgoidResponse[i].value >= 0){
                                                                                            jwObj_int( "state", AlgoidResponse[i].value);				// add object key:value pairs
                                                                                            jwObj_string("event", AlgoidResponse[i].BTNresponse.event_state);				// add object key:value pairs
                                                                                    } else
                                                                                            jwObj_string("state", "error");
                                                                                }
                                                                               else{
                                                                                    jwObj_string("state", "error");
                                                                               }
                                                                            break;                                                                                    

                                                        case STATUS :               
                                                                            // ETAT DU SYSTEM
                                                                            if(i==0){
                                                                                char wanState[10];

                                                                                jwObj_string("name", AlgoidResponse[i].SYSresponse.name);
                                                                                jwObj_double("upTime",AlgoidResponse[i].SYSresponse.startUpTime);
                                                                                jwObj_string("firmwareVersion",AlgoidResponse[i].SYSresponse.firmwareVersion);	
                                                                                jwObj_string("mcuVersion",AlgoidResponse[i].SYSresponse.mcuVersion);
                                                                                jwObj_string("boardRev",AlgoidResponse[i].SYSresponse.HWrevision);
                                                                                jwObj_double("battery_mv",AlgoidResponse[i].SYSresponse.battVoltage);
                                                                                jwObj_double("battery_capacity",AlgoidResponse[i].SYSresponse.battPercent);                                                                                // add object key:value pairs
                                                                                if(AlgoidResponse[i].SYSresponse.wan_online)
                                                                                    strcpy(wanState, "online");
                                                                                else
                                                                                    strcpy(wanState, "offline");
                                                                                jwObj_string("internet",wanState);                                              // WAN State
                                                                                jwObj_double("messageTX",AlgoidResponse[i].SYSresponse.tx_message);		// System message received
                                                                                jwObj_double("messageRX",AlgoidResponse[i].SYSresponse.rx_message);		// System message received
                                                                            }

                                                                            // ETAT DES DIN
                                                                            if(i>=1 && i<1+NBDIN){
//                                                                                        jwObj_int("din",AlgoidResponse[i].DINresponse.id);		// add object key:value pairs
//                                                                                        jwObj_int( "state", AlgoidResponse[i].value);
                                                                                jwObj_array( "din" );

                                                                                    for(j=0;j<NBDIN;j++){
                                                                                        jwArr_object();
                                                                                            jwObj_int("state",AlgoidResponse[i].value);
                                                                                            jwObj_string("event", AlgoidResponse[i].DINresponse.event_state);			// Etat des evenements DIN
                                                                                        jwEnd();           
                                                                                        i++;
                                                                                    }
                                                                                jwEnd();       
                                                                            }

                                                                            // ETAT DES BOUTON     
                                                                            if(i>=1+NBDIN && i<1+NBDIN+NBBTN){
                                                                            //    jwObj_int("btn",AlgoidResponse[i].BTNresponse.id);		// add object key:value pairs
                                                                            //    jwObj_int( "state", AlgoidResponse[i].value);
                                                                                jwObj_array( "btn" );
                                                                                    for(j=0;j<NBBTN;j++){
                                                                                        jwArr_object();
                                                                                            jwObj_int("state",AlgoidResponse[i].value);
                                                                                            jwObj_string("event", AlgoidResponse[i].BTNresponse.event_state);				// add object key:value pairs                                                                                            
                                                                                        jwEnd();           
                                                                                        i++;
                                                                                    }
                                                                                jwEnd();     
                                                                            }


                                                                            // ETAT DES MOTEUR                                                                                        // ETAT DES AIN                                                                                       // ETAT DES DIN
                                                                            if(i>=1+NBDIN+NBBTN && i<1+NBDIN+NBBTN+NBMOTOR){
                                                                            //        jwObj_int("motor",AlgoidResponse[i].PWMresponse.id);		// add object key:value pairs
                                                                            //        jwObj_int("cm", round((AlgoidResponse[i].MOTresponse.cm)));		// add object key:value pairs
                                                                            //        jwObj_int("speed", round((AlgoidResponse[i].MOTresponse.velocity)));
                                                                                jwObj_array( "motor" );
                                                                                    for(j=0;j<NBMOTOR;j++){
                                                                                        jwArr_object();
                                                                                            jwObj_double("cm",(AlgoidResponse[i].MOTresponse.cm));
                                                                                            jwObj_int("speed",round((AlgoidResponse[i].MOTresponse.speed)));
                                                                                            jwObj_int("setpoint",AlgoidResponse[i].MOTresponse.velocity);
                                                                                        jwEnd();           
                                                                                        i++;
                                                                                    }
                                                                                jwEnd(); 
                                                                            }

                                                                            // ETAT DU SONAR                                                                                        // ETAT DES AIN                                                                                       // ETAT DES DIN
                                                                            if(i>=1+NBDIN+NBBTN+NBMOTOR && i<1+NBDIN+NBBTN+NBMOTOR+NBSONAR){
                                                                            //    jwObj_int("sonar",AlgoidResponse[i].DISTresponse.id);		// add object key:value pairs
                                                                            //    jwObj_int("cm", round((AlgoidResponse[i].value)));
                                                                                jwObj_array( "sonar" );
                                                                                    for(j=0;j<NBSONAR;j++){
                                                                                        jwArr_object();
                                                                                            jwObj_int("cm", round((AlgoidResponse[i].value)));
                                                                                            jwObj_string("event", AlgoidResponse[i].DISTresponse.event_state);                                                                                            
                                                                                        jwEnd();           
                                                                                        i++;
                                                                                    }
                                                                                jwEnd(); 
                                                                            }

                                                                            // ETAT DES CAPTEURS RGB                                                                                                                                                     // ETAT DES AIN                                                                                       // ETAT DES DIN
                                                                            if(i>=1+NBDIN+NBBTN+NBMOTOR+NBSONAR && i<1+NBDIN+NBBTN+NBMOTOR+NBSONAR+NBRGBC){
                                                                                jwObj_array( "rgb" );
                                                                                    for(j=0;j<NBRGBC;j++){
                                                                                        jwArr_object();
                                                                                            jwObj_string("event", AlgoidResponse[i].RGBresponse.event_state);                                                                                        
                                                                                            jwObj_int("red",AlgoidResponse[i].RGBresponse.red.value);
                                                                                            jwObj_int("green",AlgoidResponse[i].RGBresponse.green.value);
                                                                                            jwObj_int("blue",AlgoidResponse[i].RGBresponse.blue.value);
                                                                                            jwObj_int("clear",AlgoidResponse[i].RGBresponse.clear.value);
                                                                                        jwEnd();           
                                                                                        i++;
                                                                                    }
                                                                                jwEnd();                                                                                             
                                                                            }                                                                            
                                                                            
                                                                            // ETAT DES LED                                                                                   // ETAT DES PWM                                                                                        // ETAT DES AIN                                                                                       // ETAT DES DIN
                                                                            if(i>=1+NBDIN+NBBTN+NBMOTOR+NBSONAR+NBRGBC && i<1+NBDIN+NBBTN+NBMOTOR+NBSONAR+NBRGBC+NBLED){
                                                                                jwObj_array( "led" );
                                                                                    for(j=0;j<NBLED;j++){
                                                                                        jwArr_object();
                                                                                            jwObj_int("state",AlgoidResponse[i].value);
                                                                                            jwObj_int("power",AlgoidResponse[i].LEDresponse.powerPercent);
                                                                                        jwEnd();           
                                                                                        i++;
                                                                                    }
                                                                                jwEnd();                                                                                             
                                                                            }                                                                            


                                                                            // ETAT DES PWM                                                                                   // ETAT DES PWM                                                                                        // ETAT DES AIN                                                                                       // ETAT DES DIN
                                                                            if(i>=1+NBDIN+NBBTN+NBMOTOR+NBSONAR+NBRGBC+NBLED && i<1+NBDIN+NBBTN+NBMOTOR+NBSONAR+NBRGBC+NBLED+NBPWM){
                                                                                jwObj_array( "pwm" );
                                                                                    for(j=0;j<NBPWM;j++){
                                                                                        jwArr_object();
                                                                                            jwObj_int("state",AlgoidResponse[i].value);
                                                                                            jwObj_int("power",AlgoidResponse[i].PWMresponse.powerPercent);
                                                                                        jwEnd();           
                                                                                        i++;
                                                                                    }
                                                                                jwEnd();                                                                                             
                                                                            }

                                                                            break;
                                                                                    
                                                        case pPWM :             
                                                                            switch(AlgoidResponse[i].responseType){
                                                                                case EVENT_ACTION_ERROR :   jwObj_string("action", "error");break;
                                                                                case EVENT_ACTION_END  :   jwObj_string("action", "end"); break;
                                                                                case EVENT_ACTION_BEGIN  :   jwObj_string("action", "begin"); break;
                                                                                case EVENT_ACTION_RUN  :   jwObj_string("action", "run"); break;
                                                                                case EVENT_ACTION_ABORT  :   jwObj_string("action", "abort"); break;
                                                                                case RESP_STD_MESSAGE  :   if(AlgoidResponse[i].PWMresponse.id>=0)
                                                                                                jwObj_int( "pwm", AlgoidResponse[i].PWMresponse.id);
                                                                                            else
                                                                                                jwObj_string("pwm", "unknown");
                                                                                            jwObj_string( "state", AlgoidResponse[i].PWMresponse.state);				
                                                                                            jwObj_int( "power", AlgoidResponse[i].PWMresponse.powerPercent);				
                                                                                            jwObj_int("time", AlgoidResponse[i].PWMresponse.time);
                                                                                            break;
                                                                                default :   jwObj_string("error", "unknown");break;
                                                                            }
                                                                            break;

                                                        case pSERVO :             
                                                                            switch(AlgoidResponse[i].responseType){
                                                                                case EVENT_ACTION_ERROR :   jwObj_string("action", "error");break;
                                                                                case EVENT_ACTION_END  :    jwObj_string("action", "end"); break;
                                                                                case EVENT_ACTION_BEGIN  :  jwObj_string("action", "begin"); break;
                                                                                case EVENT_ACTION_ABORT  :  jwObj_string("action", "abort"); break;
                                                                                case RESP_STD_MESSAGE  :    if(AlgoidResponse[i].PWMresponse.id>=0)
                                                                                                                jwObj_int( "pwm", AlgoidResponse[i].PWMresponse.id);
                                                                                                            else
                                                                                                                jwObj_string("pwm", "unknown");
                                                                                                            jwObj_string( "state", AlgoidResponse[i].PWMresponse.state);				
                                                                                                            jwObj_int( "position", AlgoidResponse[i].PWMresponse.powerPercent);				
                                                                                                            break;
                                                                                default :                   jwObj_string("error", "unknown");break;
                                                                            }
                                                                            break;                                                                         
                                                                                   
                                                        case pLED :             
                                                                            switch(AlgoidResponse[i].responseType){
                                                                                case EVENT_ACTION_ERROR :    jwObj_string("action", "error");break;
                                                                                case EVENT_ACTION_END  :     jwObj_string("action", "end"); break;
                                                                                case EVENT_ACTION_BEGIN  :   jwObj_string("action", "begin"); break;
                                                                                case EVENT_ACTION_RUN  :   jwObj_string("action", "run"); break;
                                                                                case EVENT_ACTION_ABORT  :   jwObj_string("action", "abort"); break;
                                                                                case RESP_STD_MESSAGE  :    if(AlgoidResponse[i].LEDresponse.id>=0)
                                                                                                                jwObj_int( "led", AlgoidResponse[i].LEDresponse.id);
                                                                                                            else
                                                                                                                jwObj_string("led", "unknown");
                                                                                                            jwObj_string( "state", AlgoidResponse[i].LEDresponse.state);				// add object key:value pairs
                                                                                                            jwObj_int( "power", AlgoidResponse[i].LEDresponse.powerPercent);				// add object key:value pairs
                                                                                                            jwObj_int("time", AlgoidResponse[i].LEDresponse.time);
                                                                                                            jwObj_int("count", AlgoidResponse[i].LEDresponse.blinkCount);
                                                                                                            break;
                                                                                default :   jwObj_string("error", "unknown");break;
                                                                            }
                                                                            break;
                                                                                
							case CONFIG :         
                                                                            switch(AlgoidResponse[i].responseType){
                                                                                case EVENT_ACTION_ERROR : jwObj_string("action", "error"); break;
                                                                                case EVENT_ACTION_END :   jwObj_string("action", "end"); break;
                                                                                case EVENT_ACTION_BEGIN : jwObj_string("action", "begin"); break;
                                                                                case EVENT_ACTION_ABORT : jwObj_string("action", "abort"); break;
                                                                                case RESP_STD_MESSAGE   :   
                                                                                                            jwObj_object( "config" );                                                                                 
                                                                                                                jwObj_string("reset", AlgoidResponse[i].CONFIGresponse.config.reset);       
                                                                                                                jwObj_string("save", AlgoidResponse[i].CONFIGresponse.config.save);                                                                                                                       
                                                                                                            jwEnd();  
                                                                                                            
                                                                                                            jwObj_object( "stream" );                                                                                 
                                                                                                                    jwObj_string("state", AlgoidResponse[i].CONFIGresponse.stream.state);
                                                                                                                    jwObj_int("time", AlgoidResponse[i].CONFIGresponse.stream.time);
                                                                                                                    jwObj_string("onEvent", AlgoidResponse[i].CONFIGresponse.stream.onEvent);         
                                                                                                            jwEnd();
                                                                                                            
                                                                                                        // CREATE JSON CONFIG FOR MOTOR  
                                                                                                            if(AlgoidResponse[i].CONFIGresponse.motValueCnt > 0){
                                                                                                                jwObj_array("motor");
                                                                                                                    for(j=0;j<AlgoidResponse[i].CONFIGresponse.motValueCnt;j++){
                                                                                                                        jwArr_object();
                                                                                                                            jwObj_int( "motor", AlgoidResponse[i].CONFIGresponse.motor[j].id);
                                                                                                                            jwObj_string("inverted", AlgoidResponse[i].CONFIGresponse.motor[j].inverted);
                                                                                                                            jwObj_int("rpmMin", AlgoidResponse[i].CONFIGresponse.motor[j].minRPM);
                                                                                                                            jwObj_int("rpmMax", AlgoidResponse[i].CONFIGresponse.motor[j].maxRPM);
                                                                                                                        jwEnd();
                                                                                                                    } 
                                                                                                                jwEnd();
                                                                                                            }
                                                                                                            
                                                                                                        // CREATE JSON CONFIG FOR WHEEL  
                                                                                                            if(AlgoidResponse[i].CONFIGresponse.wheelValueCnt > 0){
                                                                                                                jwObj_array("wheel");
                                                                                                                    for(j=0;j<AlgoidResponse[i].CONFIGresponse.wheelValueCnt;j++){
                                                                                                                        jwArr_object();
                                                                                                                            jwObj_int( "wheel", AlgoidResponse[i].CONFIGresponse.wheel[j].id);
                                                                                                                            jwObj_int("diameter", AlgoidResponse[i].CONFIGresponse.wheel[j].diameter);
                                                                                                                            jwObj_int("pulses", AlgoidResponse[i].CONFIGresponse.wheel[j].pulsesPerRot);
                                                                                                                        jwEnd();
                                                                                                                    } 
                                                                                                                jwEnd();
                                                                                                            }         

                                                                                                        // CREATE JSON CONFIG FOR STEPPER  
                                                                                                            if(AlgoidResponse[i].CONFIGresponse.stepperValueCnt > 0){
                                                                                                                jwObj_array("stepper");
                                                                                                                    for(j=0;j<AlgoidResponse[i].CONFIGresponse.wheelValueCnt;j++){
                                                                                                                        jwArr_object();
                                                                                                                            jwObj_int( "motor", AlgoidResponse[i].CONFIGresponse.stepper[j].id);
                                                                                                                            jwObj_string("inverted", AlgoidResponse[i].CONFIGresponse.stepper[j].inverted);
                                                                                                                            jwObj_int("ratio", AlgoidResponse[i].CONFIGresponse.stepper[j].ratio);
                                                                                                                            jwObj_int("steps", AlgoidResponse[i].CONFIGresponse.stepper[j].stepsPerRot);
                                                                                                                        jwEnd();
                                                                                                                    } 
                                                                                                                jwEnd();
                                                                                                            }                                                                                                              
                  
                                                                                                            
                                                                                                        // CREATE JSON CONFIG FOR LED
                                                                                                            if(AlgoidResponse[i].CONFIGresponse.ledValueCnt > 0){
                                                                                                                jwObj_array("led");
                                                                                                                    for(j=0;j<AlgoidResponse[i].CONFIGresponse.ledValueCnt;j++){
                                                                                                                        jwArr_object();
                                                                                                                            jwObj_int( "led", AlgoidResponse[i].CONFIGresponse.led[j].id);
                                                                                                                            jwObj_string("state", AlgoidResponse[i].CONFIGresponse.led[j].state);
                                                                                                                            jwObj_int( "power", AlgoidResponse[i].CONFIGresponse.led[j].power);
                                                                                                                        jwEnd();
                                                                                                                    } 
                                                                                                                jwEnd();                                             
                                                                                                            }
                                                                                                           
                                                                                                            break;
                                                                                default : jwObj_string("error", "unknown"); break;
                                                                            }
                                                                            break;
                                                                                
							case SYSTEM :           
                                                                            switch(AlgoidResponse[i].responseType){
                                                                                case EVENT_ACTION_ERROR : jwObj_string("action", "error"); break;
                                                                                case EVENT_ACTION_END :   jwObj_string("action", "end"); break;
                                                                                case EVENT_ACTION_BEGIN : jwObj_string("action", "begin"); break;
                                                                                case EVENT_ACTION_ABORT : jwObj_string("action", "abort"); break;
                                                                                case RESP_STD_MESSAGE   :                                                                                    
                                                                                                            jwObj_string("application", AlgoidResponse[i].SYSCMDresponse.application);
                                                                                                          ; break;
                                                                                default : jwObj_string("error", "unknown"); break;
                                                                            }		// add object key:value pairs

                                                                            break;                                                                                
                                                                                   
							default:                break;

						}
                                    if(orgType!=STATUS)
                                        jwEnd();
				}
				jwEnd();
			}
		jwEnd();
		jwClose();
}
