/* 
 * File:   BanPCarsServer.c
 * Author: banshee
 *
 * Created on 3 de junio de 2015, 20:02
 */

#include <stdio.h>
#include <stdlib.h>
#include "logger.h"
#include "pcarsApi.h"
#include "serial.h"
#include "serialwin.h"
#include "simController.h"

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>


#define ARDUINO_COM_PORT            5
#define MAINP_REFRESH_DELAY_MILLIS 100


pCarsContext   pCarsCtx;
serialContext  serialCtx;
simCtrlContext simCtx;

SharedMemory   hackShmMem;

void finishServer(int error){
 
    freePCarsContext(&pCarsCtx);
    freeSerialContext(&serialCtx);
    exit(error);
}


void signalHandler(int sigNum){
    
    if(sigNum == SIGTERM)
        blog(LOG_INFO, "Log Signal SIGTERM received");
    else if(sigNum == SIGINT)
        blog(LOG_INFO, "Log Signal SIGINT received");
    else if(sigNum == SIGQUIT)
        blog(LOG_INFO, "Log Signal SIGQUIT received");
    else if(sigNum == SIGSEGV)
        blog(LOG_INFO, "Log Signal SIGSEGV received");
    else
        return;
    
    finishServer(0);
}


int main(int argc, char** argv) {
    
    memset(&pCarsCtx,  0, sizeof(pCarsContext));
    memset(&serialCtx, 0, sizeof(serialContext));
    memset(&simCtx,    0, sizeof(simCtrlContext));
    
    signal(SIGTERM, signalHandler);
    signal(SIGINT,  signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGSEGV, signalHandler);

    printf("-----------------------------------------\n");
    printf("-- BanPCars Server                       \n");
    printf("--     Banshee 2014                      \n");
    printf("-- Start at : %s\n", getCurrentDate());
    printf("-----------------------------------------\n\n\n");
    
    
    blog(LOG_INFO, "Estableciendo conexion con Project Cars ...");
    if(initializePCarsContext(&pCarsCtx) != 0){
        blog(LOG_ERROR, "Error incializando contexto PCars. Abortando servidor ...");
        finishServer(1);
    }
    
    blog(LOG_INFO, "Estableciendo conexion con puerto COM%d ...", ARDUINO_COM_PORT);
    if(initializeSerialContext(&serialCtx, ARDUINO_COM_PORT) != 0){
        blog(LOG_ERROR, "Error inicializando contexto serie. Abortando servidor ...");
        finishServer(1);
    }
    
    blog(LOG_INFO, "Inicializando Sim Controller ... ");
    simCtx.pCarsSHM  = pCarsCtx.shmMem;
    simCtx.serialCtx = &serialCtx;
    
    // Hack
    //simCtx.pCarsSHM  = &hackShmMem;
    
    while(1){
        refreshMainPanel(&simCtx);
        Sleep(MAINP_REFRESH_DELAY_MILLIS);
    }

    finishServer(0);
}

