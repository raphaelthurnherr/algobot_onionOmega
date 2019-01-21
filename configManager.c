/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#define MAX_MQTT_BUFF 4096

#define FILE_KEY_CONFIG_STREAM_STATE "{'stream'{'state'"
#define FILE_KEY_CONFIG_STREAM_TIME "{'stream'{'time'"
#define FILE_KEY_CONFIG_STREAM_ONEVENT "{'stream'{'onEvent'"

#define FILE_KEY_CONFIG_LED "{'led'"
#define FILE_KEY_CONFIG_LED_ID "{'led'[*{'led'"
#define FILE_KEY_CONFIG_LED_POWER "{'led'[*{'power'"
#define FILE_KEY_CONFIG_LED_STATE "{'led'[*{'state'"

#define FILE_KEY_CONFIG_MOTOR "{'motor'"
#define FILE_KEY_CONFIG_MOTOR_ID "{'motor'[*{'motor'"
#define FILE_KEY_CONFIG_MOTOR_INVERT "{'motor'[*{'inverted'"
#define FILE_KEY_CONFIG_MOTOR_MINPWM "{'motor'[*{'pwmMin'"
#define FILE_KEY_CONFIG_MOTOR_MINRPM "{'motor'[*{'rpmMin'"
#define FILE_KEY_CONFIG_MOTOR_MAXRPM "{'motor'[*{'rpmMax'"
#define FILE_KEY_CONFIG_MOTOR_PIDEN  "{'motor'[*{'rpmRegulator'{'state'"
#define FILE_KEY_CONFIG_MOTOR_PIDkp  "{'motor'[*{'rpmRegulator'{'PID_Kp'"
#define FILE_KEY_CONFIG_MOTOR_PIDki  "{'motor'[*{'rpmRegulator'{'PID_Ki'"
#define FILE_KEY_CONFIG_MOTOR_PIDkd  "{'motor'[*{'rpmRegulator'{'PID_Kd'"

#define FILE_KEY_CONFIG_WHEEL "{'wheel'"
#define FILE_KEY_CONFIG_WHEEL_ID "{'wheel'[*{'wheel'"
#define FILE_KEY_CONFIG_WHEEL_PULSES "{'wheel'[*{'pulses'"
#define FILE_KEY_CONFIG_WHEEL_DIAMETER "{'wheel'[*{'diameter'"

#define FILE_KEY_CONFIG_STEPPER "{'stepper'"
#define FILE_KEY_CONFIG_STEPPER_ID "{'stepper'[*{'motor'"
#define FILE_KEY_CONFIG_STEPPER_INVERT "{'stepper'[*{'inverted'"
#define FILE_KEY_CONFIG_STEPPER_RATIO "{'stepper'[*{'ratio'"
#define FILE_KEY_CONFIG_STEPPER_STEPS "{'stepper'[*{'steps'"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "type.h"
#include "libs/lib_json/jRead.h"
#include "libs/lib_json/jWrite.h"

char * OpenConfigFromFile(char *filename);
char LoadConfig(t_sysConf * Config, char * fileName);
char SaveConfig(t_sysConf* Config, char * fileName);

unsigned char mode=0;
unsigned char dataCommandReady=0;

//------------------------------------------------------------------
// EXTRACTION DES DONNEES DU FICHIER
//------------------------------------------------------------------

char * OpenConfigFromFile(char *filename){   
    FILE *myFile = fopen(filename, "rw+");
   static char *srcDataContent = NULL;
   
   int string_size, read_size;

   if (myFile)
   {
       // Seek the last byte of the file
       fseek(myFile, 0, SEEK_END);
       // Offset from the first to the last byte, or in other words, filesize
       string_size = ftell(myFile);
       // go back to the start of the file
       rewind(myFile);

       // Allocate a string that can hold it all
       srcDataContent = (char*) malloc(sizeof(char) * (string_size + 1) );

       // Read it all in one operation
       read_size = fread(srcDataContent, sizeof(char), string_size, myFile);

       // fread doesn't set it so put a \0 in the last position
       // and buffer is now officially a string
       srcDataContent[string_size] = '\0';

       if (string_size != read_size)
       {
           // Something went wrong, throw away the memory and set
           // the buffer to NULL
           free(srcDataContent);
           srcDataContent = NULL;
       }

       // Always remember to close the file.
       fclose(myFile);
    }
      
   return(srcDataContent);
}

// -----------------------------------------------------------------------------
// LoadConfig
// Get configuration file and load data into config structure
// -----------------------------------------------------------------------------

char LoadConfig(t_sysConf * Config, char * fileName){
	struct jReadElement cfg_devices_list;
        int nbOfDeviceInConf, deviceId;
	int i;
        char * srcDataBuffer;
        char dataValue[15];
        
        srcDataBuffer = OpenConfigFromFile(fileName); 
        
        
        if(srcDataBuffer != NULL){
    // EXTRACT STREAM SETTINGS FROM CONFIG
            // Load data for stream TIME
            Config->communication.mqtt.stream.time_ms= jRead_int((char *) srcDataBuffer, FILE_KEY_CONFIG_STREAM_TIME, &i);

            // Load data for stream STATE
            jRead_string((char *)srcDataBuffer, FILE_KEY_CONFIG_STREAM_STATE, dataValue, 15, &i );

            if(!strcmp(dataValue, "on")){
                Config->communication.mqtt.stream.state = 1;
                
            }else
                if(!strcmp(dataValue, "off")){
                    Config->communication.mqtt.stream.state = 0;
                }

            // Load data for stream ONEVENT
            jRead_string((char *)srcDataBuffer, FILE_KEY_CONFIG_STREAM_ONEVENT, dataValue, 15, &i );
            if(!strcmp(dataValue, "on")){
                Config->communication.mqtt.stream.onEvent = 1;
            }else
                if(!strcmp(dataValue, "off")){
                    Config->communication.mqtt.stream.onEvent = 0;
                }

    // EXTRACT MOTOR SETTINGS FROM CONFIG    
            // Reset motor data config before reading
            for(i=0;i<NBMOTOR;i++){
              Config->device.motor[i].inverted = -1;
              Config->parts.dcwheel[i]->rpmMin = 20;
              Config->parts.dcwheel[i]->rpmMax = 200;
            }

        // Motor Setting
            jRead((char *)srcDataBuffer, FILE_KEY_CONFIG_MOTOR, &cfg_devices_list );

            // RECHERCHE DATA DE TYPE ARRAY
            if(cfg_devices_list.dataType == JREAD_ARRAY ){
                // Get the number of motors in array
                nbOfDeviceInConf=cfg_devices_list.elements;

                for(i=0; i < nbOfDeviceInConf; i++){ 
                    deviceId=-1;
                    deviceId=jRead_int((char *)srcDataBuffer, FILE_KEY_CONFIG_MOTOR_ID, &i); 

                    if(deviceId >= 0 && deviceId < NBMOTOR){
                        Config->parts.dcwheel[deviceId]->rpmMin = jRead_int((char *)srcDataBuffer, FILE_KEY_CONFIG_MOTOR_MINRPM, &i); 
                        Config->parts.dcwheel[deviceId]->rpmMax = jRead_int((char *)srcDataBuffer, FILE_KEY_CONFIG_MOTOR_MAXRPM, &i); 
                        Config->device.motor[deviceId].powerMin = jRead_int((char *)srcDataBuffer, FILE_KEY_CONFIG_MOTOR_MINPWM, &i); 
                        jRead_string((char *)srcDataBuffer, FILE_KEY_CONFIG_MOTOR_INVERT, dataValue, 15, &i );
                        if(!strcmp(dataValue, "on")){
                            Config->device.motor[deviceId].inverted = 1;
                        }else
                            if(!strcmp(dataValue, "off")){
                                Config->device.motor[deviceId].inverted = 0;
                            }
                        // RECUPERATION DES PARAMETRE DU REGULATOR PID
                        jRead_string((char *)srcDataBuffer, FILE_KEY_CONFIG_MOTOR_PIDEN, dataValue, 15, &i );
                        if(!strcmp(dataValue, "on")){
                            Config->parts.dcwheel[deviceId]->pidReg.enable = 1;
                        }else
                            if(!strcmp(dataValue, "off")){
                                Config->parts.dcwheel[deviceId]->pidReg.enable = 0;
                            }                        
                        Config->parts.dcwheel[deviceId]->pidReg.Kp = jRead_double((char *)srcDataBuffer, FILE_KEY_CONFIG_MOTOR_PIDkp, &i); 
                        Config->parts.dcwheel[deviceId]->pidReg.Ki = jRead_double((char *)srcDataBuffer, FILE_KEY_CONFIG_MOTOR_PIDki, &i); 
                        Config->parts.dcwheel[deviceId]->pidReg.Kd = jRead_double((char *)srcDataBuffer, FILE_KEY_CONFIG_MOTOR_PIDkd, &i); 
                        //printf("\n****CFG MOTOR #%d: %d\n",deviceId, Config->motor[deviceId].minPower);
                    }
                }
            }
            
    // EXTRACT WHEEL SETTINGS FROM CONFIG    
            // Reset motor data config before reading
            for(i=0;i<NBMOTOR;i++){
              Config->parts.dcwheel[deviceId]->diameter=-1;
              Config->parts.dcwheel[deviceId]->pulsesPerRot=-1;
            }

        // Wheel Setting
            jRead((char *)srcDataBuffer, FILE_KEY_CONFIG_WHEEL, &cfg_devices_list );

            // RECHERCHE DATA DE TYPE ARRAY
            if(cfg_devices_list.dataType == JREAD_ARRAY ){
                // Get the number of motors in array
                nbOfDeviceInConf=cfg_devices_list.elements;

                for(i=0; i < nbOfDeviceInConf; i++){ 
                    deviceId=-1;
                    deviceId=jRead_long((char *)srcDataBuffer, FILE_KEY_CONFIG_WHEEL_ID, &i); 

                    if(deviceId >= 0 && deviceId < NBMOTOR){
                        Config->parts.dcwheel[deviceId]->diameter = jRead_long((char *)srcDataBuffer, FILE_KEY_CONFIG_WHEEL_DIAMETER, &i);
                        Config->parts.dcwheel[deviceId]->pulsesPerRot = jRead_long((char *)srcDataBuffer, FILE_KEY_CONFIG_WHEEL_PULSES, &i);
//                        Config->wheel[deviceId]._MMPP = (Config->wheel[deviceId].diameter * 3.1415926535897932384) / Config->wheel[deviceId].pulsePerRot;
//                        Config->wheel[deviceId]._MAXSPEED_CMSEC = ((float)Config->motor[deviceId].maxRPM /60) *  ((Config->wheel[deviceId].diameter * 3.1415926535897932384)/10);
                    }
                }
            }
            
    // EXTRACT STEPPER MOTOR SETTINGS FROM CONFIG
        
            // Reset motor data config before reading
        for(i=0;i<NBSTEPPER;i++){
          Config->device.stepper[i].inverted = -1;
          Config->device.stepper[i].ratio = -1;
          Config->device.stepper[i].steps = -1;
        }
            
        // Stepper motor Settings
            jRead((char *)srcDataBuffer, FILE_KEY_CONFIG_STEPPER, &cfg_devices_list );

            // RECHERCHE DATA DE TYPE ARRAY
            if(cfg_devices_list.dataType == JREAD_ARRAY ){
                // Get the number of motors in array
                nbOfDeviceInConf=cfg_devices_list.elements;

                for(i=0; i < nbOfDeviceInConf; i++){ 
                    deviceId=-1;
                    deviceId=jRead_long((char *)srcDataBuffer, FILE_KEY_CONFIG_STEPPER_ID, &i); 

                    if(deviceId >= 0 && deviceId < NBSTEPPER){
                        jRead_string((char *)srcDataBuffer, FILE_KEY_CONFIG_STEPPER_INVERT, dataValue, 15, &i );
                        if(!strcmp(dataValue, "on")){
                            Config->device.stepper[deviceId].inverted = 1;
                        }else
                            if(!strcmp(dataValue, "off")){
                                Config->device.stepper[deviceId].inverted = 0;
                            }
                        Config->device.stepper[deviceId].ratio = jRead_long((char *)srcDataBuffer, FILE_KEY_CONFIG_STEPPER_RATIO, &i); 
                        Config->device.stepper[deviceId].steps = jRead_long((char *)srcDataBuffer, FILE_KEY_CONFIG_STEPPER_STEPS, &i); 
                    }
                }
            }            

    // EXTRACT LED SETTINGS FROM CONFIG    
          // Reset motor data config before reading
          for(i=0;i<NBLED;i++){
            Config->led[i].power=-1;            
            Config->led[i].state=-1;
            Config->led[i].isServoMode=-1;
          }

        // Les Setting
            jRead((char *)srcDataBuffer, FILE_KEY_CONFIG_LED, &cfg_devices_list );

            // RECHERCHE DATA DE TYPE ARRAY
            if(cfg_devices_list.dataType == JREAD_ARRAY ){
                // Get the number of motors in array
                nbOfDeviceInConf=cfg_devices_list.elements;

                for(i=0; i < nbOfDeviceInConf; i++){ 
                    deviceId=-1;
                    deviceId=jRead_long((char *)srcDataBuffer, FILE_KEY_CONFIG_LED_ID, &i); 

                   Config->led[deviceId].power = jRead_int((char *)srcDataBuffer, FILE_KEY_CONFIG_LED_POWER, &i);                     

                    if(deviceId >= 0 && deviceId < NBLED){
                        jRead_string((char *)srcDataBuffer, FILE_KEY_CONFIG_LED_STATE, dataValue, 15, &i );
                        if(!strcmp(dataValue, "on")){
                            Config->led[deviceId].state = 1;
                        }else
                            if(!strcmp(dataValue, "off")){
                                Config->led[deviceId].state = 0;
                            }
                    }
                }
            }            

            // Reset settings
    //            jRead_string((char *)srcDataBuffer, KEY_MESSAGE_VALUE_CFG_APPRESET, AlgoidMessageRX.Config.config.reset, 15, &i );
            return 0;
        }
        
        return -1;
}


// -----------------------------------------------------------------------------
// SaveConfig
// Save configuration to file
// -----------------------------------------------------------------------------

char SaveConfig(t_sysConf * Config, char * fileName){
    char buffer[MAX_MQTT_BUFF];
    unsigned int buflen= MAX_MQTT_BUFF;
    int i;
    // CREATE JSON STRING FOR CONFIGURATION
	jwOpen( buffer, buflen, JW_OBJECT, JW_PRETTY );		// start root object  
        // CREATE JSON CONFIG FOR STREAM        
            jwObj_object( "stream" );
            
            if(Config->communication.mqtt.stream.state == 0)
                jwObj_string("state", "off");
            else 
                if(Config->communication.mqtt.stream.state == 1)
                    jwObj_string("state", "on");

                jwObj_int( "time", Config->communication.mqtt.stream.time_ms);
                
            if(Config->communication.mqtt.stream.onEvent == 0)
                jwObj_string("onEvent", "off");
            else 
                if(Config->communication.mqtt.stream.onEvent == 1)
                    jwObj_string("onEvent", "on");                
            jwEnd();

        // CREATE JSON CONFIG FOR MOTOR            
            jwObj_array("motor");
                for(i=0;i<NBMOTOR;i++){
                    jwArr_object();
                        jwObj_int( "motor", i);
                        if(Config->device.motor[i].inverted == 0)
                            jwObj_string("inverted", "off");
                        else 
                            if(Config->device.motor[i].inverted == 1)
                                jwObj_string("inverted", "on");
                        jwObj_int( "pwmMin", Config->device.motor[i].powerMin);
                        jwObj_int( "rpmMin", Config->parts.dcwheel[i]->rpmMin);
                        jwObj_int( "rpmMax", Config->parts.dcwheel[i]->rpmMax);
                        jwObj_object("rpmRegulator");
                            if(Config->parts.dcwheel[i]->pidReg.enable == 0)
                                jwObj_string("state", "off");
                            else 
                                if(Config->parts.dcwheel[i]->pidReg.enable == 1)
                                    jwObj_string("state", "on");
                                    jwObj_double( "PID_Kp", Config->parts.dcwheel[i]->pidReg.Kp);
                                    jwObj_double( "PID_Ki", Config->parts.dcwheel[i]->pidReg.Ki);
                                    jwObj_double( "PID_Kd", Config->parts.dcwheel[i]->pidReg.Kd);
                        jwEnd();
                    jwEnd();
                } 
            jwEnd();
            
        // CREATE JSON CONFIG FOR WHEEL SETTING            
            jwObj_array("wheel");
                for(i=0;i<NBMOTOR;i++){
                    jwArr_object();
                        jwObj_int( "wheel", i);
                        jwObj_int( "diameter", Config->parts.dcwheel[i]->diameter);
                        jwObj_int( "pulses", Config->parts.dcwheel[i]->pulsesPerRot);
                    jwEnd();
                } 
            jwEnd();            
            
        // CREATE JSON CONFIG FOR STEPPER MOTOR            
            jwObj_array("stepper");
                for(i=0;i<NBSTEPPER;i++){
                    jwArr_object();
                        jwObj_int( "motor", i);
                        if(Config->device.stepper[i].inverted == 0)
                            jwObj_string("inverted", "off");
                        else 
                            if(Config->device.stepper[i].inverted == 1)
                                jwObj_string("inverted", "on");
                        jwObj_int( "ratio", Config->device.stepper[i].ratio);
                        jwObj_int( "steps", Config->device.stepper[i].steps);
                    jwEnd();
                } 
            jwEnd();            
            
        // CREATE JSON CONFIG FOR LED
            jwObj_array("led");
                for(i=0;i<NBLED;i++){
                    jwArr_object();
                        jwObj_int( "led", i);
                        if(Config->led[i].state == 0)
                            jwObj_string("state", "off");
                        else 
                            if(Config->led[i].state == 1)
                                jwObj_string("state", "on");
                        jwObj_int( "power", Config->led[i].power);
                    jwEnd();
                } 
            jwEnd();            
        jwClose();
       
    // CREATE JSON STRING FOR CONFIGURATION   
        FILE *fp;
        int x = 10;

        fp=fopen(fileName, "w");
        if(fp == NULL)
            printf("Error during config file open");
        
        fprintf(fp, buffer);
        fclose(fp);
    return 0;
}