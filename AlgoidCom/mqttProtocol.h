/*
 * mqttProtocol.h
 *
 *  Created on: 15 mars 2016
 *      Author: raph
 */

#ifndef MQTTPROTOCOL_H_
#define MQTTPROTOCOL_H_

#include "libs/lib_mqtt/MQTTClient.h"

// INITIALISATION DE LA CONNEXION AU BROCKER MQTT
//int mqtt_init(const char *IPaddress, const char *clientID, MQTTClient_messageArrived* msgarrFunc);
int mqtt_init(const char *IPaddress, const char *clientID, MQTTClient_messageArrived* msgarrFunc, MQTTClient_connectionLost* connLost_callback);
char mqttAddRXChannel(char * topicName);
char mqttRemoveRXChannel(char * topicName);
// Envoie le messageau brocker MQTT
int mqttPutMessage(char *topic, char *data, unsigned short lenght);

#endif /* MQTTPROTOCOL_H_ */
