/* 
 * File:   restWS.h
 * Author: banshee
 *
 * Created on 5 de octubre de 2015, 21:16
 */

#ifndef RESTWS_H
#define	RESTWS_H

#include <stdbool.h>
#include <stdio.h>
#include "../../ext/SharedMemory.h"

typedef struct restWSContext {
    int port;
    SharedMemory*  pCarsSHM;
    
    
    int listenfd;
    int clientfd;
    
} restWSContext;

#endif	/* RESTWS_H */

