#include <stdio.h>
#include <math.h>
#include <string.h>
#include "../headers/simController.h"
#include "../headers/logger.h"
#include <sys/time.h>


#define LED_RPM_NUMLEDS                 12
#define LED_RPM_START_RATIO             0.85
#define LED_BLINK_DELAY_MILLIS          250L
#define LED_NEUTRAL_DELAY_MILLIS        3000L
#define TACHOMETER_DELAY_MILLIS         100L

#define BINARY_CMD_CHANGE_MODE          0x00
#define BINARY_CMD_LED1                 0x01
#define BINARY_CMD_BLINK                0x02
#define BINARY_CMD_NEUTRAL              0x03
#define BINARY_CMD_KITT                 0x04
#define BINARY_CMD_TC                   0x05


static int  lastGameState   = 0;
static int  lastLedOn       = 0;
static int  lastSpeed       = 0;
static int  lastTCRpms      = 0;
static int  lastGear        = 0;
static int  lastEngineOn    = 0;


static int blinkOn = 0;     // 3 estados. 0=OFF, 1=MARCADO, 2=ON
static long startBlinkTime;

static int neutralOn = 0;   // 3 estados. 0=OFF, 1=MARCADO, 2=ON
static long startNeutralTime;

static int kittOn = 0;

static long lastTachometer = 0;

int sendSimBoardCmdByte(serialContext* ctx, char cmd, char value);
int sendSimBoardCmdInt(serialContext* ctx, char cmd, uint16_t value);

void loadDefaultSimCtrlContext(simCtrlContext* ctx){
    memset(ctx, 0, sizeof(ctx));
}

void setSimCtrlCOMPort(simCtrlContext* ctx, int comPort){
    ctx->comPort = comPort;
}

void setSimCtrlSimSource(simCtrlContext* ctx, simSourceContext * simSrcCtx){
    ctx->simSrcCtx = simSrcCtx;
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
        sendSimBoardCmdByte(&ctx->serialCtx, BINARY_CMD_LED1, 0);
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
    
    int gameState = getGameState(ctx->simSrcCtx);
    
    if(kittOn == 0 && (gameState == GAME_EXITED || gameState == GAME_FRONT_END || gameState == GAME_INGAME_PAUSED)){
        sendSimBoardCmdByte(&ctx->serialCtx, BINARY_CMD_KITT, 1);
        kittOn = 1;
    }else if(gameState == GAME_INGAME_PLAYING){
        
        if(kittOn == 1){
            sendSimBoardCmdByte(&ctx->serialCtx, BINARY_CMD_KITT, 0);
            kittOn = 0;
        }
        
        // Acceso sucio directo por estructuras. TODO: acceso getters
        // Aparentemente solo funciona el flag del limitador del pitlane !!!
        //unsigned int engineActive   = (ctx->simSrcCtx->pCarsSourceCtx->pCarsSHM->mCarFlags & (1<<1)) >> 1;
        

//        float throttle      = ctx->simSrcCtx->pCarsSourceCtx.pCarsSHM->mThrottle;
        float throttle      = getThrottle(ctx->simSrcCtx);
//        int nGear           = ctx->simSrcCtx->pCarsSourceCtx.pCarsSHM->mGear;
        int nGear           = getGear(ctx->simSrcCtx);
//        int rpms            = ctx->simSrcCtx->pCarsSourceCtx.pCarsSHM->mRpm;
        int rpms            = getRpm(ctx->simSrcCtx);
//        int maxRpms         = ctx->simSrcCtx->pCarsSourceCtx.pCarsSHM->mMaxRPM/**LED_RPM_CHANGE_RATIO*/;
        int maxRpms         = getMaxRpms(ctx->simSrcCtx);
        double ledsThres    = maxRpms*LED_RPM_START_RATIO;
        double ledLen       = (maxRpms - ledsThres) / LED_RPM_NUMLEDS; 
        int engineActive    = (rpms > 10); // Increiblemente con el motor calado da 0.98 (en PCars)...

        int numLeds = 0;


        if(rpms > ledsThres){
            numLeds = round((rpms - ledsThres) / ledLen); //FIXME

            if(numLeds > LED_RPM_NUMLEDS){
                numLeds = LED_RPM_NUMLEDS;
            }
        }


        // Cambio en numero de leds
        if(lastLedOn != numLeds){
            itoa(numLeds, buff, 10);
            sendSimBoardCmdByte(&ctx->serialCtx, BINARY_CMD_LED1, (uint16_t) numLeds); // Disable other leds mode if active (BLINK, NEUTRAL, ... ))
            blinkOn = neutralOn = 0;    
        }


        // Blink start condition
        if(blinkOn == 0 && numLeds == LED_RPM_NUMLEDS) {
            startBlinkTime = currentTime;
            blinkOn = 1;
        }


        // Todos los leds ON mas de LED_BLINK_DELAY_MILLIS
        if(blinkOn == 1 && (current_timestamp() - startBlinkTime) > LED_BLINK_DELAY_MILLIS){
            sendSimBoardCmdByte(&ctx->serialCtx, BINARY_CMD_BLINK, 1);
            blinkOn = 2;
        }


        // Start Engine
        if(lastEngineOn != engineActive){
            if(engineActive == 1){
                sendSimBoardCmdByte(&ctx->serialCtx, BINARY_CMD_NEUTRAL, 1);
                neutralOn = 2;
            }else{
                sendSimBoardCmdByte(&ctx->serialCtx, BINARY_CMD_NEUTRAL, 0);
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
                sendSimBoardCmdByte(&ctx->serialCtx, BINARY_CMD_NEUTRAL, 0);
                neutralOn = 0;
            } else if(neutralOn == 1 && (currentTime - startNeutralTime) > LED_NEUTRAL_DELAY_MILLIS){
                sendSimBoardCmdByte(&ctx->serialCtx, BINARY_CMD_NEUTRAL, 1);
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
    int speed = getSpeed(ctx->simSrcCtx);
    
    if(speed != lastSpeed){
        lastSpeed = speed;
        
        itoa(speed, buff, 10);
        //sendSimBoardCmd(&ctx->serialCtx, "SEG1", buff);
    }
}

void refreshTachometer(simCtrlContext* ctx) {
    long currentTime = current_timestamp();
    
    if(currentTime - lastTachometer > TACHOMETER_DELAY_MILLIS){
        lastTachometer = currentTime;
        
        int rpms = getRpm(ctx->simSrcCtx);
        if(lastTCRpms != rpms) {
            sendSimBoardCmdInt(&ctx->serialCtx, BINARY_CMD_TC, (uint16_t) rpms);
        }
        
        lastTCRpms = rpms;
    }
}


int refreshMainPanel(simCtrlContext* ctx){
    refreshLEDBar(ctx);
//    refresh8Segments(ctx);
    refreshTachometer(ctx);
}

int sendSimBoardCmdByte(serialContext* ctx, char cmd, char value) {
    char buff[3];
    int readed;
    
    buff[0] = cmd;
    buff[1] = value;
    buff[2] = '\n';
    
    writeSerialData(ctx, buff, 3);
    readed = readSerialData(ctx, buff, 1);
    if(readed < 1) {
        printf("No serial Data readed!! TC %d", value);
    }
    return 0;
}

int sendSimBoardCmdInt(serialContext* ctx, char cmd, uint16_t value) {
    byte buff[4];
    int readed;
    
    buff[0] = cmd;
    buff[1] = (value >> 8);
    buff[2] = value & 0xff;
    buff[3] = '\n';
    
    writeSerialData(ctx, buff, 4);
    readed = readSerialData(ctx, buff, 1);
    if(readed < 1) {
        printf("No serial Data readed!! TC %d", value);
    }
    return 0;
}