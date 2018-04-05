/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   asyncTools.h
 * Author: raph-pnp
 *
 * Created on 5. avril 2018, 13:21
 */

#ifndef ASYNCTOOLS_H
#define ASYNCTOOLS_H

#ifdef __cplusplus
extern "C" {
#endif

// Structure d'un message algoid recu
typedef struct msgTrace{
	int msgId;
	char msgFrom[32];
}MSG_TRACE;

// Fonction d'enregistrement et de recherche de l'expediteur d'un message
// Pour les evenement asynchrone
int getSenderFromMsgId(int msgId);
int saveSenderOfMsgId(int msgId, char* senderName);
int removeSenderOfMsgId(int msgId);

extern MSG_TRACE msgEventHeader[50];


#ifdef __cplusplus
}
#endif

#endif /* ASYNCTOOLS_H */

