/* 
 * File:   restWS.h
 * Author: banshee
 *
 * Created on 5 de octubre de 2015, 21:16
 */

#ifndef RESTWS_H
#define	RESTWS_H

#include <stdbool.h>
#include <pthread.h>
#include "../../ext/SharedMemory.h"
#include "../headers/serversocket.h"
#include "../headers/pcarsSource.h"

#define DEFAULT_REST_LISTEN_PORT 8080

typedef struct restWSContext {
    int                 port;
    serverSocketContext socketCtx;
    pCarsSourceContext* pCarsSrcCtx;
    pthread_t           restWSThread;
} restWSContext;


void loadDefaultRestWSContext(restWSContext* ctx);
void setRestWSPort(restWSContext* ctx, int port);
void setRestWSSource(restWSContext* ctx, pCarsSourceContext* pCarsSrcCtx);
int  initializeRestWSContext(restWSContext* ctx);
void freeRestWSContext(restWSContext* ctx);

#endif	/* RESTWS_H */

