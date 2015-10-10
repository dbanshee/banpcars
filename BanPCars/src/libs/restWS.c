#include "../headers/restWS.h"
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "../headers/logger.h"

#define DEFAULT_MSG_LEN 1024
#define MSG_DELIMITER "\r\n\r\n"

typedef struct restMessage {
    int     msgLen;
    int     msgTotalSize;
    char*   msgBuff;
} restMessage;

int restRequestHandler(int connfd, restWSContext* ctx);
int readRequest(int connfd, restMessage* msg);
void buildResponse(restWSContext* ctx, restMessage* msg);


void loadDefaultRestWSContext(restWSContext* ctx){
    memset(ctx, 0, sizeof(restWSContext));
}

void setRestWSPort(restWSContext* ctx, int port){
    ctx->port = port;
}

void setRestWSSource(restWSContext* ctx, pCarsSourceContext* pCarsSrcCtx){
    ctx->pCarsSrcCtx = pCarsSrcCtx;
}


int initializeRestWSContext(restWSContext* ctx){
    
    loadDefaultServerSocketContext(&ctx->socketCtx);
    setServerSocketPort(&ctx->socketCtx, ctx->port);
    setServerSocketExtContext(&ctx->socketCtx, ctx);
    setServerSocketRequestHandler(&ctx->socketCtx, restRequestHandler);
    
    if(initializeServerSocketContext(&ctx->socketCtx) != 0){
        blog(LOG_ERROR, "Error inicializando socket para Web Service Rest");
        return -1;
    }
    
    // Rest WS Thread
    if(pthread_create(&ctx->restWSThread, NULL, socketServerLoop, &ctx->socketCtx) != 0){
        blog(LOG_ERROR, "Error creando Thread RestWS");
        return -1;
    }
    
    return 0;
}

void freeRestWSContext(restWSContext* ctx){
    freeServerSocketContext(&ctx->socketCtx);
}

void freeRestMsg(restMessage* msg){
    if(msg->msgTotalSize > 0) {
        free(msg->msgBuff);
    }
}

//////////////////////////
// Rest Request Callback
//////////////////////////
int restRequestHandler(int connfd, restWSContext* ctx) {
    restMessage msg;

    readRequest(connfd, &msg);
    
    buildResponse(ctx, &msg);
    sendResponse(connfd, msg);

    freeRestMsg(&msg);
}



int readRequest(int connfd, restMessage* msg) {
    int nread;
    int finalMsg = 0;
    char delim [4];
    
    memset(msg, 0, sizeof(restMessage));
    msg->msgLen = 0;
    
    // Initial allocation
    msg->msgLen = 0;
    msg->msgTotalSize = DEFAULT_MSG_LEN;
    if((msg->msgBuff = malloc(sizeof(char)*DEFAULT_MSG_LEN)) == NULL) {
        blog(LOG_ERROR, "Error allocating memory for Rest message");
        return -1;
    }
    
    
    while(finalMsg == 0 && (nread = serverReadBuffer(connfd, &msg->msgBuff[msg->msgLen], DEFAULT_MSG_LEN/2 /* lee medio buffer */)) > 0){
        msg->msgLen += nread;
        
        if(msg->msgLen >= 4){
            memcpy(delim, &msg->msgBuff[msg->msgLen-4], sizeof(char)*4);
            if(memcmp(delim, MSG_DELIMITER, sizeof(char)*4) == 0){
                finalMsg = 1;
            }
        }
            
        // Reserva mas buffer
        if(finalMsg == 0 && (msg->msgLen >=  msg->msgTotalSize / 2)){ 
            
            if((msg->msgBuff = realloc(msg->msgBuff, msg->msgTotalSize+DEFAULT_MSG_LEN)) == NULL){
                blog(LOG_ERROR, "Error reallocating memory for Rest message");
                return -1;
            }
            
            msg->msgTotalSize += DEFAULT_MSG_LEN;
        }
    }
    
    if(nread < 0){
        blog(LOG_INFO, "Client conexion error");
        return -1;
    }
}

int sendResponse(int connfd, restMessage* msg) {
    int nwrite;
    
    nwrite = serverWriteBuffer(connfd, msg->msgBuff, msg->msgLen);
    
    if(nwrite < 0 ){
        blog(LOG_ERROR, "Error sending Rest Response");
        return -1;
    }
    
    return 1;
}


void buildResponse(restWSContext* ctx, restMessage* msg){
    
    memset(msg, 0, sizeof(restMessage));
    msg->msgTotalSize = DEFAULT_MSG_LEN;
    
    // Initial allocation
    msg->msgLen = 0;
    msg->msgTotalSize = DEFAULT_MSG_LEN;
    if((msg->msgBuff = malloc(sizeof(char)*DEFAULT_MSG_LEN)) == NULL) {
        blog(LOG_ERROR, "Error allocating memory for Rest message");
//        return -1;
    }
    
    msg->msgLen = sprintf(msg->msgBuff, "RESPONSE\r\n rpms: %f, vel : %f", ctx->pCarsSrcCtx->pCarsSHM->mRpm, ctx->pCarsSrcCtx->pCarsSHM->mSpeed);
}
