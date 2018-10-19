/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   fileIO.h
 * Author: raph
 *
 * Created on 15. octobre 2018, 21:45
 */

#ifndef FILEIO_H
#define FILEIO_H

#ifdef __cplusplus
extern "C" {
#endif

#define ERR_OPENFILE 0
#define NOERR 1
#define ERR_EXTRACT 2
#define ERR_CREATE_TAB 3

extern char LoadConfig(t_sysConfig * Config, char * fileName);
extern char SaveConfig(t_sysConfig * Config, char * fileName);

extern char * OpenConfigFromFile(char *filename);
#ifdef __cplusplus
}
#endif

#endif /* FILEIO_H */

