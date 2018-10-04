/*
 * udpPublish.c
 *
 *  Created on: 4 avr. 2016
 *      Author: raph
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>           // Used for mac generation with SIMU

// MAC ADRESS
#include <net/if.h>   //ifreq
#include <sys/ioctl.h>
//UDP
#include<arpa/inet.h>

#include "udpPublish.h"

struct sockaddr_in si_other;
static int s, slen=sizeof(si_other);
char buf[UDPBUFLEN];
char message[UDPBUFLEN];

// Get MAC Adresse function
char* getMACaddr(void);

// Initialisation UDP
void initUDP();

// Envoie d'une trame heartbit sur UDP
void sendUDPHeartBit(char * udpMsg);

// -------------------------------------------------------------------
		// INITUDP, Configuration pour envoie de data broadcast sur UDP
		// -------------------------------------------------------------------
		void initUDP(){
			if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
			    {
			       // die("socket");
			    }else{
				    // Activation Broadcast
				    int broadcastEnable=1;
				    int ret=setsockopt(s, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
				     // Fin activation broadcast
			    }

			    memset((char *) &si_other, 0, sizeof(si_other));
			    si_other.sin_family = AF_INET;
			    si_other.sin_port = htons(UDPPORT);

			    /*
			    if (inet_aton(SERVER , &si_other.sin_addr) == 0)
			    {
			        fprintf(stderr, "inet_aton() failed\n");
			        exit(1);
			    }
			    */
			    si_other.sin_addr.s_addr = htonl(INADDR_BROADCAST);
		}

		// -------------------------------------------------------------------
		// SEND UDP HEARTBIT, Publie sur UDP le nom de serveur
		// -------------------------------------------------------------------
		void sendUDPHeartBit(char * udpMsg){

	        if (sendto(s, udpMsg, strlen(udpMsg) , 0 , (struct sockaddr *) &si_other, slen)==-1){
	            die("sendto()");
	        }
		}


		// -------------------------------------------------------------------
		//GETMACADDR, r�cuperation de l'adresse MAC sur ETH0
		// -------------------------------------------------------------------
		char* getMACaddr(void){
			static char myMAC[16]="0011223344556677";

                        #ifdef I2CSIMU
                           // Create a random macAdr
                           time_t current_time;
                           current_time = time(NULL);
                           
                           sprintf(&myMAC[0], "sim%d", current_time % 9999); 
                        #else
                            int fd;
			    struct ifreq ifr;
                            char *iface = "ra0";
			    unsigned char *mac = NULL;

			    memset(&ifr, 0, sizeof(ifr));

			    fd = socket(AF_INET, SOCK_DGRAM, 0);

			    ifr.ifr_addr.sa_family = AF_INET;
			    strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);

			    if (0 == ioctl(fd, SIOCGIFHWADDR, &ifr)) {
			        mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
			    }

			    close(fd);
                            
//			    sprintf(&myMAC[0], "%02x%02x%02x%02x%02x%02x",mac[0], mac[1], mac[2], mac[3], mac[4],mac[5]);
                            sprintf(&myMAC[0], "%02x%02x", mac[4],mac[5]);                            
                        #endif                           

  
			    return myMAC;
		}

		void die(char *s)
		{
		    perror(s);
		}
