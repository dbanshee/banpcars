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
#include "../headers/restWS.h"

#include <termios.h>
#include <unistd.h>
    
#define PCARS_SERVER_VERSION "0.5a"

#define PCARS_CONN_RETRIES          100
#define PCARS_CONN_DELAY_SECS         5    

#define ARDUINO_DEFAULT_COM_PORT     11
#define RESTWS_DEFAULT_PORT        8080
#define MAINP_REFRESH_DELAY_MILLIS  100

const char * help = " \n\
\n\
BanPCars Server Version : %s \n\
\n\
Usage: \n\
\n\
banpcars.exe [-r dumpReadFile] [-d startSecs] [-rest port] [-com port] \n\
\n\
    -r dumpReadFile : Read dumpFile instead PCars shared memory \n\
    -d startSecs    : Only with -r option. Ignore first 'startSecs' of dumpFile \n\
    -rest port      : Start Rest Web Service at 'port' to export data at JSON. \n\
    -com port       : Start Arduino connection at COM 'port'. Only for BanSimBoard. \n\
\n\
\n\
\n\
Example : \n\
    banpcars.exe -r dumpSample.dmp -d 20 -rest 8080 -com 7 \n\n\
";

// Flags
int flagDumpWrite   = 0;
int flagDumpRead    = 0;
int flagSimBoard    = 0;
int flagRestWS      = 0;

///////////////////
// Global Contexts
///////////////////
pCarsContext                pCarsCtx;
pCarsDumpWriterContext      pCarsDumpWriterCtx;
pCarsDumpReaderContext      pCarsDumpReaderCtx;
pCarsSourceContext          pCarsSrcCtx;
simCtrlContext              simCtx;
restWSContext               restWSCtx;

void finishServer(int error){
    
    freeSimCtrlContext(&simCtx);
    freePCarsDumpWriterContext(&pCarsDumpWriterCtx);
    freePCarsDumpReaderContext(&pCarsDumpReaderCtx);
    freeRestWSContext(&restWSCtx);
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
            setSimCtrlCOMPort(&simCtx, atoi(argv[i]));
            
            continue;
        }
        
         // -rest port
        if(strcmp(argv[i], "-rest") == 0) {
            
            flagRestWS = 1;
            
            if(argc < i + 1){
                blog(LOG_ERROR, "Numero incorrecto de argumentos.");
                return -1;
            }
            
            i++;
            setRestWSPort(&restWSCtx, atoi(argv[i]));
            
            continue;
        }
        
        blog(LOG_ERROR, "Error parsing arguments. See Help.");
    }
    
    return 0;
}

int tempRequestHandler(int desc);

int main(int argc, char** argv) {
    
    if(argc < 2 || (argc == 2 && strcmp(argv[1], "-h") == 0)){
        printf(help, PCARS_SERVER_VERSION);
        return -1;
    }
    
    // Load default context values
    loadDefaultPCarsContext(&pCarsCtx);
    loadDefaultpCarsSourceContext(&pCarsSrcCtx);
    loadDefaultPCarsDumpReaderContext(&pCarsDumpReaderCtx);
    loadDefaultPCarsDumpWriterContext(&pCarsDumpWriterCtx);
    loadDefaultSimCtrlContext(&simCtx);
    loadDefaultRestWSContext(&restWSCtx);
    
    // Main default values and args parse
    setSimCtrlCOMPort(&simCtx, ARDUINO_DEFAULT_COM_PORT);
    setRestWSPort(&restWSCtx, RESTWS_DEFAULT_PORT);
    
     // Access global contexts to config initialize
    if(parseArgs(argc, argv)){
        printf("Bad Usage.\n\n");
        printf(help, PCARS_SERVER_VERSION);
        finishServer(-1);
    }
    
    // Signals callbacks
    signal(SIGTERM, signalHandler);
    signal(SIGINT,  signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGSEGV, signalHandler);

    printf("-----------------------------------------\n");
    printf("-- BanPCars Server      version: %s      \n", PCARS_SERVER_VERSION);
    printf("--     Banshee 2015                      \n");
    printf("-- Start at : %s\n", getCurrentDate());
    printf("-----------------------------------------\n\n\n");
    
    // Source Data initialization
    if(flagDumpRead == 1) {
        blog(LOG_INFO, "Iniciando Dump Reader en %s", pCarsDumpReaderCtx.fileName);
        
        // Read from file
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
        blog(LOG_INFO, "Inicializando Sim Controller en puerto COM%d ...", simCtx.comPort);
        
        setSimCtrlPCarsSource(&simCtx, &pCarsSrcCtx);
        
        // Set Source Data
        if(initializetSimCtrlContext(&simCtx) != 0) {
            blog(LOG_ERROR, "Error inicializado Sim Controller.  Abortando servidor ...");
            finishServer(1);
        }
        
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
    
    // Rest WS
    if(flagRestWS){
        setRestWSSource(&restWSCtx, &pCarsSrcCtx);
        
        blog(LOG_INFO, "Iniciando Web Service Rest en puerto %d ...", restWSCtx.port);
        if(initializeRestWSContext(&restWSCtx) != 0){
            blog(LOG_ERROR, "Error inicializando Web Service Rest en puerto %d. Abortando servidor ...", restWSCtx.port);
            finishServer(1);    
        }
    }
    
    
    // Main Loop (exit by signals)
    blog(LOG_INFO, "BanPCars inicializado correctamente. ");
    while(1){
        
        // Check and Recover Contexts
        
        if(flagDumpRead){
            readPCarsFrame(&pCarsDumpReaderCtx); 
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
