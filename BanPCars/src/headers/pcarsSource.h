/* 
 * File:   pcarsDump.h
 * Author: banshee
 *
 * Created on 3 de octubre de 2015, 18:31
 */

#ifndef PCARSSOURCE_H
#define	PCARSSOURCE_H

#include <stdbool.h>
#include <stdio.h>
#include "../../ext/SharedMemory.h"
#include "pcarsApi.h"
#include "pcarsDump.h"


typedef struct pCarsSourceContext {
    SharedMemory * pCarsSHM;
    pCarsContext* pCarsCtx;
    pCarsDumpReaderContext* pCarsDumpCtx;
} pCarsSourceContext;


void loadDefaultpCarsSourceContext(pCarsSourceContext* ctx);
void setPCarsSourcePCarsAPI(pCarsSourceContext* ctx, pCarsContext* pCarsCtx);
void setPCarsSourcePCarsDump(pCarsSourceContext* ctx, pCarsDumpReaderContext* pCarsDumpCtx);
int initializePCarsSourceContext(pCarsSourceContext* ctx);
void freePCarsSourceContext(pCarsSourceContext* ctx);





#endif	/* PCARSSOURCE_H */
