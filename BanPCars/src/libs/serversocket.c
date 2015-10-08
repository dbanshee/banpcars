#include "../headers/serversocket.h"
#include "../headers/logger.h"


#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>


/*
 * Loop de ejecucion de escucha de conexiones.
 */
int socketServerLoop(serverSocketContext *serverCtx){
    
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;
    unsigned int clientLen;
    
    blog(LOG_INFO, "Initializing Socket Server at port %d ... ", serverCtx->serverPort);
    
    // Socket initialization
    if((serverCtx->listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        blog(LOG_ERROR, "Error opening socket");
        abortSocketServer(serverCtx);
    }
    
    // Server address initialization
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port        = htons(serverCtx->serverPort); 
    
    // Client address initialization
    clientLen = sizeof(client_addr);

    // Bind socket to address
    if(bind(serverCtx->listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1){
        blog(LOG_ERROR, "Error binding socket");
        abortSocketServer(serverCtx);
    }

    // Listen queue
    if(listen(serverCtx->listenfd, 10) == -1){
        blog(LOG_ERROR, "Error listen on socket");
        abortSocketServer(serverCtx);
    }
    
    blog(LOG_INFO, "Server initialized on port : %d ... ", serverCtx->serverPort);
    
    // Conexion accept loop
    while(serverCtx->serverLoop){
        
        blog(LOG_INFO, "Waiting for connection ...");
        memset(&client_addr, 0, sizeof(struct sockaddr_in));
        serverCtx->clientfd = accept(serverCtx->listenfd, (struct sockaddr *) &client_addr, &clientLen);
        
        /*
         * Se comprueba el estado del descriptor del socket. Si esta cerrado, se ignora el accept,
         * puesto que provendra de la terminacion del servidor. close() en finishSocketServer() 
         */
        if(fcntl(serverCtx->listenfd, F_GETFL) != -1 && serverCtx->clientfd != -1){
            blog(LOG_INFO, "Conexion received at port : %d", ntohs(client_addr.sin_port));
            
            if(serverCtx->clientfd == -1)
                blog(LOG_ERROR, "Error on accept");
            else
                serverCtx->requestHandler(serverCtx->clientfd); // Request process

            // Close client socket
            close(serverCtx->clientfd);
        }else if(serverCtx->serverLoop == 1)
            blog(LOG_ERROR, "Error. Bad socket listen descriptor");
    }
}

void finishSocketServer(serverSocketContext *serverCtx){
    blog(LOG_INFO, "Finishing Socket Server ...");
    serverCtx->serverLoop = 0;
    close(serverCtx->listenfd);  // Close sobre el desc del socket fuerza la terminacion del bloqueo en el accept.
    close(serverCtx->clientfd);  // Close sobre la conexion del cliente activa
}

void abortSocketServer(serverSocketContext *serverCtx){
    blog(LOG_ERROR, "Aborting Socket Server ...");
    serverCtx->serverLoop = 0;
    close(serverCtx->listenfd);  // Close sobre el desc del socket fuerza la terminacion del bloqueo en el accept.
    close(serverCtx->clientfd);  // Close sobre la conexion del cliente activa
    exit(-1);
}

ssize_t serverReadBuffer(int connfd, char *buff, size_t buffSize){
    
    if(fcntl(connfd, F_GETFL) == -1){
        blog(LOG_ERROR, "Error. Input server socket is not ready.");
        return -1;
    }
    
    memset(buff, 0, buffSize);
    ssize_t nread = read(connfd, buff, buffSize);
    
    if(fcntl(connfd, F_GETFL) == -1){
        blog(LOG_ERROR, "Error. Input server socket is not ready.");
        return -1;
    }else if(nread == -1){
        blog(LOG_WARN, "Empty Request.");
        buff[0] = '\0';
        return 0;
    }else{
        cleanLine(buff);
        blog(LOG_DEBUG, "Client Request (%d bytes) : '%s'", nread, buff);
        return nread;
    }
}



ssize_t serverWriteBuffer(int connfd, char *buff, size_t buffSize){
    
    if(fcntl(connfd, F_GETFL) == -1){
        blog(LOG_ERROR, "Error. Output server socket is not ready.");
        return -1;
    }   
    
    if(buffSize == -1)
        buffSize = strlen(buff);
        
    size_t nwrite = write(connfd, buff, buffSize);
    //fflush(connfd); ?
    
    if(nwrite == -1){
        blog(LOG_ERROR, "Error sending buffer.");
        return -1;
    }else{
        blog(LOG_DEBUG, "Server Response (%d bytes) : '%s'", nwrite, buff);
        return nwrite;
    }
}


