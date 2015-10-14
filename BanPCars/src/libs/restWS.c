#include "../headers/restWS.h"
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "../headers/logger.h"
#include "../headers/jSon.h"

#define DEFAULT_MSG_LEN         1024
#define MSG_DELIMITER_WIN       "\r\n\r\n"

#define GET_LIST_HEADER         "GET /getfields HTTP/1.1\r\n"
#define GET_DATA_HEADER         "GET /getdata HTTP/1.1\r\n"

enum eRequestType {
    REQUEST_TYPE_LIST_FIELDS,
    REQUEST_TYPE_GET_DATA,
    REQUEST_TYPE_UNKOWN
};


typedef struct restRequest {
    
    // Entrada bruta
    int                 msgLen;
    int                 msgBuffTotalSize;
    char*               msgBuff;
    
    // Request
    enum eRequestType   requestType;
    char*               body; // Pointer to msgBuff area
    
} restRequest;

typedef struct restResponse {
    
    int                 msgLen;
    int                 msgBuffTotalSize;
    char*               msgBuff;
    
} restResponse;


int restRequestHandler(int connfd, restWSContext* ctx);
int readRequest(int connfd, restRequest* msg);
int processRequest(restWSContext* ctx, restRequest* req, restResponse* res);


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
    setServerSocketRequestHandler(&ctx->socketCtx, (int (*)(int,  void *)) restRequestHandler);
    
    if(initializeServerSocketContext(&ctx->socketCtx) != 0){
        blog(LOG_ERROR, "Error inicializando socket para Web Service Rest");
        return -1;
    }
    
    // Rest WS Thread
    if(pthread_create(&ctx->restWSThread, NULL, (void * (*)(void *)) socketServerLoop, &ctx->socketCtx) != 0){
        blog(LOG_ERROR, "Error creando Thread RestWS");
        return -1;
    }
    
    return 0;
}

void freeRestWSContext(restWSContext* ctx){
    freeServerSocketContext(&ctx->socketCtx);
}

void freeRestRequest(restRequest* req){
    if(req->msgBuffTotalSize > 0) {
        free(req->msgBuff);
    }
}

void freeRestResponse(restResponse* res){
    if(res->msgBuffTotalSize > 0) {
        free(res->msgBuff);
    }
}

//////////////////////////
// Rest Request Callback
//////////////////////////
int restRequestHandler(int connfd, restWSContext* ctx) {
    restRequest req;
    restResponse res;
    
    // Read raw request and categorize request
    readRequest(connfd, &req);
    
    // Process Request and produce response
    processRequest(ctx, &req, &res);
    
    // Send Response
    sendResponse(connfd, res);

    freeRestRequest(&req);
    freeRestResponse(&res);
}

int readRequest(int connfd, restRequest* req) {
    int nread;
    int finalMsg;
    char delim[4];
    int delimLen;
    int i;
    
    
    memset(req, 0, sizeof(restRequest));
    req->msgLen = 0;
    
    // Initial allocation
    req->msgLen = 0;
    req->msgBuffTotalSize = DEFAULT_MSG_LEN;
    if((req->msgBuff = malloc(sizeof(char)*DEFAULT_MSG_LEN)) == NULL) {
        blog(LOG_ERROR, "Error allocating memory for Rest message");
        return -1;
    }
    
    delimLen = strlen(MSG_DELIMITER_WIN);
    finalMsg = 0;
    while(finalMsg != 2 && (nread = serverReadBuffer(connfd, &req->msgBuff[req->msgLen], DEFAULT_MSG_LEN/2 /* lee medio buffer */)) > 0){
        req->msgLen += nread;
        
        if(req->msgLen >= delimLen){
            memcpy(delim, &req->msgBuff[req->msgLen-delimLen], sizeof(char)*delimLen);
            if(memcmp(delim, MSG_DELIMITER_WIN, sizeof(char)*delimLen) == 0){
                finalMsg++;
            }
        }
        
        // Reserva mas buffer
        if(finalMsg < 2 && (req->msgLen >=  req->msgBuffTotalSize / 2)){ 
            
            if((req->msgBuff = realloc(req->msgBuff, req->msgBuffTotalSize+DEFAULT_MSG_LEN)) == NULL){
                blog(LOG_ERROR, "Error reallocating memory for Rest message");
                return -1;
            }
            
            req->msgBuffTotalSize += DEFAULT_MSG_LEN;
        }
    }
    
    if(nread < 0){
        blog(LOG_INFO, "Client conexion error");
        return -1;
    }
    
    
    // Request Analysis
    if(strlen(req->msgBuff) > strlen(GET_LIST_HEADER) && memcmp(req->msgBuff, GET_LIST_HEADER, sizeof(char)*strlen(GET_LIST_HEADER)) == 0) {
        req->requestType = REQUEST_TYPE_LIST_FIELDS;
    }else if(strlen(req->msgBuff) > strlen(GET_DATA_HEADER) && memcmp(req->msgBuff, GET_DATA_HEADER, sizeof(char)*strlen(GET_DATA_HEADER)) == 0) {
        req->requestType = REQUEST_TYPE_GET_DATA;
    }else{
        req->requestType = REQUEST_TYPE_UNKOWN;
    }
    
    // Body Analisis
    if(req->requestType != REQUEST_TYPE_UNKOWN){
        i = 0;
        while(i < req->msgLen+delimLen && memcmp(&req->msgBuff[i], MSG_DELIMITER_WIN, delimLen) != 0)
            i++;
        
        if(i < req->msgLen+4){
            req->body = &req->msgBuff[i+4];
        }
        
        blog(LOG_TRACE, "RestWS Request Body : '%s'", req->body);
    }   
}


int processRequest(restWSContext* ctx, restRequest* req, restResponse* res){
    
    jSonDocument jSonDoc;
    char* s;
    
    
    switch (req->requestType) {
        case REQUEST_TYPE_LIST_FIELDS: 
            
            // Returns jSon with loaded data
            if(getPCarsSourceFields(ctx->pCarsSrcCtx, &jSonDoc) == -1) {
                blog(LOG_ERROR, "Error recuperando lista de campos de source PCars");
                return -1;
            }
            
            s = getJSonString(&jSonDoc);
            res->msgLen             = strlen(s);
            res->msgBuffTotalSize   = res->msgLen + 1;            
            res->msgBuff            = malloc(sizeof(char)*res->msgLen); // TODO: Tratar error
            strcpy(res->msgBuff, s);
            
            freeJSonDocument(&jSonDoc);
            
            break;
            
        case REQUEST_TYPE_GET_DATA :
            
            if(parseJSon(&jSonDoc, req->body) == -1){
                // Error parsing input. No JSon
            }
            
            // TODO
            
            break;
            
        case REQUEST_TYPE_UNKOWN:
            
            // TODO
            
            break;
        default :
            // Unkown error.
            return -1;
    }
    
    return 0;
}


int buildResponse(restWSContext* ctx, restRequest* msg){
    
    memset(msg, 0, sizeof(restRequest));
    msg->msgBuffTotalSize = DEFAULT_MSG_LEN;
    
    // Initial allocation
    msg->msgLen = 0;
    msg->msgBuffTotalSize = DEFAULT_MSG_LEN;
    if((msg->msgBuff = malloc(sizeof(char)*DEFAULT_MSG_LEN)) == NULL) {
        blog(LOG_ERROR, "Error allocating memory for Rest message");
        return -1;
    }
    
    msg->msgLen = sprintf(msg->msgBuff, "RESPONSE\r\n rpms: %f, vel : %f", ctx->pCarsSrcCtx->pCarsSHM->mRpm, ctx->pCarsSrcCtx->pCarsSHM->mSpeed);
}


int sendResponse(int connfd, restResponse* res) {
    int nwrite;
    
    nwrite = serverWriteBuffer(connfd, res->msgBuff, res->msgLen);
    
    if(nwrite < 0 ){
        blog(LOG_ERROR, "Error sending Rest Response");
        return -1;
    }
    
    return 1;
}