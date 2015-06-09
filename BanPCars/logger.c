#include "logger.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>


int LOG_LEVEL = LOG_INFO;

void setLogLevel(int level){
    LOG_LEVEL = level;
}



char *getCurrentDate(){
    time_t t;
    time(&t);
    
    char* timeStr = ctime(&t);
    timeStr[strlen(timeStr)-1] = '\0';
    return timeStr;
}

/*
 * Ver : http://www.swig.org/Doc1.3/Varargs.html
 * 
 * void blog(int level, char* format, ...)
 * 
 * Although this code might compile, it won't do what you expect. 
 * This is because the call to printf() is compiled as a procedure call involving 
 * only two arguments. However, clearly a two-argument configuration of the call stack is 
 * completely wrong if your intent is to pass an arbitrary number of arguments to the real printf(). 
 * Needless to say, it won't work.
 * 
 * va_list argptr;
 * va_start(argptr, format);
 * sprintf(msgBuffer, format, argptr);
 * va_end(argptr);
 * 
 * void blog(int level, char* format, ...){}
 */
void blog(int level, char *format, ...){
    char msg[MAXSIZEMSG];
    
    va_list argptr;
    va_start(argptr, format);
    vsnprintf(msg, MAXSIZEMSG, format, argptr);
    va_end(argptr);
    
    if(level > LOG_LEVEL)
        return;
    
    if(level == LOG_INFO)
        printf("[INFO]  <%s>  %s\n", getCurrentDate(), msg);
    else if(level == LOG_DEBUG)
        printf("[DEBUG] <%s>  %s\n", getCurrentDate(), msg);
    else if(level == LOG_WARN)
        printf("[WARN]  <%s>  %s\n", getCurrentDate(), msg);
    else if(level == LOG_TRACE)
        printf("[TRACE]  <%s>  %s\n", getCurrentDate(), msg);
    else if(level == LOG_ERROR){
        if(errno == 0)
            printf("[ERROR] <%s>  %s\n", getCurrentDate(), msg);
        else
            printf("[ERROR] <%s>  %s\n\t%s\n", getCurrentDate(), msg, strerror(errno));
        
        errno = 0;
    }
    
    fflush(stdout);
}