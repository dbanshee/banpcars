/* 
 * File:   BanPCarsServer.c
 * Author: banshee
 *
 * Created on 3 de junio de 2015, 20:02
 */

#include <stdio.h>
#include <stdlib.h>
#include "../headers/logger.h"
#include "../headers/pcarsApi.h"
#include "../headers/serialwin.h"
#include "../headers/simController.h"
#include "../headers/pcarsDump.h"

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>


#define PCARS_CONN_RETRIES          100
#define PCARS_CONN_DELAY_SECS         5    

#define ARDUINO_DEFAULT_COM_PORT     11
#define MAINP_REFRESH_DELAY_MILLIS  100

// Flags
int flagDumpWrite   = 0;
int flagDumpRead    = 0;
int flagSimBoard    = 0;

// Contexts
pCarsContext                pCarsCtx;
serialContext               serialCtx;
simCtrlContext              simCtx;
pCarsDumpWriterContext      pCarsDumpWriterCtx;
pCarsDumpReaderContext      pCarsDumpReaderCtx;


void finishServer(int error){
 
    freePCarsContext(&pCarsCtx);
    freeSerialContext(&serialCtx);
    freePCarsDumpWriterContext(&pCarsDumpWriterCtx);
    freePCarsDumpReaderContext(&pCarsDumpReaderCtx);
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

int parseArgs(int argc, char** argv){
    int i;
    for(i = 1; i < argc; i++){
        
        // -w dumpFile
        if(strcmp(argv[i], "-w") == 0) {
            
            // Enable global flag
            flagDumpWrite = 1;
            
            if(argc < i + 1){
                blog(LOG_ERROR, "Numero incorrecto de argumentos.");
                return -1;
            }
            
            i++;
            strcpy(pCarsDumpWriterCtx.fileName, argv[i]);
            
            continue;
        }
        
        
        // -r dumpFile
        if(strcmp(argv[i], "-r") == 0) {
            
            // Enable global flag
            flagDumpRead = 1;
            
            if(argc < i + 1){
                blog(LOG_ERROR, "Numero incorrecto de argumentos.");
                return -1;
            }
            
            i++;
            strcpy(pCarsDumpReaderCtx.fileName, argv[i]);
            
            continue;
        }
        
        // -d reader offset seconds
        if(strcmp(argv[i], "-d") == 0) {
            
            if(argc < i + 1){
                blog(LOG_ERROR, "Numero incorrecto de argumentos.");
                return -1;
            }
            
            i++;
            pCarsDumpReaderCtx.offsetSecs = atoi(argv[i]);
            
            continue;
        }
        
        // -com COM port
        if(strcmp(argv[i], "-com") == 0) {
            
            flagSimBoard = 1;
            
            if(argc < i + 1){
                blog(LOG_ERROR, "Numero incorrecto de argumentos.");
                return -1;
            }
            
            i++;
            serialCtx.comPortNumber = atoi(argv[i]);
            
            continue;
        }
        
        blog(LOG_ERROR, "Error parsing arguments. See Help.");
    }
    
    return 0;
}


int main(int argc, char** argv) {
    
    memset(&pCarsCtx,               0, sizeof(pCarsContext));
    memset(&serialCtx,              0, sizeof(serialContext));
    memset(&simCtx,                 0, sizeof(simCtrlContext));
    memset(&pCarsDumpWriterCtx,     0, sizeof(pCarsDumpWriterContext));
    memset(&pCarsDumpReaderCtx,     0, sizeof(pCarsDumpReaderContext));
    
    parseArgs(argc, argv);
    
    signal(SIGTERM, signalHandler);
    signal(SIGINT,  signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGSEGV, signalHandler);

    printf("-----------------------------------------\n");
    printf("-- BanPCars Server                       \n");
    printf("--     Banshee 2014                      \n");
    printf("-- Start at : %s\n", getCurrentDate());
    printf("-----------------------------------------\n\n\n");
    
    
    if(flagDumpRead == 1) {
        blog(LOG_INFO, "Iniciando Dump Reader en %s", pCarsDumpReaderCtx.fileName);
        
        pCarsDumpReaderCtx.samplingMilis = MAINP_REFRESH_DELAY_MILLIS;
        
        if(initializePCarsDumpReaderContext(&pCarsDumpReaderCtx) != 0){
            blog(LOG_ERROR, "Error inicializando contexto PCars Dump Reader. Abortando servidor ...");
            finishServer(1);
        }
    } else {
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
    }
   
    
    if(flagSimBoard) {
        blog(LOG_INFO, "Estableciendo conexion con puerto COM%d ...", serialCtx.comPortNumber);
        if(initializeSerialContext(&serialCtx) != 0){
            blog(LOG_ERROR, "Error inicializando contexto serie. Abortando servidor ...");
            finishServer(1);
        }
        blog(LOG_INFO, "Conexion con puerto COM%d establecida", ARDUINO_DEFAULT_COM_PORT);

        blog(LOG_INFO, "Inicializando Sim Controller ... ");
        simCtx.serialCtx = &serialCtx;
        
        if(flagDumpRead) 
            simCtx.pCarsSHM  = &pCarsDumpReaderCtx.pCarsSHM;
        else
            simCtx.pCarsSHM  = pCarsCtx.shmMem;
        
        blog(LOG_INFO, "Sim Controller inicializado");
    }
    
        
    if(flagDumpWrite){    
        blog(LOG_INFO, "Iniciando Dump Writer en %s ...", pCarsDumpWriterCtx.fileName);
        
        if(flagDumpRead)
            pCarsDumpWriterCtx.pCarsSHM = &pCarsDumpReaderCtx.pCarsSHM;
        else
            pCarsDumpWriterCtx.pCarsSHM = pCarsCtx.shmMem;
        
        if(initializePCarsDumpWriterContext(&pCarsDumpWriterCtx) != 0){
            blog(LOG_ERROR, "Error inicializando contexto PCars Dump Writer. Abortando servidor ...");
            finishServer(1);
        }
    }
    
    
    // Main Loop (exit by signals)
    while(1){
        
        // Check and Recover Contexts
        
        if(flagDumpRead){
            readPCarsFrame(&pCarsDumpReaderCtx);
            
            blog(LOG_INFO, "Rpms : %f", pCarsDumpReaderCtx.pCarsSHM.mRpm);   
        }
        
        if(flagSimBoard){
            refreshMainPanel(&simCtx);
        }
        
        if(flagDumpWrite){
            writePCarsFrame(&pCarsDumpWriterCtx);
        }
        
        // Main Loop Sleep
        Sleep(MAINP_REFRESH_DELAY_MILLIS); 
    }

    finishServer(0);
}


