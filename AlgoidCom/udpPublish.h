/*
 * udpPublish.h
 *
 *  Created on: 4 avr. 2016
 *      Author: raph
 */

#ifndef UDPPUBLISH_H_
#define UDPPUBLISH_H_

#define UDPPORT 53530  //The port on which to send data
#define UDPBUFLEN 512	// Size of UDP Buffer

void initUDP();
char* getMACaddr(void);
void sendUDPHeartBit(char * udpMsg);

#endif /* UDPPUBLISH_H_ */
