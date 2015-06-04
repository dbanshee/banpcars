#include "pcarsApi.h"
#include "logger.h"




int initializePCarsContext(pCarsContext* pContext){
    
    memset(pContext, 0, sizeof(pCarsContext));
    
    pContext->hFileHandle   = OpenFileMapping(PAGE_READONLY, FALSE, PCARS_SHAREDMEM_FILE);
    
    if(pContext->hFileHandle == NULL){
        CloseHandle(pContext->hFileHandle);
        blog(LOG_ERROR, "Error opening pcars shared memory file\n");
        
        return -1;
    }
    
    pContext->shmMem = (SharedMemory*) MapViewOfFile(pContext->hFileHandle, PAGE_READONLY, 0, 0, sizeof(SharedMemory));
    
    if(pContext->shmMem == NULL){
        CloseHandle(pContext->hFileHandle);
        blog(LOG_ERROR, "Error maping memory file\n");
        
        return -1;
    }
}

void closePCarsContext(pCarsContext* pContext){
    UnmapViewOfFile(pContext->shmMem);
    CloseHandle(pContext->hFileHandle);
}


