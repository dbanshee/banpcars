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
#include "serialwin.h"
#include "simController.h"

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>


#define PCARS_CONN_RETRIES          100
#define PCARS_CONN_DELAY_SECS         5    

#define ARDUINO_DEFAULT_COM_PORT     4
#define MAINP_REFRESH_DELAY_MILLIS  100


pCarsContext   pCarsCtx;
serialContext  serialCtx;
simCtrlContext simCtx;

// Hack
//SharedMemory   hackShmMem;
//#define MAX_RPMS 		5000
//#define BASE_RPMS		1200
//#define RANGE_MILLIS		15000
//#define TIME_DELAY_MILLIS  	20

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

//------------------------
//float evaluateFunction(float x1, float y1, float x2, float y2, float p){
//
//  float m, b;
//  
//  m = (y2 - y1) / (x2 - x1);
//  b = y1 - ((y2 - y1)/(x2 - x1))*x1;
//  
//  return m*p + b;
//  
//}
//
//
//int genRPM(int millis){
//  if(millis <= 800)
//    return evaluateFunction(0, 1200, 800, MAX_RPMS, millis);
//  else if (millis > 800 && millis <= 1500)
//    return evaluateFunction(800, 1200, 1500, MAX_RPMS, millis);
//  else if (millis > 1500 && millis <= 3000)
//    return evaluateFunction(1500, 1200, 3000, MAX_RPMS, millis);
//  else if (millis > 3000 && millis <= 5000)
//    return evaluateFunction(3000, 1200, 5000, MAX_RPMS, millis);
//  else if (millis > 5000 && millis <= 8000)
//    return evaluateFunction(5000, 1200, 8000, MAX_RPMS, millis);
//  else
//    return 5000;
//}
//------------------------

int main(int argc, char** argv) {
    
    // Hack
//    int i, j;
//    int currentRPMs;
//    int currentLeds;
    
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
    int pCarsConnTry = 1;
    while(pCarsConnTry < PCARS_CONN_RETRIES && initializePCarsContext(&pCarsCtx) != 0){
        
        blog(LOG_WARN, "No se ha podido establecer conexion con Project Cars. Intento [%d/%d] ...", pCarsConnTry++, PCARS_CONN_RETRIES);
        Sleep(PCARS_CONN_DELAY_SECS*1000);
    }
    
    if(pCarsConnTry >= PCARS_CONN_RETRIES){
        blog(LOG_ERROR, "Error incializando contexto PCars. Abortando servidor ...");
        finishServer(1);
    }
    blog(LOG_INFO, "Conexion con Project Cars establecida");
    
    blog(LOG_INFO, "Estableciendo conexion con puerto COM%d ...", ARDUINO_DEFAULT_COM_PORT);
    if(initializeSerialContext(&serialCtx, ARDUINO_DEFAULT_COM_PORT) != 0){
        blog(LOG_ERROR, "Error inicializando contexto serie. Abortando servidor ...");
        finishServer(1);
    }
    blog(LOG_INFO, "Conexion con puerto COM%d establecida", ARDUINO_DEFAULT_COM_PORT);
    
    blog(LOG_INFO, "Inicializando Sim Controller ... ");
    simCtx.pCarsSHM  = pCarsCtx.shmMem;
    simCtx.serialCtx = &serialCtx;
    blog(LOG_INFO, "Sim Controller inicializado");
    
    // Hack
    //simCtx.pCarsSHM  = &hackShmMem;
    
    while(1){
        
        // Hack
        /*
        printf("Time : %d\n", genRPM(0));
        for(i = 0; i <= RANGE_MILLIS; i = i + TIME_DELAY_MILLIS){

            currentRPMs = genRPM(i);
            //colorLeds(fd, currentRPMs, MAX_RPMS);
            simCtx.pCarsSHM->mMaxRPM = MAX_RPMS;
            simCtx.pCarsSHM->mRpm    = currentRPMs;

            //printf("Time Tick [%d] : %d\n", i, currentRPMs);   

            refreshMainPanel(&simCtx);
            Sleep(MAINP_REFRESH_DELAY_MILLIS);
        }
        */
        
        refreshMainPanel(&simCtx);
        Sleep(MAINP_REFRESH_DELAY_MILLIS); 
    }

    finishServer(0);
}

