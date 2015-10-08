#include <string.h>

#include "../headers/pcarsSource.h"


void loadDefaultpCarsSourceContext(pCarsSourceContext* ctx){
    memset(ctx, 0, sizeof(pCarsSourceContext));
}

void setPCarsSourcePCarsAPI(pCarsSourceContext* ctx, pCarsContext* pCarsCtx){
    ctx->pCarsCtx = pCarsCtx;
}

void setPCarsSourcePCarsDump(pCarsSourceContext* ctx, pCarsDumpReaderContext* pCarsDumpCtx){
    ctx->pCarsDumpCtx = pCarsDumpCtx;
}

int initializePCarsSourceContext(pCarsSourceContext* ctx){
    
    //TODO: Check and return errors
    
    if(ctx->pCarsDumpCtx != NULL){
        ctx->pCarsSHM = &ctx->pCarsDumpCtx->pCarsSHM;
    }
    
    if(ctx->pCarsCtx != NULL){
        ctx->pCarsSHM = ctx->pCarsCtx->shmMem;
    }
    
}

void freePCarsSourceContext(pCarsSourceContext* ctx){
    // Nothing to do
}

