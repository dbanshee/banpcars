/* 
 * File:   pcarsDump.h
 * Author: banshee
 *
 * Created on 3 de octubre de 2015, 18:31
 */

#ifndef PCARSDUMP_H
#define	PCARSDUMP_H

#include <stdbool.h>
#include <stdio.h>
#include "../../ext/SharedMemory.h"


typedef struct pCarsDumpWriterContext {
    char            fileName [2048];
    FILE*           fileDesc;
    SharedMemory*   pCarsSHM;
    int             samplingMilis;
} pCarsDumpWriterContext;

typedef struct pCarsDumpReaderContext {
    char            fileName [2048];
    long            frameCnt;
    FILE*           fileDesc;
    SharedMemory    pCarsSHM;
    int             offsetSecs;
    int             samplingMilis;
} pCarsDumpReaderContext;




// Writer
void loadDefaultPCarsDumpWriterContext(pCarsDumpWriterContext* ctx);
void setDumpWriterFileName(pCarsDumpWriterContext* ctx, char* fileName);
void setDumpWriterSamplingMillis(pCarsDumpWriterContext* ctx, int milis);
void setDumpWriterSharedMemory(pCarsDumpWriterContext* ctx, SharedMemory* shm);
int  initializePCarsDumpWriterContext(pCarsDumpWriterContext* ctx);
void freePCarsDumpWriterContext(pCarsDumpWriterContext* ctx);


// Reader
void loadDefaultPCarsDumpReaderContext(pCarsDumpReaderContext* ctx);
void setDumpReaderFileName(pCarsDumpReaderContext* ctx, char* fileName);
void setDumpReaderOffSecs(pCarsDumpReaderContext* ctx, int secs);
void setDumpReaderSamplingMillis(pCarsDumpReaderContext* ctx, int milis);
int  initializePCarsDumpReaderContext(pCarsDumpReaderContext* ctx);
void freePCarsDumpReaderContext(pCarsDumpReaderContext* ctx);


int readPCarsFrame(pCarsDumpReaderContext* ctx);
int writePCarsFrame(pCarsDumpWriterContext* ctx);


#endif	/* PCARSDUMP_H */
