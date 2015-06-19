#include <stdio.h>
#include <math.h>
#include <string.h>
#include "simController.h"
#include "logger.h"

#define LED_RPM_NUMLEDS         12
#define LED_RPM_START_RATIO     0.85
#define LED_RPM_CHANGE_RATIO    0.93
#define LED_RPM_BLINK_RATIO     0.99


//#define LED_BLACK 		"\x00\x00\x00"
//#define LED_WHITE 		"\xff\xff\xff"
//#define LED_RED   		"\xff\x00\x00"
//#define LED_GREEN 		"\x00\xff\x00"
//#define LED_BLUE  		"\x00\x00\xff"

//typedef struct {
//  uint8_t r,g,b;
//} pixel_t;

// static pixel_t lastLedArray  [LED_RPM_NUMLEDS];
static int lastLedOn = 0;
static int lastSpeed = 0;
static int lastBlink = 0;


char* itoa (int value, char * buffer, int radix) {
    if (sprintf(buffer, "%d", value)) return buffer;
    else return NULL;
}

int refreshLEDBar(simCtrlContext* ctx){    
    
    char buff[5];

    int i;
    int groupLen        = LED_RPM_NUMLEDS/3;
    int rpms            = ctx->pCarsSHM->mRpm;
    int maxRpms         = ctx->pCarsSHM->mMaxRPM*LED_RPM_CHANGE_RATIO;
    double ledsThres 	= maxRpms*LED_RPM_START_RATIO;
    double ledLen    	= (maxRpms - ledsThres) / LED_RPM_NUMLEDS; 
    
    int numLeds = 0;
    
    //blog(LOG_TRACE, "Enviando RPMS %d", rpms);

    if(rpms > maxRpms*LED_RPM_START_RATIO){
        numLeds = round((rpms - ledsThres) / ledLen); //FIXME

        if(numLeds > LED_RPM_NUMLEDS){
            //blog(LOG_WARN, "Calculo de leds erroneo. numLeds = %d, Total Leds = %d, maxRpms = %d, rpms = %d", numLeds, LED_RPM_NUMLEDS, maxRpms, rpms);
            numLeds = LED_RPM_NUMLEDS;
        }
    }
    
    if(lastLedOn != numLeds){
        lastLedOn = numLeds;
        
        itoa(numLeds, buff, 10);
        sendSimBoardCmd(ctx->serialCtx, "L1N", buff);
    }

    if(rpms < maxRpms*LED_RPM_BLINK_RATIO)
        lastBlink = 0;
    
    if(rpms > maxRpms*LED_RPM_BLINK_RATIO && lastBlink == 0){
        sendSimBoardCmd(ctx->serialCtx, "BLINK", "1");
        lastBlink = 1;
    }
    
    
    return 1;
}


void refresh8Segments(simCtrlContext* ctx){
    char buff [8];
    
    int speed = ctx->pCarsSHM->mSpeed *3.6;
    
    if(speed != lastSpeed){
        lastSpeed = speed;
        
        itoa(speed, buff, 10);
        sendSimBoardCmd(ctx->serialCtx, "SEG1", buff);
    }
}


int refreshMainPanel(simCtrlContext* ctx){
    refreshLEDBar(ctx);
    refresh8Segments(ctx);
}


int sendSimBoardCmd(serialContext* ctx, char* cmd, char* value) {
    char buff[256];
    int i = 256;
    int readed;
    
    sprintf(buff, "SET %s=%s\r", cmd, value);
    
    blog(LOG_WARN, "Sended (%d), '%s'", strlen(buff), buff);
    writeSerialData(ctx, buff, strlen(buff));
    
    readed = readSerialData(ctx, buff, i);
    return 0;
}