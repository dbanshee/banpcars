#include "serialwin.h"
#include "logger.h"
#include <stdio.h>

int initializeSerialContext(serialContext* ctx, int comPortNumber)
{
    memset(ctx, 0, sizeof(serialContext));
    
    char portName[16];
    sprintf(portName, "\\\\.\\COM%d", comPortNumber);
 
    ctx->comPortNumber   = comPortNumber;
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
            //Define serial connection parameters for the arduino board
            dcbSerialParams.BaudRate    = CBR_9600;
            dcbSerialParams.ByteSize    = 8;
            dcbSerialParams.StopBits    = ONESTOPBIT;
            dcbSerialParams.Parity      = NOPARITY;
            
            //Setting the DTR to Control_Enable ensures that the Arduino is properly
            //reset upon establishing a connection
            dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

             if(!SetCommState(ctx->hSerial, &dcbSerialParams))
                blog(LOG_ERROR, "NO se han podido establecer los parametros serie al puerto COM%d", ctx->comPortNumber);
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

int readSerialData(serialContext* ctx, char *buffer, unsigned int nbChar)
{
    DWORD bytesRead;
    unsigned int toRead;

    //Use the ClearCommError function to get status info on the Serial port
    ClearCommError(ctx->hSerial, &ctx->errors, &ctx->status);

    //Check if there is something to read
    if(ctx->status.cbInQue > 0)
    {
        //If there is we check if there is enough data to read the required number
        //of characters, if not we'll read only the available characters to prevent
        //locking of the application.
        if(ctx->status.cbInQue > nbChar)
            toRead = nbChar;
        else
            toRead = ctx->status.cbInQue;

        //Try to read the require number of chars, and return the number of read bytes on success
        if(ReadFile(ctx->hSerial, buffer, toRead, &bytesRead, NULL) && bytesRead != 0){
            FlushFileBuffers(ctx->hSerial);
            return bytesRead;
        }
    }

    //If nothing has been read, or that an error was detected return -1
    return -1;

}

int writeSerialData(serialContext* ctx, char *buffer, unsigned int nbChar)
{
    DWORD bytesSend;

    //Try to write the buffer on the Serial port
    if(!WriteFile(ctx->hSerial, (void *)buffer, nbChar, &bytesSend, 0))
    {
        FlushFileBuffers(ctx->hSerial);
        
        //In case it don't work get comm error and return false
        ClearCommError(ctx->hSerial, &ctx->errors, &ctx->status);
        return FALSE;
    }
    else
        return TRUE;
}

int isSerialConnected(serialContext* ctx)
{
    return ctx->connected;
}
