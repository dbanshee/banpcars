#include <string.h>
#include <stdio.h>
#include "../headers/pcarsDump.h"
#include "../headers/logger.h"

void loadDefaultPCarsDumpReaderContext(pCarsDumpReaderContext* ctx){
    memset(ctx, 0, sizeof(pCarsDumpReaderContext));
    ctx->offsetSecs = 100;
}

void setDumpReaderFileName(pCarsDumpReaderContext* ctx, char* fileName){
    strcpy(ctx->fileName, fileName);
}

void setDumpReaderOffSecs(pCarsDumpReaderContext* ctx, int secs){
    ctx->offsetSecs = secs;
}

void setDumpReaderSamplingMillis(pCarsDumpReaderContext* ctx, int milis){
    ctx->samplingMilis = milis;
}

int initializePCarsDumpReaderContext(pCarsDumpReaderContext *ctx){
   if((ctx->fileDesc = fopen(ctx->fileName, "r")) == NULL){
         blog(LOG_ERROR, "Error opening file for reading %s", ctx->fileName);
         return -1;
    }
   
   if(seekDumpFile(ctx) !=  0){
       blog(LOG_ERROR, "Error seeking file %d secs", ctx->offsetSecs);
       return -1;
   }
   
   return 0;
}


void freePCarsDumpReaderContext(pCarsDumpReaderContext* ctx) {
    if(ctx->fileDesc != NULL){
        fflush(ctx->fileDesc);
        fclose(ctx->fileDesc);
    }
}


void loadDefaultPCarsDumpWriterContext(pCarsDumpWriterContext* ctx){
    memset(ctx, 0, sizeof(pCarsDumpWriterContext));
}

void setDumpWriterFileName(pCarsDumpWriterContext* ctx, char* fileName){
    strcpy(ctx->fileName, fileName);
}

void setDumpWriterSamplingMillis(pCarsDumpWriterContext* ctx, int milis){
    ctx->samplingMilis = milis;
}

void setDumpWriterSharedMemory(pCarsDumpWriterContext* ctx, SharedMemory* shm){
    ctx->pCarsSHM = shm;
}

int initializePCarsDumpWriterContext(pCarsDumpWriterContext *ctx){
   if((ctx->fileDesc = fopen(ctx->fileName, "rw+")) == NULL){
         blog(LOG_ERROR, "Error creating file %s", ctx->fileName);
         return -1;
    }
   
   return 0;
}

void freePCarsDumpWriterContext(pCarsDumpWriterContext* ctx) {
    
    if(ctx->fileDesc != NULL){
        fflush(ctx->fileDesc);
        fclose(ctx->fileDesc);
    }
}

int seekDumpFile(pCarsDumpReaderContext *ctx) {
    return fseek(ctx->fileDesc, sizeof(SharedMemory)* ctx->offsetSecs*1000/ctx->samplingMilis, SEEK_SET);
}

int readPCarsFrame(pCarsDumpReaderContext *ctx){
    
    if(fread(&ctx->pCarsSHM, sizeof(SharedMemory), 1, ctx->fileDesc) != 1 /*n de registros, no de bytes*/) {
        
        if(feof(ctx->fileDesc)){
            blog(LOG_TRACE, "Seeking Dump Reader file %s", ctx->fileName);
            if(seekDumpFile(ctx) != 0) {
                blog(LOG_ERROR, "Error seeking Dumo Reader file %s", ctx->fileName);
                return -1;
            } 
            
            if(fread(&ctx->pCarsSHM, sizeof(SharedMemory), 1, ctx->fileDesc) != 1 /*n de registros, no de bytes*/){
                blog(LOG_ERROR, "Error reading pCars frame");
                return -1 ;
            }
            
        } else {
            blog(LOG_ERROR, "Error reading pCars frame");
            return -1 ;
        }
    }
    
    return 0;
}

int writePCarsFrame(pCarsDumpWriterContext *ctx){
    
    if(fwrite(ctx->pCarsSHM, sizeof(SharedMemory), 1, ctx->fileDesc) != 1 /*n de registros, no de bytes*/) {
        blog(LOG_ERROR, "Error writing pCars frame");
        return -1;
    }
    
    return 0;
}
