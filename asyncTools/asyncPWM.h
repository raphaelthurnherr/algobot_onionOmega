/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   asyncPWM.h
 * Author: raph-pnp
 *
 * Created on 5. avril 2018, 12:53
 */

#ifndef ASYNCPWM_H
#define ASYNCPWM_H

#ifdef __cplusplus
extern "C" {
#endif

int setAsyncPwmAction(int actionNumber, int ledName, int time, int count);
int checkBlinkPwmCount(int actionNumber, int ledName);
int endPwmAction(int actionNumber, int wheelNumber);

#ifdef __cplusplus
}
#endif

#endif /* ASYNCPWM_H */

