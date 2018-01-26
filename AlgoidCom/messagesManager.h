/*
 * messagesManager.h
 *
 *  Created on: 15 mars 2016
 *      Author: raph
 */

#ifndef MESSAGESMANAGER_H_
#define MESSAGESMANAGER_H_

extern char ClientID[50];

// Définition des topic de réponses MQTT
#define TOPIC_COMMAND "Command"					// Topic d'écoute de l'hote
#define TOPIC_RESPONSE "Response"					// Topic de réponse à l'hote
#define TOPIC_EVENT "Event"						// Topic d'envoie des evenements à l'hote
#define TOPIC_ACK "Ack"					// Topic de réponse ack du gestionnaire de message

// Initialisation de la messagerie system (JSON<->MQTT)
int InitMessager(void);
int pullMsgStack(unsigned char ptrStack);
int CloseMessager(void);
void sendMqttReport(int msgId, char * msg);
void sendResponse(int msgId, char * msgTo, unsigned char msgType, unsigned char msgParam, unsigned char valCnt);
#endif /* MESSAGESMANAGER_H_ */
