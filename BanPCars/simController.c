#include <stdio.h>
#include <math.h>
#include "simController.h"
#include "logger.h"

#define LED_RPM_NUMLEDS         12
#define LED_RPM_START_RATIO     0.7
#define LED_RPM_CHANGE_RATIO    0.95


#define LED_BLACK 		"\x00\x00\x00"
#define LED_WHITE 		"\xff\xff\xf"
#define LED_RED   		"\xff\x00\x00"
#define LED_GREEN 		"\x00\xff\x00"
#define LED_BLUE  		"\x00\x00\xff"

typedef struct {
  uint8_t r,g,b;
} pixel_t;

static pixel_t lastLedArray  [LED_RPM_NUMLEDS];

int refreshLEDBar(simCtrlContext* ctx){    
    
    // Inicializar leds en funcion de las RPMS
    pixel_t ledArray  [LED_RPM_NUMLEDS];

    int i;
    int groupLen        = LED_RPM_NUMLEDS/3;
    int rpms            = ctx->pCarsSHM->mRpm;
    int maxRpms         = ctx->pCarsSHM->mMaxRPM*LED_RPM_CHANGE_RATIO;
    double ledsThres 	= maxRpms*LED_RPM_START_RATIO;
    double ledLen    	= (maxRpms - ledsThres) / LED_RPM_NUMLEDS; 
    int numLeds;
    
    //blog(LOG_TRACE, "Enviando RPMS %d", rpms);
    memset(ledArray, 0, sizeof(pixel_t)*LED_RPM_NUMLEDS);

    if(rpms > maxRpms*LED_RPM_START_RATIO){
        numLeds = round((rpms - ledsThres) / ledLen);

        for(i = 0; i < numLeds; i++){
          if(i < groupLen)
            // GREEN
            memcpy(&(ledArray[i]), LED_GREEN, 3);
          else if(i < groupLen*2)
            // RED
            memcpy(&(ledArray[i]), LED_RED,   3);
          else
            // BLUE
            memcpy(&(ledArray[i]), LED_BLUE,  3);
        }
    }else{
        // Apagar leds
        memset(ledArray, 0, sizeof(pixel_t)*LED_RPM_NUMLEDS);
    }
    
    
    if(memcmp(ledArray, lastLedArray, sizeof(pixel_t)*LED_RPM_NUMLEDS) != 0){
        sendLEDData(ctx->serialCtx, ledArray, LED_RPM_NUMLEDS);
        memcpy(lastLedArray, ledArray, sizeof(pixel_t)*LED_RPM_NUMLEDS);
    }
    
    return 1;
}


int sendLEDData(serialContext* ctx, pixel_t* leds, int numLeds) {
    char buf[256];
    int i = 256;
    int readed;
    
    writeSerialData(ctx, "DATA\r", 5);
    writeSerialData(ctx, leds, numLeds*3);
    readed = readSerialData(ctx, buf, i);
    return 0;
}


void refresh8Segments(simCtrlContext* ctx){
    // TODO:
}


int refreshMainPanel(simCtrlContext* ctx){
    refreshLEDBar(ctx);
    refresh8Segments(ctx);
}