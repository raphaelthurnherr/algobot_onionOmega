/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   asyncSERVO.h
 * Author: raph-pnp
 *
 * Created on 5. avril 2018, 12:53
 */

#ifndef ASYNCSERVO_H
#define ASYNCSERVO_H

#ifdef __cplusplus
extern "C" {
#endif

int setAsyncServoAction(int actionNumber, int pwmName, int mode, int time);
int checkBlinkServoCount(int actionNumber, int pwmName);
int endServoAction(int actionNumber, int pwmNumber);

#ifdef __cplusplus
}
#endif

#endif /* ASYNCPWM_H */

