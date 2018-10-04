/*
 * messagesManager.h
 *
 *  Created on: 15 mars 2016
 *      Author: raph
 */

#ifndef MESSAGESMANAGER_H_
#define MESSAGESMANAGER_H_

extern char ClientID[50];
extern char ADDRESS[25];

// D�finition des topic de r�ponses MQTT
#define TOPIC_COMMAND "Command"					// Topic d'�coute de l'hote
#define TOPIC_RESPONSE "Response"					// Topic de r�ponse � l'hote
#define TOPIC_EVENT "Event"						// Topic d'envoie des evenements � l'hote
#define TOPIC_ACK "Ack"					// Topic de r�ponse ack du gestionnaire de message
#define TOPIC_DEBUG "Debug"
#define TOPIC_DATAFLOW "Stream"

// Initialisation de la messagerie system (JSON<->MQTT)
int InitMessager(void);
int pullMsgStack(unsigned char ptrStack);
int CloseMessager(void);
void sendMqttReport(int msgId, char * msg);
void sendResponse(int msgId, char * msgTo, unsigned char msgType, unsigned char msgParam, unsigned char valCnt);

typedef struct msg_statistics{
        int messageTX;
        int messageRX;
}STATISTICS;

extern STATISTICS msg_stats;
#endif /* MESSAGESMANAGER_H_ */
