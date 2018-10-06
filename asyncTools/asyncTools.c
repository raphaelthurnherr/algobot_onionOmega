/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "asyncTools.h"

MSG_TRACE msgEventHeader[50];

// -------------------------------------------------------------------
// GETSENDERFROMMSGID
// Retourne l'index dans la table des messages avec le ID correspondant
// -------------------------------------------------------------------
int getSenderFromMsgId(int msgId){
	char i = -1;
	int ptr = -1;

		for(i=0; i<50; i++){
			if(msgEventHeader[i].msgId == msgId){
				ptr = i;
				break;
			}
		}
	return ptr;			// Return -1 if no msgId found
}

// -------------------------------------------------------------------
// SAVESENDEROFMSGID
// Enregistre l'ID et expediteur dans la table des messages
// -------------------------------------------------------------------

int saveSenderOfMsgId(int msgId, char* senderName){
	unsigned char i;
	unsigned char messageIsSave=0;

		while((i<20) && (!messageIsSave)){
			if(msgEventHeader[i].msgId<=0){
				strcpy(msgEventHeader[i].msgFrom, senderName);
				msgEventHeader[i].msgId=msgId;
				messageIsSave=1;
			}
			i++;
		}

	if(messageIsSave)	return (i-1);
	else	return (-1);

}

// -------------------------------------------------------------------
// REMOVESENDEROFMSGID
// Libere l'emplacement dans la table des messages
// -------------------------------------------------------------------
int removeSenderOfMsgId(int msgId){
	unsigned char i;
		for(i=0; i<20; i++){
			if(msgEventHeader[i].msgId == msgId){
				strcpy(msgEventHeader[i].msgFrom, "*");
				msgEventHeader[i].msgId = -1;
				break;
			}
		}
	return 1;
}
