/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   asyncSTEPPER.h
 * Author: raph-pnp
 *
 * Created on 10. décembre 2018, 11:21
 */

#ifndef ASYNCSTEPPER_H
#define ASYNCSTEPPER_H

int setAsyncStepperAction(int actionNumber, int motorNb, int veloc, char unit, int value);
int endStepperAction(int actionNumber, int motorNb);
int checkStepperStatus(int actionNumber, int motorName);
int dummyStepperAction(int actionNumber, int motorName);

#ifdef __cplusplus
extern "C" {
#endif




#ifdef __cplusplus
}
#endif

#endif /* ASYNCSTEPPER_H */

