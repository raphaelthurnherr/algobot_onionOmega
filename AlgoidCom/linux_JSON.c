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

#define KEY_MESSAGE_VALUE_SAFETY_STOP "{'MsgData'{'MsgValue'[*{'safety_stop'"
#define KEY_MESSAGE_VALUE_SAFETY_VALUE "{'MsgData'{'MsgValue'[*{'safety_value'"


#define KEY_MESSAGE_VALUE_MOTOR "{'MsgData'{'MsgValue'[*{'motor'"
#define KEY_MESSAGE_VALUE_VELOCITY "{'MsgData'{'MsgValue'[*{'velocity'"
#define KEY_MESSAGE_VALUE_TIME "{'MsgData'{'MsgValue'[*{'time'"
#define KEY_MESSAGE_VALUE_CM "{'MsgData'{'MsgValue'[*{'cm'"
#define KEY_MESSAGE_VALUE_ACCEL "{'MsgData'{'MsgValue'[*{'accel'"
#define KEY_MESSAGE_VALUE_DECEL "{'MsgData'{'MsgValue'[*{'decel'"

#define KEY_MESSAGE_VALUE_CFG_STREAM_STATE "{'MsgData'{'MsgValue'[*{'stream'{'state'"
#define KEY_MESSAGE_VALUE_CFG_STREAM_TIME "{'MsgData'{'MsgValue'[*{'stream'{'time'"
#define KEY_MESSAGE_VALUE_CFG_STREAM_ONEVENT "{'MsgData'{'MsgValue'[*{'stream'{'onEvent'"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../buggy_descriptor.h"
#include "linux_json.h"
#include "libs/lib_json/jRead.h"
#include "libs/lib_json/jWrite.h"

void ackToJSON(char * buffer, int msgId, char* to, char * from, char * msgType,char * msgParam, unsigned char orgType, unsigned char count);
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
				if(!strcmp(myDataString, "error")) AlgoidMessageRX.msgType = ERR_TYPE;

	// MESSAGE PARAM
				// Clear string
				for(i=0;i<20;i++) myDataString[i]=0;
				jRead_string((char *)srcBuffer,  KEY_MESSAGE_PARAM,myDataString,15, NULL);

				AlgoidMessageRX.msgParam=ERR_PARAM;
					if(!strcmp(myDataString, "stop")) AlgoidMessageRX.msgParam = STOP;
					if(!strcmp(myDataString, "move")) AlgoidMessageRX.msgParam = MOVE;
					if(!strcmp(myDataString, "motor")) AlgoidMessageRX.msgParam = MOTORS;
					if(!strcmp(myDataString, "pwm")) AlgoidMessageRX.msgParam = pPWM;
					if(!strcmp(myDataString, "led")) AlgoidMessageRX.msgParam = pLED;
                                        if(!strcmp(myDataString, "button")) AlgoidMessageRX.msgParam = BUTTON;
					if(!strcmp(myDataString, "distance")) AlgoidMessageRX.msgParam = DISTANCE;
					if(!strcmp(myDataString, "battery")) AlgoidMessageRX.msgParam = BATTERY;
					if(!strcmp(myDataString, "din")) AlgoidMessageRX.msgParam = DINPUT;
					if(!strcmp(myDataString, "status")) AlgoidMessageRX.msgParam = STATUS;
                                        if(!strcmp(myDataString, "rgb")) AlgoidMessageRX.msgParam = COLORS;
                                        if(!strcmp(myDataString, "config")) AlgoidMessageRX.msgParam = CONFIG;

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
					    	  AlgoidMessageRX.DCmotor[i].accel= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_ACCEL, &i);
					    	  AlgoidMessageRX.DCmotor[i].decel= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_DECEL, &i);
/*                                                  printf("\n --DEBUG: MOTOR ID: %d, SPEED: %d, TIME, %d, DIST: %d\n", 
                                                  AlgoidMessageRX.DCmotor[i].motor,
                                                          AlgoidMessageRX.DCmotor[i].velocity,
                                                          AlgoidMessageRX.DCmotor[i].time,
                                                          AlgoidMessageRX.DCmotor[i].cm);
 */ 
				    	  }

				    	  if(AlgoidMessageRX.msgParam == DINPUT){
						     AlgoidMessageRX.DINsens[i].id= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_DIN, &i);
				    		 jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_STATE, AlgoidMessageRX.DINsens[i].event_state, 15, &i );
				    		 jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_SAFETY_STOP, AlgoidMessageRX.DINsens[i].safetyStop_state, 15, &i );
				    		 AlgoidMessageRX.DINsens[i].safetyStop_value= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_SAFETY_VALUE, &i);
					    	 //printf("id: %d event: %s Safety: %s Value: %d\n", AlgoidMessageRX.DINsens[i].id, AlgoidMessageRX.DINsens[i].event_state,AlgoidMessageRX.DINsens[i].safetyStop_state  ,AlgoidMessageRX.DINsens[i].safetyStop_value);
				    	  }
                                          
                                          if(AlgoidMessageRX.msgParam == BUTTON){
						 AlgoidMessageRX.BTNsens[i].id= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_BTN, &i);
				    		 jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_EVENT_STATE, AlgoidMessageRX.BTNsens[i].event_state, 15, &i );
				    		 jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_SAFETY_STOP, AlgoidMessageRX.BTNsens[i].safetyStop_state, 15, &i );
				    		 AlgoidMessageRX.BTNsens[i].safetyStop_value= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_SAFETY_VALUE, &i);
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
                                                  printf("RED EVENT LOW: %d\n", AlgoidMessageRX.RGBsens[i].red.event_low);
                                                  printf("RED EVENT HIGH: %d\n", AlgoidMessageRX.RGBsens[i].red.event_high);
                                                  printf("GREEN EVENT LOW: %d\n", AlgoidMessageRX.RGBsens[i].green.event_low);
                                                  printf("GREEN EVENT LOW: %d\n", AlgoidMessageRX.RGBsens[i].green.event_high);
                                                  printf("BLUE EVENT LOW: %d\n", AlgoidMessageRX.RGBsens[i].blue.event_low);
                                                  printf("BLUE EVENT LOW: %d\n", AlgoidMessageRX.RGBsens[i].blue.event_high);
                                                  printf("CLEAR EVENT LOW: %d\n", AlgoidMessageRX.RGBsens[i].clear.event_low);
                                                  
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


				    	  if(AlgoidMessageRX.msgParam == STATUS){
                                                  // Nothing to get, return status of all system
				    	  }
                                          
                                          if(AlgoidMessageRX.msgParam == CONFIG){
                                                  jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_STREAM_STATE, AlgoidMessageRX.Config.stream.state, 15, &i );
                                                  AlgoidMessageRX.Config.stream.time= jRead_long((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_STREAM_TIME, &i);
                                                  jRead_string((char *)srcBuffer, KEY_MESSAGE_VALUE_CFG_STREAM_ONEVENT, AlgoidMessageRX.Config.stream.onEvent, 15, &i );
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
        
// Formatage de la r�ponse en JSON
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
                                                                                            case EVENT_ACTION_ABORT : jwObj_string("action", "abort"); break;
                                                                                            case RESP_STD_MESSAGE   :    if(AlgoidResponse[i].MOTresponse.motor>=0)
                                                                                                            jwObj_int( "motor", AlgoidResponse[i].MOTresponse.motor);
                                                                                                        else
                                                                                                            jwObj_string("motor", "unknown");
                                                                                                        jwObj_int( "cm", AlgoidResponse[i].MOTresponse.cm);				// add object key:value pairs
                                                                                                        jwObj_int( "time", AlgoidResponse[i].MOTresponse.time);				// add object key:value pairs
                                                                                                        jwObj_int("velocity", round((AlgoidResponse[i].MOTresponse.velocity)));
                                                                                                        ; break;
                                                                                            default : jwObj_string("error", "unknown"); break;
                                                                                        }		// add object key:value pairs
										
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
                                                                                        jwObj_string("safety_stop", AlgoidResponse[i].DISTresponse.safetyStop_state);				// add object key:value pairs
                                                                                        jwObj_int("safety_value", AlgoidResponse[i].DISTresponse.safetyStop_value);				// add object key:value pairs
                                                                                } else
                                                                                        jwObj_string("cm", "error");
                                                                                        
                                                                                break;

							case COLORS :                 
                                                                                jwObj_int( "rgb",AlgoidResponse[i].RGBresponse.id);

                                                                                jwObj_string("event", AlgoidResponse[i].RGBresponse.event_state);				// add object key:value pairs
                                                                                
//                                                                               // jwObj_array( "color" );
                                                                                    jwObj_object("color");
                                                                                        jwObj_int("red", AlgoidResponse[i].RGBresponse.red.value);
                                                                                        jwObj_int("green", AlgoidResponse[i].RGBresponse.green.value);
                                                                                        jwObj_int("blue", AlgoidResponse[i].RGBresponse.blue.value);
                                                                                        jwObj_int("clear", AlgoidResponse[i].RGBresponse.clear.value);
                                                                                    jwEnd();
                                                                                //jwEnd();
                                                                                
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
                                                                                                jwObj_int("mV", AlgoidResponse[i].value);				// add object key:value pairs
                                                                                                jwObj_string("event", AlgoidResponse[i].BATTesponse.event_state);				// add object key:value pairs
                                                                                                jwObj_int("event_lower", AlgoidResponse[i].BATTesponse.event_low);				// add object key:value pairs
                                                                                                jwObj_int("event_higher", AlgoidResponse[i].BATTesponse.event_high);				// add object key:value pairs
                                                                                                jwObj_string("safety_stop", AlgoidResponse[i].BATTesponse.safetyStop_state);				// add object key:value pairs
                                                                                                jwObj_int("safety_value", AlgoidResponse[i].BATTesponse.safetyStop_value);				// add object key:value pairs
                                                                                        } else{
                                                                                                jwObj_string("mV", "error");
                                                                                        }
                                                                                        
											break;

							case DINPUT :                  jwObj_int("din",AlgoidResponse[i].DINresponse.id);	
                                                                                       if(AlgoidResponse[i].value >= 0){
                                                                                        
                                                                                            			// add object key:value pairs
                                                                                            if(AlgoidResponse[i].value >= 0){
                                                                                                    jwObj_int( "state", AlgoidResponse[i].value);				// add object key:value pairs
                                                                                                    jwObj_string("event", AlgoidResponse[i].DINresponse.event_state);				// add object key:value pairs
                                                                                                    jwObj_string("safety_stop", AlgoidResponse[i].DINresponse.safetyStop_state);				// add object key:value pairs
                                                                                                    jwObj_int("safety_value", AlgoidResponse[i].DINresponse.safetyStop_value);				// add object key:value pairs
                                                                                            } else
                                                                                                    jwObj_string("state", "error");
                                                                                        }
                                                                                       else{
                                                                                            jwObj_string("state", "error");
                                                                                       }
										    break;
                                                                                    
							case BUTTON :                  jwObj_int("btn",AlgoidResponse[i].BTNresponse.id);	
                                                                                       if(AlgoidResponse[i].value >= 0){
                                                                                        
                                                                                            			// add object key:value pairs
                                                                                            if(AlgoidResponse[i].value >= 0){
                                                                                                    jwObj_int( "state", AlgoidResponse[i].value);				// add object key:value pairs
                                                                                                    jwObj_string("event", AlgoidResponse[i].BTNresponse.event_state);				// add object key:value pairs
                                                                                                    jwObj_string("safety_stop", AlgoidResponse[i].BTNresponse.safetyStop_state);				// add object key:value pairs
                                                                                                    jwObj_int("safety_value", AlgoidResponse[i].BTNresponse.safetyStop_value);				// add object key:value pairs
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
                                                                                        jwObj_string("name", AlgoidResponse[i].SYSresponse.name);
                                                                                        jwObj_int("upTime",AlgoidResponse[i].SYSresponse.startUpTime);
                                                                                        jwObj_string("firmwareVersion",AlgoidResponse[i].SYSresponse.firmwareVersion);	
                                                                                        jwObj_string("mcuVersion",AlgoidResponse[i].SYSresponse.mcuVersion);
                                                                                        jwObj_string("boardRev",AlgoidResponse[i].SYSresponse.HWrevision);
                                                                                        jwObj_double("battery_mv",AlgoidResponse[i].SYSresponse.battVoltage);		// add object key:value pairs
                                                                                    }

                                                                                    // ETAT DES DIN
                                                                                    if(i>=1 && i<1+NBDIN){
//                                                                                        jwObj_int("din",AlgoidResponse[i].DINresponse.id);		// add object key:value pairs
//                                                                                        jwObj_int( "state", AlgoidResponse[i].value);
                                                                                        jwObj_array( "din" );
                                                                                            
                                                                                            for(j=0;j<NBDIN;j++){
                                                                                                jwArr_object();
                                                                                                    jwObj_int("state",AlgoidResponse[i].value);
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
                                                                                                    jwObj_int("cm",round((AlgoidResponse[i].MOTresponse.cm)));
                                                                                                    jwObj_int("speed",round((AlgoidResponse[i].MOTresponse.velocity)));
                                                                                                jwEnd();           
                                                                                                i++;
                                                                                            }
                                                                                        jwEnd(); 
                                                                                    }
                                                                                    
                                                                                                                                                                        // ETAT DES MOTEUR                                                                                        // ETAT DES AIN                                                                                       // ETAT DES DIN
                                                                                    if(i>=1+NBDIN+NBBTN+NBMOTOR && i<1+NBDIN+NBBTN+NBMOTOR+NBSONAR){
                                                                                    //    jwObj_int("sonar",AlgoidResponse[i].DISTresponse.id);		// add object key:value pairs
                                                                                    //    jwObj_int("cm", round((AlgoidResponse[i].value)));
                                                                                        jwObj_array( "sonar" );
                                                                                            for(j=0;j<NBSONAR;j++){
                                                                                                jwArr_object();
                                                                                                    jwObj_int("cm", round((AlgoidResponse[i].value)));
                                                                                                jwEnd();           
                                                                                                i++;
                                                                                            }
                                                                                        jwEnd(); 
                                                                                    }
                                                                                    
                                                                                      
                                                                                    // ETAT DES PWM                                                                                   // ETAT DES PWM                                                                                        // ETAT DES AIN                                                                                       // ETAT DES DIN
                                                                                    if(i>=1+NBDIN+NBBTN+NBMOTOR+NBSONAR && i<1+NBDIN+NBBTN+NBMOTOR+NBSONAR+NBPWM){
                                                                                     //       jwObj_int("pwm",AlgoidResponse[i].PWMresponse.id);		// add object key:value pairs
                                                                                     //       jwObj_int( "state", AlgoidResponse[i].value);
                                                                                     //       jwObj_int( "power", AlgoidResponse[i].PWMresponse.powerPercent);
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
                                                                                    
                                                        case pPWM :             switch(AlgoidResponse[i].responseType){
                                                                                    case EVENT_ACTION_ERROR :   jwObj_string("action", "error");break;
                                                                                    case EVENT_ACTION_END  :   jwObj_string("action", "end"); break;
                                                                                    case EVENT_ACTION_BEGIN  :   jwObj_string("action", "begin"); break;
                                                                                    case EVENT_ACTION_ABORT  :   jwObj_string("action", "abort"); break;
                                                                                    case RESP_STD_MESSAGE  :   if(AlgoidResponse[i].PWMresponse.id>=0)
                                                                                                    jwObj_int( "pwm", AlgoidResponse[i].PWMresponse.id);
                                                                                                else
                                                                                                    jwObj_string("pwm", "unknown");
                                                                                                jwObj_string( "state", AlgoidResponse[i].PWMresponse.state);				// add object key:value pairs
                                                                                                jwObj_int( "power", AlgoidResponse[i].PWMresponse.powerPercent);				// add object key:value pairs
                                                                                                jwObj_int("time", AlgoidResponse[i].PWMresponse.time);
                                                                                                break;
                                                                                    default :   jwObj_string("error", "unknown");break;
                                                                                }
                                                                                break;
                                                                                   
                                                        case pLED :             switch(AlgoidResponse[i].responseType){
                                                                                    case EVENT_ACTION_ERROR :    jwObj_string("action", "error");break;
                                                                                    case EVENT_ACTION_END  :     jwObj_string("action", "end"); break;
                                                                                    case EVENT_ACTION_BEGIN  :   jwObj_string("action", "begin"); break;
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
