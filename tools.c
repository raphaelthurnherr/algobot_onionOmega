/*
 * tools.c
 *
 *  Created on: 15 avr. 2016
 *      Author: raph
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <math.h>

#include "tools.h"

int mygetch();  // Fonction getch non blocante
int PID_speedControl(int currentSpeed, int setPoint);
int speed_to_percent(float maxSpeed, float speed_cmS);

// -------------------------------------------------------------------
// SPEED_TO_PERCENT, Fonction de conversion de la vitesse mesurée en %
// de la vitesse max
// -------------------------------------------------------------------
int speed_to_percent(float maxSpeed, float speed_cmS){
    
    int result = 100 - (round((speed_cmS - maxSpeed)/maxSpeed * 100) * -1);
    return result;
}


// -------------------------------------------------------------------
// PID_SPEEDCPMTROL, Fonction PID pour gestion vitesse du moteur
// -------------------------------------------------------------------
int PID_speedControl(int currentSpeed, int setPoint){
    float Kp = 0.5;    
    float Ki = 0.01;
    float Kd = 0.00;
    float loopTimeDT = 1; 
    
    static int lastSpeed;
    int output=0;
    int outputMin=0;
    int outputMax=100;
    float error;
    static float sumError=0;
    float newSum;
    float dErrorLoopTime;
     
    error = setPoint - currentSpeed;
    newSum = (sumError + error) * loopTimeDT;
    dErrorLoopTime = (lastSpeed - currentSpeed) / loopTimeDT;
    lastSpeed = currentSpeed;
    
    output = Kp * error + Ki * sumError + Kd * dErrorLoopTime;
    
    if(output >= outputMax)
        output = outputMax;
    else
        if(output <= outputMin)
            output = outputMin;
        else 
            sumError =  newSum;
            
    
    printf("\n----- PID ADJUST  -  SETPOINT    %d    -  NEW SETPOINT %d  -----\n", setPoint, output);
    
    return output;
}


// -------------------------------------------------------------------
// MYGETCH, Fonction getch non blocante
// -------------------------------------------------------------------
int mygetch(void)
{
	struct termios oldt,newt;
	int ch;
	tcgetattr( STDIN_FILENO, &oldt );
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	ch = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}
