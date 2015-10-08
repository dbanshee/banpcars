/* 
 * File:   restWS.h
 * Author: banshee
 *
 * Created on 5 de octubre de 2015, 21:16
 */

#ifndef RESTWS_H
#define	RESTWS_H

#include <stdbool.h>
#include "../../ext/SharedMemory.h"
#include "../headers/serversocket.h"

#define DEFAULT_REST_LISTEN_PORT 8080

typedef struct restWSContext {
    int port;
    SharedMemory*  pCarsSHM;
    serverSocketContext socketCtx;
} restWSContext;


void setPort(int port);

#endif	/* RESTWS_H */

