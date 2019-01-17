
//#define ADDRESS     "192.168.3.1:1883"
//#define ADDRESS     "localhost:1883"

#define PORT     ":1883"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "pthread.h"
#include "messagesManager.h"
#include "mqttProtocol.h"
#include "linux_json.h"
#include "udpPublish.h"
#include "../buggy_descriptor.h"

// Thread Messager
pthread_t th_messager;

char ADDRESS[25] = "localhost";

char BroadcastID[50]="algo_";
char ClientID[50]="algo_";

void sendMqttReport(int msgId, char * msg);

int  mqttMsgArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message);
void mqttConnectionLost(void *context, char *cause);

void sendResponse(int msgId, char * msgTo, unsigned char msgType, unsigned char msgParam, unsigned char valCnt);
int pushMsgStack(void);
int pullMsgStack(unsigned char ptrStack);
char clearMsgStack(unsigned char ptrStack);

// Initialisation des variables
unsigned char mqttDataReady=0;
int mqttStatus;

char MqttDataBuffer[2048];
char msgReportBuffer[2048];

STATISTICS msg_stats;



// Initialisation principale du system de messagerie
void *MessagerTask (void * arg){
	int lastMessage;
	int i;


	// Initialisation de la pile de reception de message
	for(i=0;i<10;i++)
		clearMsgStack(i);

	// Creation d'un id unique avec l'adresse mac si non defini au demarrage
	if(!strcmp(ClientID, "algo_"))
            sprintf(&ClientID[5], "%s", getMACaddr());
        
        // Creation de l'adresse de connexion au brocker
        sprintf(&ADDRESS[strlen(ADDRESS)], "%s", PORT);
        
	// Connexion au broker MQTT
	mqttStatus=mqtt_init(ADDRESS, ClientID, mqttMsgArrived, mqttConnectionLost);

	if(mqttStatus==0){
		printf("#[MSG MANAGER] Connection au broker MQTT (%s): OK -> ID: \"%s\"\n", ADDRESS, ClientID   );
		if(!mqttAddRXChannel(TOPIC_COMMAND)){
			printf("#[MSG MANAGER] Inscription au topic: OK\n");
                        sendMqttReport(-1, "IS NOW ONLINE");
		}
		else {
			printf("#[MSG MANAGER] Inscription au topic: ERREUR\n");
		}
	}else {
		printf("#[MSG MANAGER] Connexion au broker MQTT: ERREUR [%d]\n", mqttStatus);
	}
        
// BOUCLE PRINCIPALE
	while(1)
	{
            // Vérification de la connexion au brocker
            if(mqttStatus<0){
                // Connexion au broker MQTT
                mqttStatus=mqtt_init(ADDRESS, ClientID, mqttMsgArrived, mqttConnectionLost);

                if(mqttStatus==0){
                        printf("#[MSG MANAGER] Connection au broker MQTT (%s): OK -> ID: \"%s\"\n", ADDRESS, ClientID   );
                        if(!mqttAddRXChannel(TOPIC_COMMAND)){
                                printf("#[MSG MANAGER] Inscription au topic: OK\n");
                                sendMqttReport(-1, "IS NOW ONLINE");
                        }
                        else {
                                printf("#[MSG MANAGER] Inscription au topic: ERREUR\n");
                        }
                }else {
                        printf("#[MSG MANAGER] Connexion au broker MQTT: ERREUR [%d]\n", mqttStatus);
                }
                sleep(5);
            }
            
	    // Verification de l'arriv�e d'un message MQTT
	    if(mqttDataReady){
	    // RECEPTION DES DONNES UTILES
                if(GetAlgoidMsg(AlgoidMessageRX, MqttDataBuffer)>0){
                        // Contr�le du destinataire
//                        if(!strncmp(AlgoidMessageRX.msgTo, ClientID) || !strcmp(AlgoidMessageRX.msgTo, BroadcastID)){
                        // Accept messages if destination differe of "algo_"  (is brodcast) or if client ID is Exactly the same
                        if(strncmp(AlgoidMessageRX.msgTo, ClientID, 5) || !strcmp(AlgoidMessageRX.msgTo, ClientID)){
                            // Enregistrement du message dans la pile
                                lastMessage=pushMsgStack();
                                if(lastMessage>=0){
                                        // Mise a jour du compteur de message syst�me
                                        msg_stats.messageRX++;
                                        
                                        // Retourne un ack a l'expediteur
                                        sendResponse(AlgoidMessageRX.msgID, AlgoidMessageRX.msgFrom, ACK, AlgoidMessageRX.msgParam, 0);
                                        sprintf(msgReportBuffer, "%s", ClientID);
                                        sendMqttReport(-1, "New message received");
                                }
                                else{
                                        printf("ERROR: Message stack full !\n");
                                        sendMqttReport(-1, "ERROR: Message stack full !");
                                        
                                }
                        }
                        else{
                                sprintf(msgReportBuffer, "IGNORE MESSAGE: destination name is [%s]\n", AlgoidMessageRX.msgTo);
                                printf(msgReportBuffer);
                                sendMqttReport(-1, msgReportBuffer);
                        }
                        
                }else{
                        // Retourne une erreur a l'expediteur
                        sendResponse(AlgoidMessageRX.msgID, AlgoidMessageRX.msgFrom, AlgoidMessageRX.msgType, AlgoidMessageRX.msgParam, 0);
                        printf("\nERROR: Incorrect message format\n");
                        sprintf(msgReportBuffer, "%s", ClientID);
                        sprintf(&msgReportBuffer[8], " -> %s", "ERROR: Incorrect message format");
                        sendMqttReport(AlgoidMessageRX.msgID, msgReportBuffer);
                        
                }
                mqttDataReady=0;
            }
            
            usleep(10000);
        }
 // FIN BOUCLE PRINCIPAL

  usleep(5000);
  pthread_exit (0);
}



int pushMsgStack(void){
	int ptrMsgRXstack=0;

	// Recherche un emplacement vide dans la pile
	for(ptrMsgRXstack=0;(ptrMsgRXstack<10) && (AlgoidMsgRXStack[ptrMsgRXstack].msgID>=0);ptrMsgRXstack++);

	// CONTROLE DE L'ETAT DE LA PILE DE MESSAGE
	if(ptrMsgRXstack>=10){
		return -1;
	}else{
		// ENREGISTREMENT DU MESSAGE DANS LA PILE
		AlgoidMsgRXStack[ptrMsgRXstack]=AlgoidMessageRX;

		ptrMsgRXstack++;
		return ptrMsgRXstack-1;
	}
}

int pullMsgStack(unsigned char ptrStack){
		int i;
		unsigned char result;

		if(AlgoidMsgRXStack[ptrStack].msgType!=-1){
			AlgoidCommand=AlgoidMsgRXStack[ptrStack];

			// Contr�le le ID, FROM, TO du message et creation g�n�rique si inexistant
			if(AlgoidCommand.msgID <= 0){
				AlgoidCommand.msgID = rand() & 0xFFFFFF;
			}


			if(!strcmp(AlgoidCommand.msgFrom, "")){
				strcpy(AlgoidCommand.msgFrom,"unknown");
			}

			// D�place les elements de la pile
			for(i=ptrStack;i<9;i++){
				AlgoidMsgRXStack[ptrStack]=AlgoidMsgRXStack[ptrStack+1];
				ptrStack++;
			}

			// EFFACE LES DONNEES DE LA PILE
			strcpy(AlgoidMsgRXStack[9].msgFrom, "");
			strcpy(AlgoidMsgRXStack[9].msgTo, "");
			AlgoidMsgRXStack[9].msgID=-1;
			AlgoidMsgRXStack[9].msgParam=-1;
			AlgoidMsgRXStack[9].msgType=-1;
			AlgoidMsgRXStack[9].msgValueCnt=0;

			for(i=0;i<AlgoidMsgRXStack[9].msgValueCnt;i++){
				AlgoidMsgRXStack[9].DCmotor[i].motor=UNKNOWN;
			}

			return 1;
		}else
			return 0;
}
// ----------------------

char clearMsgStack(unsigned char ptrStack){
		int i;

		if(ptrStack<10){
			// EFFACE LES DONNEES DE LA PILE
			strcpy(AlgoidMsgRXStack[ptrStack].msgFrom, "");
			strcpy(AlgoidMsgRXStack[ptrStack].msgTo, "");
			AlgoidMsgRXStack[ptrStack].msgID=-1;
			AlgoidMsgRXStack[ptrStack].msgParam=-1;
			AlgoidMsgRXStack[ptrStack].msgType=-1;

			for(i=0;i<AlgoidMsgRXStack[ptrStack].msgValueCnt;i++){
				//strcpy(AlgoidMsgRXStack[ptrStack].DCmotor[i].motor, "");
                                AlgoidMsgRXStack[ptrStack].DCmotor[i].motor=-1;
				AlgoidMsgRXStack[ptrStack].DCmotor[i].time=-1;
				AlgoidMsgRXStack[ptrStack].DCmotor[i].velocity=-1;
                                AlgoidMsgRXStack[ptrStack].DCmotor[i].cm=-1;
			}
			return 0;
		}
		return 1;
}

// ------------------------------------------------------------------------------------
// INITMESSAGER: Initialisation du gestionnaire de message
// - Demarrage thread messager
// -
// ------------------------------------------------------------------------------------
int InitMessager(void){
	// CREATION DU THREAD DE MESSAGERIE (Tache "MessagerTask")
	  if (pthread_create (&th_messager, NULL, MessagerTask, NULL)!= 0) {
		  //Connexion au brocker MQTT

		return (1);
	  }else return (0);

}

// ------------------------------------------------------------------------------------
// CLOSEMESSAGE: Fermeture du gestionnaire de messages
// - Fermeture MQTT
// - Stop le thread manager
// ------------------------------------------------------------------------------------
int CloseMessager(void){
	int result;
	// TERMINE LE THREAD DE MESSAGERIE
	pthread_cancel(th_messager);
	// Attends la terminaison du thread de messagerie
	result=pthread_join(th_messager, NULL);
	return (result);
}


// -------------------------------------------------------------------
// Fonction Call-Back reception d'un message MQTT
// -------------------------------------------------------------------
int mqttMsgArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    unsigned int i;
    char* payloadptr;
    int messageCharCount=0;

    payloadptr = message->payload;
    messageCharCount=message->payloadlen;

    for(i=0; i<messageCharCount; i++)
    {
    	MqttDataBuffer[i]=payloadptr[i];
    }

    // Termine la chaine de caract�re
    MqttDataBuffer[messageCharCount]=0;

	mqttDataReady=1;

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

// -------------------------------------------------------------------
// Fonction Call-Back en cas de perte de connexion MQTT
// -------------------------------------------------------------------
void mqttConnectionLost(void* context, char* cause)
{
    printf("#[MSG MANAGER] Perte de connexion avec le broker MQTT, cause# %d\n", cause);
    mqttStatus = -1;
}


// -------------------------------------------------------------------
// SENDRESPONSE
// Retourne un message MQTT
// -------------------------------------------------------------------

void sendResponse(int msgId, char * msgTo, unsigned char msgType, unsigned char msgParam, unsigned char valCnt){
	char MQTTbuf[MAX_MQTT_BUFF];
	char ackType[15], ackParam[15];
	char topic[50];

        // Mise a jour du compteur de message syst�me
        msg_stats.messageTX++;
        
	// G�n�ration du texte de reponse TYPE pour message MQTT et selection du topic de destination
	switch(msgType){
		case COMMAND : strcpy(ackType, "command"); strcpy(topic, TOPIC_COMMAND); break;			// Commande vers l'h�te ****** NON UTILISE **********
		case REQUEST : strcpy(ackType, "request"); strcpy(topic, TOPIC_COMMAND); break;			// Requ�te vers l'h�te ****** NON UTILISE **********
		case ACK : strcpy(ackType, "ack");  strcpy(topic, TOPIC_ACK); break;				// Ack vers l'h�te
		case RESPONSE : strcpy(ackType, "response"); strcpy(topic, TOPIC_RESPONSE); break;		// Reponse vers l'h�te
		case EVENT : strcpy(ackType, "event"); strcpy(topic, TOPIC_EVENT); break;				// Reponse vers l'h�tebreak;
                case DATAFLOW : strcpy(ackType, "event"); strcpy(topic, TOPIC_DATAFLOW); break;
		case ERR_TYPE : strcpy(ackType, "error"); break;
		default : strcpy(ackType, "unknown"); break;
	}

// G�n�ration du texte de reponse TYPE pour message MQTT
	switch(msgParam){
		case STOP : strcpy(ackParam, "stop"); break;
		case MOTORS : strcpy(ackParam, "motor"); break;
                case STEPPER : strcpy(ackParam, "stepper"); break;
		case pPWM : strcpy(ackParam, "pwm"); break;
                case pSERVO : strcpy(ackParam, "servo"); break;
		case MOVE : strcpy(ackParam, "move"); break;
		case DINPUT : strcpy(ackParam, "din"); break;
                case BUTTON : strcpy(ackParam, "button"); break;
		case BATTERY : strcpy(ackParam, "battery"); break;
		case DISTANCE : strcpy(ackParam, "distance"); break;
		case pLED : strcpy(ackParam, "led"); break;
		case STATUS : strcpy(ackParam, "status"); break;
                case COLORS : strcpy(ackParam, "rgb"); break;
		case ERR_PARAM : strcpy(ackParam, "error param"); break;
                case CONFIG : strcpy(ackParam, "config"); break;
                case SYSTEM : strcpy(ackParam, "system"); break;
                case ERR_HEADER : strcpy(ackParam, "error header"); break;
		default : strcpy(ackParam, "unknown"); break;
	}

	ackToJSON(MQTTbuf, msgId, msgTo, ClientID, ackType, ackParam, msgParam, valCnt);
	mqttPutMessage(&topic, MQTTbuf, strlen(MQTTbuf));
}

// -------------------------------------------------------------------
// SENDMQTTREPORT
// Retourne un message MQTT
// -------------------------------------------------------------------

void sendMqttReport(int msgId, char * msg){
	char MQTTbuf[1024];

	// Creation d'un id unique avec l'adresse mac
	sprintf(&MQTTbuf[0], "%s -> Message ID: %d -> %s", ClientID, msgId, msg);
	mqttPutMessage(TOPIC_DEBUG, MQTTbuf, strlen(MQTTbuf));
}