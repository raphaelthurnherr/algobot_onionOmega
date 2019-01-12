/*
 * tools.h
 *
 *  Created on: 15 avr. 2016
 *      Author: raph
 */

#ifndef TOOLS_H_
#define TOOLS_H_

// Fonction getch non blocante
int mygetch(void);

int PID_speedControl(int motorId, float currentSpeed, float setPoint);
int speed_to_percent(float maxSpeed, float speed_cmS);

#endif /* TOOLS_H_ */
