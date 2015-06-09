#include "serialwin.h"
#include "logger.h"
#include <stdio.h>

int initializeSerialContext(serialContext* ctx, int comPortNumber)
{
    char portName[16];
    
    memset(ctx, 0, sizeof(serialContext));
    
    sprintf(portName, "\\\\.\\COM%d", comPortNumber);
 
    ctx->comPortNumber  = comPortNumber;
    ctx->connected      = FALSE;
    ctx->hSerial        = CreateFile(   portName,
                                        GENERIC_READ | GENERIC_WRITE,
                                        0,
                                        NULL,
                                        OPEN_EXISTING,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL);

    if(ctx->hSerial == INVALID_HANDLE_VALUE)
    {
        if(GetLastError() == ERROR_FILE_NOT_FOUND)
            blog(LOG_ERROR, "Puerto COM '%s' no disponible.", portName);
        else
            blog(LOG_ERROR, "Undefined error.");   
    }
    else
    {
        DCB dcbSerialParams = {0};

        if (!GetCommState(ctx->hSerial, &dcbSerialParams))
            blog(LOG_ERROR, "No se ha podido obtener los parametros del puerto COM%d", ctx->comPortNumber);
        else
        {
            dcbSerialParams.BaudRate    = CBR_9600;
            dcbSerialParams.ByteSize    = 8;
            dcbSerialParams.StopBits    = ONESTOPBIT;
            dcbSerialParams.Parity      = NOPARITY;
            
            //Setting the DTR to Control_Enable ensures that the Arduino is properly
            //reset upon establishing a connection
            dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

            if(!SetCommState(ctx->hSerial, &dcbSerialParams))
               blog(LOG_ERROR, "No se han podido establecer los parametros serie al puerto COM%d", ctx->comPortNumber);
            else
            {
                //If everything went fine we're connected
                ctx->connected = TRUE;

                //Flush any remaining characters in the buffers 
                PurgeComm(ctx->hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

                //We wait 2s as the arduino board will be reseting
                Sleep(ARDUINO_WAIT_TIME);

                blog(LOG_INFO, "Puerto COM%d inicializado correctamente", ctx->comPortNumber);
                return 0;
            }
        }
    }
    
    return -1;
}

void freeSerialContext(serialContext* ctx)
{
    if(ctx->connected)
    {
        blog(LOG_INFO, "Liberando contexto Serial ...");
        
        ctx->connected = FALSE;
        CloseHandle(ctx->hSerial);
        
        blog(LOG_INFO, "Puerto COM%d cerrado correctamente", ctx->comPortNumber);
    }
}

int readSerialData(serialContext* ctx, void *buffer, unsigned int nbChar)
{
    DWORD bytesRead;
    unsigned int toRead;

    ClearCommError(ctx->hSerial, &ctx->errors, &ctx->status);

    if(ctx->status.cbInQue > 0)
    {
        if(ctx->status.cbInQue > nbChar)
            toRead = nbChar;
        else
            toRead = ctx->status.cbInQue;

        if(ReadFile(ctx->hSerial, buffer, toRead, &bytesRead, NULL) && bytesRead != 0){
            FlushFileBuffers(ctx->hSerial);
            blog(LOG_TRACE, "Read Serial Data (to read %d bytes) (readed %d bytes) : '%s'", nbChar, bytesRead, buffer);
            return bytesRead;
        }
    }
    
    blog(LOG_WARN, "Read Serial Data. Nothing to read");
    return -1;

}

int writeSerialData(serialContext* ctx, void *buffer, unsigned int nbChar)
{
    DWORD bytesSend = 0;

    if(!WriteFile(ctx->hSerial, (void *)buffer, nbChar, &bytesSend, NULL))
    {
        blog(LOG_ERROR, "Write serial data error (to write %d bytes) (writed %d bytes) : '%s'", nbChar, bytesSend, buffer);
        FlushFileBuffers(ctx->hSerial);
        
        ClearCommError(ctx->hSerial, &ctx->errors, &ctx->status);
        return FALSE;
    }
    else{
        FlushFileBuffers(ctx->hSerial);
        
        blog(LOG_TRACE, "Write Serial Data (to write %d bytes) (writed %d bytes) : '%s'", nbChar, bytesSend, buffer);
        return TRUE;
    }       
}

int isSerialConnected(serialContext* ctx)
{
    return ctx->connected;
}
