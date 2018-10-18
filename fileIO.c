/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdlib.h>
#include <stdio.h>

char * OpenConfigFromFile(char *filename);
unsigned char mode=0;
unsigned char dataCommandReady=0;

//------------------------------------------------------------------
// EXTRACTION DES DONNEES DU FICHIER
//------------------------------------------------------------------

char * OpenConfigFromFile(char *filename){   
    FILE *myFile = fopen(filename, "rw+");
   static char *srcDataBuffer = NULL;
   
   int string_size, read_size;

   if (myFile)
   {
       // Seek the last byte of the file
       fseek(myFile, 0, SEEK_END);
       // Offset from the first to the last byte, or in other words, filesize
       string_size = ftell(myFile);
       // go back to the start of the file
       rewind(myFile);

       // Allocate a string that can hold it all
       srcDataBuffer = (char*) malloc(sizeof(char) * (string_size + 1) );

       // Read it all in one operation
       read_size = fread(srcDataBuffer, sizeof(char), string_size, myFile);

       // fread doesn't set it so put a \0 in the last position
       // and buffer is now officially a string
       srcDataBuffer[string_size] = '\0';

       if (string_size != read_size)
       {
           // Something went wrong, throw away the memory and set
           // the buffer to NULL
           free(srcDataBuffer);
           srcDataBuffer = NULL;
       }

       // Always remember to close the file.
       fclose(myFile);
    }
      
   return(srcDataBuffer);
}