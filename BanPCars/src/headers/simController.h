/* 
 * File:   simController.h
 * Author: banshee
 *
 * Created on 5 de junio de 2015, 17:04
 */

#ifndef SIMCONTROLLER_H
#define	SIMCONTROLLER_H

#include "serialwin.h"
#include <stdbool.h>
#include "pcarsSource.h"
#include "serialwin.h"


#define RPM_LED_BAR_LEN 9
#define RPM_BUFF_LEN    RPM_LED_BAR_LEN*3

typedef struct simCtrlContext {
    int                 comPort;
    serialContext       serialCtx;
    pCarsSourceContext* pCarsSrcCtx;
} simCtrlContext;


void loadDefaultSimCtrlContext(simCtrlContext* ctx);
void setSimCtrlCOMPort(simCtrlContext* ctx, int comPort);
void setSimCtrlPCarsSource(simCtrlContext* ctx, pCarsSourceContext * pCarsSrcCtx);
int initializetSimCtrlContext(simCtrlContext* ctx);
void freeSimCtrlContext(simCtrlContext* ctx);


int refreshMainPanel(simCtrlContext* ctx);

#endif	/* SIMCONTROLLER_H */

