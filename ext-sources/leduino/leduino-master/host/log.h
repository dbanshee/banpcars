#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <sys/time.h>

// Log levels
#define LEVEL_NONE  0
#define LEVEL_ERROR 1
#define LEVEL_WARN  2
#define LEVEL_INFO  3
#define LEVEL_DEBUG 4
#define LEVEL_TRACE 5

#define LOG_ERROR(...) logger_write(LEVEL_ERROR,__FILE__,__LINE__,NULL,__VA_ARGS__);
#define LOG_WARN(...)  logger_write(LEVEL_WARN, __FILE__,__LINE__,NULL,__VA_ARGS__);
#define LOG_INFO(...)  logger_write(LEVEL_INFO, __FILE__,__LINE__,NULL,__VA_ARGS__);
#define LOG_DEBUG(...) logger_write(LEVEL_DEBUG,__FILE__,__LINE__,NULL,__VA_ARGS__);
#define LOG_TRACE(...) logger_write(LEVEL_TRACE,__FILE__,__LINE__,NULL,__VA_ARGS__);

#define LOG_INFO_TIME(time,...)  logger_write(LEVEL_INFO, __FILE__,__LINE__,time,__VA_ARGS__);
#define LOG_DEBUG_TIME(time,...) logger_write(LEVEL_DEBUG,__FILE__,__LINE__,time,__VA_ARGS__);
#define LOG_TRACE_TIME(time,...) logger_write(LEVEL_TRACE,__FILE__,__LINE__,time,__VA_ARGS__);

int logger_init(int level, FILE* out);
int logger_set_thread_name(const char* name);
int logger_write(int level, const char* file, int line, struct timeval * time, const char* format, ...);
int logger_destroy();
#endif

