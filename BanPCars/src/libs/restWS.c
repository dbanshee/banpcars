#include "../headers/restWS.h"
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>





int initializeRestWSContext(restWSContext* ctx){
    
    memset(&ctx->socketCtx, 0, sizeof(serverSocketContext));
    initializeSerialContext(ctx->socketCtx);
}

void setPort(int port);
void freeRestWSContext(restWSContext* ctx){

}