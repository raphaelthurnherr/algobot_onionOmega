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
