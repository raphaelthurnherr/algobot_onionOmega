/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   asyncLED.h
 * Author: raph-pnp
 *
 * Created on 5. avril 2018, 14:53
 */

#ifndef ASYNCLED_H
#define ASYNCLED_H

#ifdef __cplusplus
extern "C" {
#endif

int setAsyncLedAction(int actionNumber, int ledName, int mode, int time, int count);
int endLedAction(int actionNumber, int ledNumber);
int checkBlinkLedCount(int actionNumber, int ledName);


#ifdef __cplusplus
}
#endif

#endif /* ASYNCLED_H */

