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

#define KEY_MESSAGE_VALUE_SONAR "{'MsgData'{'MsgValue'[*{'sonar'"
#define KEY_MESSAGE_VALUE_ANGLE "{'MsgData'{'MsgValue'[*{'angle'"
#define KEY_MESSAGE_VALUE_BATT "{'MsgData'{'MsgValue'[*{'battery'"

#define KEY_MESSAGE_VALUE_PWM "{'MsgData'{'MsgValue'[*{'pwm'"
#define KEY_MESSAGE_VALUE_LED "{'MsgData'{'MsgValue'[*{'led'"
#define KEY_MESSAGE_VALUE_POWER "{'MsgData'{'MsgValue'[*{'power'"
#define KEY_MESSAGE_VALUE_STATE "{'MsgData'{'MsgValue'[*{'state'"
#define KEY_MESSAGE_VALUE_COUNT "{'MsgData'{'MsgValue'[*{'count'"

#define KEY_MESSAGE_VALUE_EVENT_STATE "{'MsgData'{'MsgValue'[*{'event'"
#define KEY_MESSAGE_VALUE_EVENT_LOWER "{'MsgData'{'MsgValue'[*{'event_lower'"
#define KEY_MESSAGE_VALUE_EVENT_HIGHER "{'MsgData'{'MsgValue'[*{'event_higher'"

#define KEY_MESSAGE_VALUE_SAFETY_STOP "{'MsgData'{'MsgValue'[*{'safety_stop'"
#define KEY_MESSAGE_VALUE_SAFETY_VALUE "{'MsgData'{'MsgValue'[*{'safety_value'"



#define KEY_MESSAGE_VALUE_WHEEL "{'MsgData'{'MsgValue'[*{'wheel'"
#define KEY_MESSAGE_VALUE_VELOCITY "{'MsgData'{'MsgValue'[*{'velocity'"
#define KEY_MESSAGE_VALUE_TIME "{'MsgData'{'MsgValue'[*{'time'"
#define KEY_MESSAGE_VALUE_CM "{'MsgData'{'MsgValue'[*{'cm'"
#define KEY_MESSAGE_VALUE_ACCEL "{'MsgData'{'MsgValue'[*{'accel'"
#define KEY_MESSAGE_VALUE_DECEL "{'MsgData'{'MsgValue'[*{'decel'"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../buggy_descriptor.h"
#include "linux_json.h"
#include "libs/lib_json/jRead.h"
#include "libs/lib_json/jWrite.h"

void ackToJSON(char * buffer, int msgId, char* to, char * from, char * msgType,char * msgParam, unsigned char valStr, unsigned char count);
char GetAlgoidMsg(ALGOID destMessage, char *srcBuffer);

ALGOID myReplyMessage;

// -----------------------------------------------------------------------------
// MAIN, APPLICATION PRINCIPALE-------------------------------------------------
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// GetAlgoidMsg
// Get message from buffer and set in the message structure
// -----------------------------------------------------------------------------

char GetAlgoidMsg(ALGOID destMessage, char *srcBuffer){
	struct jReadElement element;
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
				if(!strcmp(myDataString, "negoc")) AlgoidMessageRX.msgType = NEGOC;
				if(!strcmp(myDataString, "error")) AlgoidMessageRX.msgType = ERR_TYPE;

	// MESSAGE PARAM
				// Clear string
				for(i=0;i<20;i++) myDataString[i]=0;
				jRead_string((char *)srcBuffer,  KEY_MESSAGE_PARAM,myDataString,15, NULL);

				AlgoidMessageRX.msgParam=ERR_PARAM;
					if(!strcmp(myDataString, "stop")) AlgoidMessageRX.msgParam = STOP;
					if(!strcmp(myDataString, "move")) AlgoidMessageRX.msgParam = MOVE;
					if(!strcmp(myDataString, "2wd")) AlgoidMessageRX.msgParam = LL_2WD;
					if(!strcmp(myDataString, "pwm")) AlgoidMessageRX.msgParam = pPWM;
					if(!strcmp(myDataString, "led")) AlgoidMessageRX.msgParam = pLED;

					if(!strcmp(myDataString, "distance")) AlgoidMessageRX.msgParam = DISTANCE;
					if(!strcmp(myDataString, "battery")) AlgoidMessageRX.msgParam = BATTERY;
					if(!strcmp(myDataString, "din")) AlgoidMessageRX.msgParam = DINPUT;
					if(!strcmp(myDataString, "status")) AlgoidMessageRX.msgParam = STATUS;

				  jRead((char *)srcBuffer, KEY_MESSAGE_VALUE, &element );

					// RECHERCHE DATA ARRAY
				  if(element.dataType == JREAD_ARRAY ){
					  AlgoidMessageRX.msgValueCnt=element.elements;

				      for(i=0; i<element.elements; i++ )    // loop for no. of elements in JSON
				      {
				    	  if(AlgoidMessageRX.msgParam == LL_2WD){
				    		  AlgoidMessageRX.DCmotor[i].wheel=UNKNOWN;	// Initialisation roue inconnue
				    		  jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_WHEEL, myDataString, 15, &i );
				    		  if(!strcmp(myDataString, "left")) AlgoidMessageRX.DCmotor[i].wheel = MOTOR_LEFT;
				    		  if(!strcmp(myDataString, "right")) AlgoidMessageRX.DCmotor[i].wheel = MOTOR_RIGHT;


					    	  AlgoidMessageRX.DCmotor[i].velocity= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_VELOCITY, &i);
					    	  AlgoidMessageRX.DCmotor[i].time= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_TIME, &i);
					    	  AlgoidMessageRX.DCmotor[i].cm= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_CM, &i);
					    	  AlgoidMessageRX.DCmotor[i].accel= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_ACCEL, &i);
					    	  AlgoidMessageRX.DCmotor[i].decel= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_DECEL, &i);

				    	  }

				    	  if(AlgoidMessageRX.msgParam == DINPUT){
						     AlgoidMessageRX.DINsens[i].id= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_DIN, &i);
				    		 jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_STATE, AlgoidMessageRX.DINsens[i].event_state, 15, &i );
				    		 jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_SAFETY_STOP, AlgoidMessageRX.DINsens[i].safetyStop_state, 15, &i );
				    		 AlgoidMessageRX.DINsens[i].safetyStop_value= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_SAFETY_VALUE, &i);
					    	 //printf("id: %d event: %s Safety: %s Value: %d\n", AlgoidMessageRX.DINsens[i].id, AlgoidMessageRX.DINsens[i].event_state,AlgoidMessageRX.DINsens[i].safetyStop_state  ,AlgoidMessageRX.DINsens[i].safetyStop_value);
				    	  }

				    	  if(AlgoidMessageRX.msgParam == DISTANCE){
				    		  jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_STATE, AlgoidMessageRX.DISTsens[i].event_state, 15, &i );
				    		  AlgoidMessageRX.DISTsens[i].id= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_SONAR, &i);
				    		  AlgoidMessageRX.DISTsens[i].angle= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_ANGLE, &i);
				    		  AlgoidMessageRX.DISTsens[i].event_low= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_LOWER, &i);
				    		  AlgoidMessageRX.DISTsens[i].event_high= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_HIGHER, &i);
							 jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_SAFETY_STOP, AlgoidMessageRX.DISTsens[i].safetyStop_state, 15, &i );
							 AlgoidMessageRX.DISTsens[i].safetyStop_value= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_SAFETY_VALUE, &i);
//				    		  printf("\n-SONAR: %d DIST_EVENT_HIGH: %d, DIST_EVENT_LOW: %d  DIST_EVENT_ENABLE: %s\n", AlgoidMessageRX.DISTsens[i].id,
//				    		  AlgoidMessageRX.DISTsens[i].event_high, AlgoidMessageRX.DISTsens[i].event_low, AlgoidMessageRX.DISTsens[i].event_state);
				    	  }

				    	  if(AlgoidMessageRX.msgParam == BATTERY){
				    		  jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_STATE, AlgoidMessageRX.BATTsens[i].event_state, 15, &i );
				    		  AlgoidMessageRX.BATTsens[i].id= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_BATT, &i);
				    		  AlgoidMessageRX.BATTsens[i].event_low= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_LOWER, &i);
				    		  AlgoidMessageRX.BATTsens[i].event_high= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_HIGHER, &i);
							 jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_SAFETY_STOP, AlgoidMessageRX.BATTsens[i].safetyStop_state, 15, &i );
							 AlgoidMessageRX.BATTsens[i].safetyStop_value= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_SAFETY_VALUE, &i);
				    	  }
/*
                                          // SERVO A RETRAVAILLER
				    	  if(AlgoidMessageRX.msgParam == pPWM){ 
				    		  jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_STATE, AlgoidMessageRX.PWMout[i].state, 15, &i );
				    		  int organId=jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_SERVO, &i);
				    		  switch(organId){
				    		  	  case 0 : AlgoidMessageRX.PWMout[i].id=PWM_0; break;
				    		  	  case 1 : AlgoidMessageRX.PWMout[i].id=PWM_1; break;
				    		  	  case 2 : AlgoidMessageRX.PWMout[i].id=PWM_2; break;
				    		  	  default : AlgoidMessageRX.PWMout[i].id=-1; break;
				    		  }
				    		  AlgoidMessageRX.PWMout[i].angle= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_ANGLE, &i);
                                            }
 */
                                          // LED
				    	  if(AlgoidMessageRX.msgParam == pLED){
                                              
                                                  AlgoidMessageRX.LEDarray[i].time=-1;
                                                  AlgoidMessageRX.LEDarray[i].powerPercent=-1;
                                                  strcpy(AlgoidMessageRX.LEDarray[i].state,"null");
                                                  AlgoidMessageRX.LEDarray[i].blinkCount=-1;
                                                  
				    		  jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_STATE, AlgoidMessageRX.LEDarray[i].state, 15, &i );
				    		  int organId=jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_LED, &i);
				    		  AlgoidMessageRX.LEDarray[i].id=organId;
				    		  AlgoidMessageRX.LEDarray[i].powerPercent= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_POWER, &i);                                                  
                                                  AlgoidMessageRX.LEDarray[i].time= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_TIME, &i);
                                                  AlgoidMessageRX.LEDarray[i].blinkCount= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_COUNT, &i);
				    	  }
                                          
                                          // PWM
				    	  if(AlgoidMessageRX.msgParam == pPWM){
				    		  jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_STATE, AlgoidMessageRX.PWMarray[i].state, 15, &i );
				    		  int organId=jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_PWM, &i);
				    		  AlgoidMessageRX.PWMarray[i].id=organId;
				    		  AlgoidMessageRX.PWMarray[i].powerPercent= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_POWER, &i);
				    	  }


				    	  if(AlgoidMessageRX.msgParam == STATUS){
				    		  // To do: Detect if wheel, sonar, batery, etc...
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
void ackToJSON(char * buffer, int msgId, char* to, char* from, char* msgType, char* msgParam, unsigned char valStr, unsigned char count ){
	unsigned int buflen= 1024;
	unsigned char i;

// Formatage de la r�ponse en JSON
	jwOpen( buffer, buflen, JW_OBJECT, JW_PRETTY );		// start root object
		jwObj_string( "MsgTo", to );				// add object key:value pairs
		jwObj_string( "MsgFrom", from );				// add object key:value pairs
		jwObj_int( "MsgID", msgId );
		jwObj_object( "MsgData" );
			jwObj_string( "MsgType", msgType );				// add object key:value pairs
			if(msgParam!=0) jwObj_string( "MsgParam", msgParam );				// add object key:value pairs

			if(count>0){

				jwObj_array( "MsgValue" );
				for(i=0;i<count;i++){
					//printf("Make array: %d values: %d %d\n", i, 0,9);
					jwArr_object();
						switch(valStr){
							case LL_2WD :
											if (AlgoidResponse[i].actionState == 0) jwObj_string("action", "end");
											if (AlgoidResponse[i].actionState == 1) jwObj_string("action", "begin");
											if (AlgoidResponse[i].actionState == 2) jwObj_string("action", "abort");

										/*
											if(AlgoidResponse[i].MOTresponse.id == 0) jwObj_string("wheel", "left");							// add object key:value pairs
											if(AlgoidResponse[i].MOTresponse.id == 1) jwObj_string("wheel", "right");							// add object key:value pairs
											jwObj_int( "cm", AlgoidResponse[i].MOTresponse.distance);				// add object key:value pairs
											jwObj_int( "time", AlgoidResponse[i].MOTresponse.time);				// add object key:value pairs
											jwObj_int("speed", round((AlgoidResponse[i].MOTresponse.speed)));		// add object key:value pairs
										*/
										   break;

							case DISTANCE :
											jwObj_int("sonar",AlgoidResponse[i].DISTresponse.id);				// add object key:value pairs
											if(AlgoidResponse[i].value >= 0){
												jwObj_int("cm", round((AlgoidResponse[i].value)));					// add object key:value pairs
												jwObj_int("angle", AlgoidResponse[i].DISTresponse.angle);				// add object key:value pairs
												jwObj_string("event", AlgoidResponse[i].DISTresponse.event_state);				// add object key:value pairs
												jwObj_int("event_lower", AlgoidResponse[i].DISTresponse.event_low);				// add object key:value pairs
												jwObj_int("event_higher", AlgoidResponse[i].DISTresponse.event_high);				// add object key:value pairs
												jwObj_string("safety_stop", AlgoidResponse[i].DISTresponse.safetyStop_state);				// add object key:value pairs
												jwObj_int("safety_value", AlgoidResponse[i].DISTresponse.safetyStop_value);				// add object key:value pairs
											} else
												jwObj_string("cm", "error");

											break;

							case BATTERY :
											jwObj_int( "battery",AlgoidResponse[i].BATTesponse.id);				// add object key:value pairs
											if(AlgoidResponse[i].value >= 0){
												jwObj_int("mV", AlgoidResponse[i].value);				// add object key:value pairs
												jwObj_string("event", AlgoidResponse[i].BATTesponse.event_state);				// add object key:value pairs
												jwObj_int("event_lower", AlgoidResponse[i].BATTesponse.event_low);				// add object key:value pairs
												jwObj_int("event_higher", AlgoidResponse[i].BATTesponse.event_high);				// add object key:value pairs
												jwObj_string("safety_stop", AlgoidResponse[i].BATTesponse.safetyStop_state);				// add object key:value pairs
												jwObj_int("safety_value", AlgoidResponse[i].BATTesponse.safetyStop_value);				// add object key:value pairs
											} else
												jwObj_string("mV", "error");
											break;

							case DINPUT :
											jwObj_int("din",AlgoidResponse[i].DINresponse.id);				// add object key:value pairs
											if(AlgoidResponse[i].value >= 0){
												jwObj_int( "state", AlgoidResponse[i].value);				// add object key:value pairs
												jwObj_string("event", AlgoidResponse[i].DINresponse.event_state);				// add object key:value pairs
												jwObj_string("safety_stop", AlgoidResponse[i].DINresponse.safetyStop_state);				// add object key:value pairs
												jwObj_int("safety_value", AlgoidResponse[i].DINresponse.safetyStop_value);				// add object key:value pairs
											} else
												jwObj_string("State", "error");
										   break;

							case STATUS :

												switch(i){
													case 0 :jwObj_int("din",AlgoidResponse[i].DINresponse.id);		// add object key:value pairs
															jwObj_int( "state", AlgoidResponse[i].value);			// add object key:value pairs
															break;

													case 1 :jwObj_int("din",AlgoidResponse[i].DINresponse.id);		// add object key:value pairs
															jwObj_int( "state", AlgoidResponse[i].value);			// add object key:value pairs
															break;

													case 2 :jwObj_int("battery",AlgoidResponse[i].BATTesponse.id);	// add object key:value pairs
															jwObj_int( "mV", AlgoidResponse[i].value);				// add object key:value pairs
															break;

													case 3 :jwObj_int("sonar",AlgoidResponse[i].DISTresponse.id);	// add object key:value pairs
															jwObj_int("cm", round((AlgoidResponse[i].value)));		// add object key:value pairs
															break;

													case 4 :jwObj_string("wheel", "left");	// add object key:value pairs
															jwObj_int("distance", round((AlgoidResponse[i].MOTresponse.distance)));		// add object key:value pairs
															jwObj_int("speed", round((AlgoidResponse[i].MOTresponse.speed)));		// add object key:value pairs
															break;

													case 5 :jwObj_string("wheel", "right");	// add object key:value pairs
															jwObj_int("distance", round((AlgoidResponse[i].MOTresponse.distance)));		// add object key:value pairs
															jwObj_int("speed", round((AlgoidResponse[i].MOTresponse.speed)));		// add object key:value pairs
															break;
													default : break;
												}

										   break;

							default:  	   break;

						}
					jwEnd();
				}
				jwEnd();
			}
		jwEnd();
		jwClose();
}
