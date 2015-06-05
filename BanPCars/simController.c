#include <stdio.h>
#include "simController.h"
#include "logger.h"


void sendRPMS(simCtrlContext* ctx){
    
    uint8_t data[RPM_BUFF_LEN];
    //memset(data, 0, RPM_BUFF_LEN);
    
    // Inicializar leds en funcion de las RPMS
    
    
    blog(LOG_TRACE, "Enviando RPMS %f", ctx->pCarsSHM->mRpm);
    if(writeSerialData(ctx->serialCtx, data, RPM_BUFF_LEN) != TRUE)
        blog(LOG_ERROR, "No se han podido enviar RPMs.");
}