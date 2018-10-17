/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdlib.h>
#include <stdio.h>

int loadConfigFile(unsigned char mode);
unsigned char mode=0;
unsigned char dataCommandReady=0;

//------------------------------------------------------------------
// EXTRACTION DES DONNEES DU FICHIER
//------------------------------------------------------------------

int loadConfigFile(unsigned char mode){   
    FILE *myFile = fopen("algobot.cfg", "rw+");
    char * buffer = 0;
    
    if (myFile != NULL)
    {
           //assert(myFile);
               fseek(myFile, 0, SEEK_END);
    long length = ftell(myFile);
    fseek(myFile, 0, SEEK_SET);
    buffer = (char *) malloc(length + 1);

    buffer = malloc (length);

    if(buffer){
        fread (buffer, 1, length, myFile);
        
        sprintf(buffer, "MERDE\n");
        fprintf(myFile, "NEW: %s", buffer);
    }

    fclose(myFile); // On ferme le fichier qui a ete ouvert
    } else {
    	return(0);
    }
    return(1);
}