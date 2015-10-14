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


int getPCarsSourceFields(pCarsSourceContext* ctx, jSonDocument* jSonDoc){
    int i;
    
    initializeJSonDocument(jSonDoc);
    
    openJSonArray(jSonDoc, NULL);
    for(i = 0; i < END_PCARS_FIELDS; i++){
        addJSonArrayString(jSonDoc, enumPCarsFieldsToString(i));
    }
    closeJSonArray(jSonDoc, NULL);
    
    endJSonDocument(jSonDoc);
    
    return 1;
    
}


int enumPCarsFieldsFromString(char *s){

    if(strcmp(s, "MRPM") == 0)
        return MRPM;
    else if(strcmp(s, "MMAXRPM") == 0)
        return MMAXRPM;
    else
        return -1;
}

char* enumPCarsFieldsToString(int e){
    switch (e) {
        case MRPM:
            return "MRPM";
        case MMAXRPM:
            return "MMAXRPM";
        default:
            return NULL;
    }   
}




