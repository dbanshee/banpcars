#include "../headers/ACApi.h"
#include "../headers/logger.h"


void loadDefaultACContext(aCContext* acCtx){
    memset(acCtx, 0, sizeof(aCContext));
}

int initializeACContext(aCContext* acCtx){
    
    // Static
    acCtx->hFileStatic   = OpenFileMapping(PAGE_READONLY, FALSE, AC_SHAREDMEM_STATIC_FILE);
    if(acCtx->hFileStatic == NULL){
        freeACContext(acCtx);
        blog(LOG_ERROR, "Error opening AC shared memory STATIC file");
        
        return -1;
    }
    
    acCtx->shmStatic = (SPageFileStatic*) MapViewOfFile(acCtx->hFileStatic, PAGE_READONLY, 0, 0, sizeof(SPageFileStatic));
    if(acCtx->shmStatic == NULL){
        freeACContext(acCtx);
        blog(LOG_ERROR, "Error maping memory file");
        
        return -1;
    }
    
    // Physics
    acCtx->hFilePhysics   = OpenFileMapping(PAGE_READONLY, FALSE, AC_SHAREDMEM_PHYSICS_FILE);
    if(acCtx->hFilePhysics == NULL){
        freeACContext(acCtx);
        blog(LOG_ERROR, "Error opening AC shared memory PHYSICS file");
        
        return -1;
    }
    
    acCtx->shmPhysics = (SPageFilePhysics*) MapViewOfFile(acCtx->hFilePhysics, PAGE_READONLY, 0, 0, sizeof(SPageFilePhysics));
    if(acCtx->shmPhysics == NULL){
        freeACContext(acCtx);
        blog(LOG_ERROR, "Error maping memory file");
        
        return -1;
    }
    
    // Graphics
    acCtx->hFileGraphics   = OpenFileMapping(PAGE_READONLY, FALSE, AC_SHAREDMEM_GRAPHICS_FILE);
    if(acCtx->hFileGraphics == NULL){
        freeACContext(acCtx);
        blog(LOG_ERROR, "Error opening AC shared memory GRAPHICS file");
        
        return -1;
    }
    
    acCtx->shmGraphics = (SPageFileGraphic*) MapViewOfFile(acCtx->hFileGraphics, PAGE_READONLY, 0, 0, sizeof(SPageFileGraphic));
    if(acCtx->shmGraphics == NULL){
        freeACContext(acCtx);
        blog(LOG_ERROR, "Error maping memory file");
        
        return -1;
    }
    
    return 0;
}

void freeACContext(aCContext* acCtx){
    blog(LOG_INFO, "Liberando contexto PCars ...");
    
    // Static
    if(acCtx->shmStatic != NULL){
        UnmapViewOfFile(acCtx->shmStatic);
    }
    
    if(acCtx->hFileStatic != NULL){
        CloseHandle(acCtx->hFileStatic);
    }
    
    // Physics
    if(acCtx->shmPhysics != NULL){
        UnmapViewOfFile(acCtx->shmPhysics);
    }
    
    if(acCtx->hFilePhysics != NULL){
        CloseHandle(acCtx->hFilePhysics);
    }
    
    // Graphics
    if(acCtx->shmGraphics != NULL){
        UnmapViewOfFile(acCtx->shmGraphics);
    }
    
    if(acCtx->hFileGraphics != NULL){
        CloseHandle(acCtx->hFileGraphics);
    }
}
