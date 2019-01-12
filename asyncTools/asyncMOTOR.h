/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   asyncMOTOR.h
 * Author: raph-pnp
 *
 * Created on 5. avril 2018, 15:01
 */

#ifndef ASYNCMOTOR_H
#define ASYNCMOTOR_H

#ifdef __cplusplus
extern "C" {
#endif

int setAsyncMotorAction(int actionNumber, int motorNb, int veloc, char unit, int value);
int endWheelAction(int actionNumber, int motorNb);
int checkMotorEncoder(int actionNumber, int encoderName);
int dummyMotorAction(int actionNumber, int motorNb);
void checkDCmotorPower(void);				// Fonction temporaire pour rampe d'acceleration
int motorSpeedSetpoint(int motorName, int ratio);  // Applique la consigne de vélocité pour un moteur donné
float rpmToPercent(int motorName, int rpm);
#ifdef __cplusplus
}
#endif

#endif /* ASYNCMOTOR_H */

