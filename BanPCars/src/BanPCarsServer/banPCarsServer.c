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
#include "../headers/serversocket.h"
#include "../headers/pcarsSource.h"

#include <termios.h>
#include <unistd.h>


#define PCARS_CONN_RETRIES          100
#define PCARS_CONN_DELAY_SECS         5    

#define ARDUINO_DEFAULT_COM_PORT     11
#define MAINP_REFRESH_DELAY_MILLIS  100

// Flags
int flagDumpWrite   = 0;
int flagDumpRead    = 0;
int flagSimBoard    = 0;

///////////////////
// Global Contexts
///////////////////
pCarsContext                pCarsCtx;
pCarsSourceContext          pCarsSrcCtx;
serialContext               serialCtx;
simCtrlContext              simCtx;
pCarsDumpWriterContext      pCarsDumpWriterCtx;
pCarsDumpReaderContext      pCarsDumpReaderCtx;


void finishServer(int error){
    
    freeSimCtrlContext(&simCtx);
    freeSerialContext(&serialCtx);
    freePCarsDumpWriterContext(&pCarsDumpWriterCtx);
    freePCarsDumpReaderContext(&pCarsDumpReaderCtx);
    freePCarsSourceContext(&pCarsSrcCtx);
    freePCarsContext(&pCarsCtx);
    
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
            setDumpWriterFileName(&pCarsDumpWriterCtx, argv[i]);
            
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
            setDumpReaderFileName(&pCarsDumpReaderCtx, argv[i]);
            
            continue;
        }
        
        // -d reader offset seconds
        if(strcmp(argv[i], "-d") == 0) {
            
            if(argc < i + 1){
                blog(LOG_ERROR, "Numero incorrecto de argumentos.");
                return -1;
            }
            
            i++;
            setDumpReaderOffSecs(&pCarsDumpReaderCtx, atoi(argv[i]));
            
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
            setSerialPort(&serialCtx, atoi(argv[i]));
            
            continue;
        }
        
        blog(LOG_ERROR, "Error parsing arguments. See Help.");
    }
    
    return 0;
}


int main(int argc, char** argv) {
    
    // Load default context values
    loadDefaultPCarsContext(&pCarsCtx);
    loadDefaultpCarsSourceContext(&pCarsSrcCtx);
    loadDefaultPCarsDumpReaderContext(&pCarsDumpReaderCtx);
    loadDefaultPCarsDumpWriterContext(&pCarsDumpWriterCtx);
    loadDefaultSerialContext(&serialCtx);
    loadDefaultSimCtrlContext(&simCtx);
    
    // Main default values and args parse
    setSerialPort(&serialCtx, ARDUINO_DEFAULT_COM_PORT);
    parseArgs(argc, argv);  // Access global contexts to config initialize
    
    // Signals callbacks
    signal(SIGTERM, signalHandler);
    signal(SIGINT,  signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGSEGV, signalHandler);

    printf("-----------------------------------------\n");
    printf("-- BanPCars Server                       \n");
    printf("--     Banshee 2014                      \n");
    printf("-- Start at : %s\n", getCurrentDate());
    printf("-----------------------------------------\n\n\n");
    
    // Source Datas initialization
    if(flagDumpRead == 1) {
        blog(LOG_INFO, "Iniciando Dump Reader en %s", pCarsDumpReaderCtx.fileName);
        
        setDumpReaderSamplingMillis(&pCarsDumpReaderCtx, MAINP_REFRESH_DELAY_MILLIS);
        
        if(initializePCarsDumpReaderContext(&pCarsDumpReaderCtx) != 0){
            blog(LOG_ERROR, "Error inicializando contexto PCars Dump Reader. Abortando servidor ...");
            finishServer(1);
        }
        
        setPCarsSourcePCarsDump(&pCarsSrcCtx, &pCarsDumpReaderCtx);
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
        
        setPCarsSourcePCarsAPI(&pCarsSrcCtx, &pCarsCtx);
        blog(LOG_INFO, "Conexion con Project Cars establecida");
    }
    
    
    if(initializePCarsSourceContext(&pCarsSrcCtx) != 0){
        blog(LOG_ERROR, "Error inicializando source de datos.");
        finishServer(1);
    }
   
    
    // Arduino Sim Board
    if(flagSimBoard) {
        blog(LOG_INFO, "Estableciendo conexion con puerto COM%d ...", serialCtx.comPortNumber);
        if(initializeSerialContext(&serialCtx) != 0){
            blog(LOG_ERROR, "Error inicializando contexto serie. Abortando servidor ...");
            finishServer(1);
        }
        blog(LOG_INFO, "Conexion con puerto COM%d establecida", serialCtx.comPortNumber);

        blog(LOG_INFO, "Inicializando Sim Controller ... ");
        simCtx.serialCtx = &serialCtx;
        
        // Ser Source Data
        setSimCtrlPCarsSource(&simCtx, &pCarsSrcCtx);
        
        blog(LOG_INFO, "Sim Controller inicializado");
    }
    
    
    // Dump Writer
    if(flagDumpWrite){    
        blog(LOG_INFO, "Iniciando Dump Writer en %s ...", pCarsDumpWriterCtx.fileName);
        
        setDumpWriterSharedMemory(&pCarsDumpWriterCtx, pCarsSrcCtx.pCarsSHM);
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



