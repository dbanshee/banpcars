#include "../headers/pcarsApi.h"
#include "../headers/logger.h"


void loadDefaultPCarsContext(pCarsContext* pContext){
    memset(pContext, 0, sizeof(pContext));
}

int initializePCarsContext(pCarsContext* pContext){
    
    pContext->hFileHandle   = OpenFileMapping(PAGE_READONLY, FALSE, PCARS_SHAREDMEM_FILE);
    
    if(pContext->hFileHandle == NULL){
        CloseHandle(pContext->hFileHandle);
        blog(LOG_ERROR, "Error opening pcars shared memory file");
        
        return -1;
    }
    
    pContext->shmMem = (SharedMemory*) MapViewOfFile(pContext->hFileHandle, PAGE_READONLY, 0, 0, sizeof(SharedMemory));
    
    if(pContext->shmMem == NULL){
        CloseHandle(pContext->hFileHandle);
        blog(LOG_ERROR, "Error maping memory file");
        
        return -1;
    }
    
    return 0;
}

void freePCarsContext(pCarsContext* pContext){
    blog(LOG_INFO, "Liberando contexto PCars ...");
    UnmapViewOfFile(pContext->shmMem);
    CloseHandle(pContext->hFileHandle);
}


