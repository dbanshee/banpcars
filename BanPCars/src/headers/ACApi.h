/* 
 * File:   ACsApi.h
 * Author: banshee
 *
 * Created on 13 de febrero de 2016, 18:467
 */

#include <stdbool.h>
#include <windows.h>
#include "../../ext/ACSharedFileOut.h"


#ifndef ACAPI_H
#define	ACAPI_H

#define AC_SHAREDMEM_PHYSICS_FILE   "Local\\acpmf_physics"
#define AC_SHAREDMEM_GRAPHICS_FILE  "Local\\acpmf_graphics"
#define AC_SHAREDMEM_STATIC_FILE    "Local\\acpmf_static"



typedef struct aCContext {
    HANDLE              hFilePhysics;
    HANDLE              hFileGraphics;
    HANDLE              hFileStatic;
    
    SPageFilePhysics*   shmPhysics;
    SPageFileGraphic*   shmGraphics;
    SPageFileStatic*    shmStatic;
} aCContext;

void loadDefaultACContext(aCContext* acCtx);
int  initializeACContext(aCContext* acCtx);
void freeACContext(aCContext* acCtx);
        
#endif	/* ACAPI_H */

