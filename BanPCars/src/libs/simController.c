#include <stdio.h>
#include <math.h>
#include <string.h>
#include "../headers/simController.h"
#include "../headers/logger.h"
#include <sys/time.h>


#define LED_RPM_NUMLEDS             12
#define LED_RPM_START_RATIO         0.85
#define LED_BLINK_DELAY_MILLIS      250L
#define LED_NEUTRAL_DELAY_MILLIS    3000L
#define TACHOMETER_DELAY_MILLIS     250L


static int  lastGameState   = 0;
static int  lastLedOn       = 0;
static int  lastSpeed       = 0;
static int  lastGear        = 0;
static int  lastEngineOn    = 0;


static int blinkOn = 0;     // 3 estados. 0=OFF, 1=MARCADO, 2=ON
static long startBlinkTime;

static int neutralOn = 0;   // 3 estados. 0=OFF, 1=MARCADO, 2=ON
static long startNeutralTime;

static int kittOn = 0;

static long lastTachometer = 0;


void loadDefaultSimCtrlContext(simCtrlContext* ctx){
    memset(ctx, 0, sizeof(ctx));
}

void setSimCtrlCOMPort(simCtrlContext* ctx, int comPort){
    ctx->comPort = comPort;
}

void setSimCtrlPCarsSource(simCtrlContext* ctx, pCarsSourceContext * pCarsSrcCtx){
    ctx->pCarsSrcCtx = pCarsSrcCtx;
}

int initializetSimCtrlContext(simCtrlContext* ctx){
    loadDefaultSerialContext(&ctx->serialCtx);
    setSerialPort(&ctx->serialCtx, ctx->comPort);
    
     if(initializeSerialContext(&ctx->serialCtx) != 0){
        blog(LOG_ERROR, "Error inicializando contexto serie. Abortando servidor ...");
        return -1;
    }
    blog(LOG_INFO, "Conexion con puerto COM%d establecida", ctx->serialCtx.comPortNumber);
    
    return 0;
}

void freeSimCtrlContext(simCtrlContext* ctx){
//    if(ctx->serialCtx != NULL){
        sendSimBoardCmd(&ctx->serialCtx, "L1N", "0");
        freeSerialContext(&ctx->serialCtx);
//    }
}


char* itoa (int value, char * buffer, int radix) {
    if (sprintf(buffer, "%d", value)) return buffer;
    else return NULL;
}

long current_timestamp() {
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    return milliseconds;
}

int refreshLEDBar(simCtrlContext* ctx){    
    long currentTime = current_timestamp();
    char buff[5];
    
    int gameState = ctx->pCarsSrcCtx->pCarsSHM->mGameState;
    
    if(kittOn == 0 && (gameState == GAME_EXITED || gameState == GAME_FRONT_END || gameState == GAME_INGAME_PAUSED)){
        sendSimBoardCmd(&ctx->serialCtx, "L1KITT", "1");
        kittOn = 1;
    }else if(gameState == GAME_INGAME_PLAYING){
        
        if(kittOn == 1){
            sendSimBoardCmd(&ctx->serialCtx, "L1KITT", "0");
            kittOn = 0;
        }
        
        // Acceso sucio directo por estructuras. TODO: acceso getters
        // Aparentemente solo funciona el flag del limitador del pitlane !!!
        //unsigned int engineActive   = (ctx->pCarsSrcCtx->pCarsSHM->mCarFlags & (1<<1)) >> 1;
        int engineActive    = (ctx->pCarsSrcCtx->pCarsSHM->mRpm > 10); // Increiblemente con el motor calado da 0.98...

        float throttle      = ctx->pCarsSrcCtx->pCarsSHM->mThrottle;
        int nGear           = ctx->pCarsSrcCtx->pCarsSHM->mGear;
        int rpms            = ctx->pCarsSrcCtx->pCarsSHM->mRpm;
        int maxRpms         = ctx->pCarsSrcCtx->pCarsSHM->mMaxRPM/**LED_RPM_CHANGE_RATIO*/;
        double ledsThres    = maxRpms*LED_RPM_START_RATIO;
        double ledLen       = (maxRpms - ledsThres) / LED_RPM_NUMLEDS; 

        int numLeds = 0;


        if(rpms > ledsThres){
            numLeds = round((rpms - ledsThres) / ledLen); //FIXME

            if(numLeds > LED_RPM_NUMLEDS){
                //blog(LOG_WARN, "Calculo de leds erroneo. numLeds = %d, Total Leds = %d, maxRpms = %d, rpms = %d", numLeds, LED_RPM_NUMLEDS, maxRpms, rpms);
                numLeds = LED_RPM_NUMLEDS;
            }
        }


        // Cambio en numero de leds
        if(lastLedOn != numLeds){

            itoa(numLeds, buff, 10);
            sendSimBoardCmd(&ctx->serialCtx, "L1N", buff); // Disable other leds mode if active (BLINK, NEUTRAL, ... ))
            blinkOn = neutralOn = 0;    
        }


        // Blink start condition
        if(blinkOn == 0 && numLeds == LED_RPM_NUMLEDS) {
            startBlinkTime = currentTime;
            blinkOn = 1;
        }


        // Todos los leds ON mas de LED_BLINK_DELAY_MILLIS
        if(blinkOn == 1 && (current_timestamp() - startBlinkTime) > LED_BLINK_DELAY_MILLIS){
            sendSimBoardCmd(&ctx->serialCtx, "L1BLINK", "1");
            blinkOn = 2;
        }


        // Start Engine
        if(lastEngineOn != engineActive){
            if(engineActive == 1){
                sendSimBoardCmd(&ctx->serialCtx, "L1NEUTRAL", "1");
                neutralOn = 2;
            }else{
                sendSimBoardCmd(&ctx->serialCtx, "L1NEUTRAL", "0");
                neutralOn = 0;
            }
        }

        // Neutral start condition
        if(engineActive && neutralOn == 0 && nGear == 0 && numLeds == 0 && throttle == 0){
            startNeutralTime = currentTime;
            neutralOn = 1;
        }

        // Neutral start/stop
        if(neutralOn > 0){
            if(neutralOn == 2 && (nGear != 0 || numLeds != 0 || throttle != 0 || !engineActive)){
                sendSimBoardCmd(&ctx->serialCtx, "L1NEUTRAL", "0");
                neutralOn = 0;
            } else if(neutralOn == 1 && (currentTime - startNeutralTime) > LED_NEUTRAL_DELAY_MILLIS){
                sendSimBoardCmd(&ctx->serialCtx, "L1NEUTRAL", "1");
                neutralOn = 2;
            }
        }
        
        // Direct Cars magnitudes
        lastLedOn       = numLeds;
        lastGear        = nGear;
        lastEngineOn    = engineActive;
    }
    
    lastGameState = gameState;
    
    return 1;
}


void refresh8Segments(simCtrlContext* ctx){
    char buff [8];
    
    // Acceso sucio directo por estructuras. TODO: acceso getters
    int speed = ctx->pCarsSrcCtx->pCarsSHM->mSpeed *3.6;
    
    if(speed != lastSpeed){
        lastSpeed = speed;
        
        itoa(speed, buff, 10);
        sendSimBoardCmd(&ctx->serialCtx, "SEG1", buff);
    }
}

void refreshTachometer(simCtrlContext* ctx) {
    char buff [8];
    
    long currentTime = current_timestamp();
    
    if(currentTime - lastTachometer > TACHOMETER_DELAY_MILLIS){
        lastTachometer = currentTime;
        
        int rpms = ctx->pCarsSrcCtx->pCarsSHM->mRpm;
        itoa(rpms, buff, 10);
        
        sendSimBoardCmd(&ctx->serialCtx, "TC", buff);
    }
}


int refreshMainPanel(simCtrlContext* ctx){
    //refreshLEDBar(ctx);
//    refresh8Segments(ctx);
    refreshTachometer(ctx);
}


int sendSimBoardCmd(serialContext* ctx, char* cmd, char* value) {
    char buff[256];
    int i = 256;
    int readed;
    
    sprintf(buff, "SET %s=%s\r", cmd, value);
    
    blog(LOG_TRACE, "Sended (%d), '%s'", strlen(buff), buff);
    writeSerialData(ctx, buff, strlen(buff));
    
    //readed = readSerialData(ctx, buff, i);
    return 0;
}
