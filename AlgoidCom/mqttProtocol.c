/*
 * mqttProtocol.c
 *
 *  Created on: 15 mars 2016
 *      Author: raph
 */

// Connexion au brocker local
#define QOS         0
#define TIMEOUT     10000L

#include "libs/lib_mqtt/MQTTClient.h"
#include "mqttProtocol.h"


MQTTClient_deliveryToken deliveredtoken, token;
MQTTClient client;

void delivered(void *context, MQTTClient_deliveryToken dt);
void connlost(void *context, char *cause);

int mqtt_init(const char *IPaddress, const char *clientID, MQTTClient_messageArrived* msgarr);
char mqttAddRXChannel(char * topicName);
char mqttRemoveRXChannel(char * topicName);
// Envoie le messageau brocker MQTT
int mqttPutMessage(char *topic, char *data, unsigned short lenght);
// -------------------------------------------------------------------
// INITIALISATION DE LA CONNEXION AU BROCKER MQTT
// -------------------------------------------------------------------
int mqtt_init(const char *IPaddress, const char *clientID, MQTTClient_messageArrived* msgarrFunc){
		int rc;

		MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
		//MQTTClient_message pubmsg = MQTTClient_message_initializer;

		// Configuration des paramï¿½tres de connexion
		MQTTClient_create(&client, IPaddress, clientID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

		conn_opts.keepAliveInterval = 20;
		conn_opts.cleansession = 1;
		// Fin de config connexion

		// Configuration de la fonction callback de souscription
		MQTTClient_setCallbacks(client, NULL, connlost, msgarrFunc, delivered);

		// Tentative de connexion au broker mqtt
		if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
		{
			return(rc);
		}else return 0;
	}

// -------------------------------------------------------------------
// Défini le topic sur lequel doit se connecter Eduspider pour la Reception
// -------------------------------------------------------------------
char mqttAddRXChannel(char * topicName){
	// SOUSCRIPTION AU TOPIC MANAGER
	// Configuration souscription
	if(!MQTTClient_subscribe(client, topicName, QOS)) return (0);
	else return 1;

}

// -------------------------------------------------------------------
// Desabonnement a un canal algoid
// -------------------------------------------------------------------

char mqttRemoveRXChannel(char * topicName){
	// Configuration souscription
	if(!MQTTClient_unsubscribe(client, topicName))return(0);
	else return 1;
}


// -------------------------------------------------------------------
// Fonction Call-back de retour de token MQTT pour contrï¿½le
// -------------------------------------------------------------------

void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

// -------------------------------------------------------------------
// Fonction call-back perte de connexion avec le brocker
// -------------------------------------------------------------------
void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("              cause: %s\n", cause);
}

int mqttPutMessage(char *topic, char *data, unsigned short lenght){
	int rc;

	MQTTClient_publish(client, topic, lenght, data, QOS, 0, &token);
	rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);

	return (rc);
}
