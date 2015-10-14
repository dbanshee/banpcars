/* 
 * File:   serversocket.h
 * Author: banshee
 *
 * Created on 5 de octubre de 2015, 21:40
 */

#ifndef SERVERSOCKET_H
#define	SERVERSOCKET_H

#include <sys/types.h>

#define REQUEST_OK      "REQUEST-OK"
#define REQUEST_ERROR   "REQUEST-ERROR"


typedef struct serverSocketContext {
    int serverPort;
    int serverLoop;
    int listenfd;
    int clientfd; // Cambiar por array o lista cuando se permitan conexiones concurrentes.
    
    void* extCtx;
    int (*requestHandler)(int, void*);
} serverSocketContext;


// Public Functions
void loadDefaultServerSocketContext(serverSocketContext* ctx);
void setServerSocketPort(serverSocketContext* ctx, int port);
void setServerSocketExtContext(serverSocketContext* ctx, void* extCtx);
void setServerSocketRequestHandler(serverSocketContext* ctx, int (*requestHandler)(int, void*));
int  initializeServerSocketContext(serverSocketContext* ctx);
void freeServerSocketContext(serverSocketContext* ctx);


int     socketServerLoop(serverSocketContext *serverCtx);
void    abortSocketServer(serverSocketContext *serverCtx);
void    finishSocketServer(serverSocketContext *serverCtx);
ssize_t serverReadBuffer(int connfd, char *buff, size_t buffSize);
ssize_t serverWriteBuffer(int connfd, char *buff, size_t buffSize);


#endif	/* SERVERSOCKET_H */

