#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <sys/time.h>
#include <libgen.h>

#include "log.h"

typedef struct {
    // Config
    int level;
    FILE *output;

    // Sync
    pthread_mutex_t mutex;

    // Time
    struct timeval t_tv;
    time_t t_time;
    double t_now;
    struct tm *t_tm;
    char t_tmbuf[64];
    char t_str[64];

    // Input buffer
    char *strbuf;
    size_t strbuflen;

    // Thread Names
    pthread_t* th_id;
    char** th_name;
    int th_len;
} Logger;

static int init = 0;
static Logger logger;

#define MIN_BUF_SIZE 128

static int find_thread_index() {
    int i;
    for (i = 0; i < logger.th_len; i++) {
        if (logger.th_id[i] == pthread_self()) {
            return i;
        }
    }
    return -1;
}

static int set_thread_name(const char* name) {
    int i = find_thread_index();
    if (i < 0) {
        logger.th_len++;
        logger.th_id = realloc(logger.th_id, logger.th_len * sizeof (pthread_t*));
        if (logger.th_id == NULL) {
            return -1;
        }
        logger.th_name = realloc(logger.th_name, logger.th_len * sizeof (char**));
        if (logger.th_name == NULL) {
            return -1;
        }
        i = logger.th_len - 1;
    }
    logger.th_id[i] = pthread_self();
    logger.th_name[i] = strdup(name);
    return 0;
}

int logger_set_thread_name(const char* name) {
    if (!init) return -1;

    pthread_mutex_lock(&logger.mutex);

    int res = set_thread_name(name);

    pthread_mutex_unlock(&logger.mutex);

    return res;
}

static char* logger_get_thread_name() {
    int i = find_thread_index();
    if (i < 0) {
        char name[64];
        sprintf(name, "%08X", (unsigned int)pthread_self());
        set_thread_name(name);
        i = find_thread_index();
    }
    if (i < 0) return "Unknown Thread";
    return logger.th_name[i];
}

static char *get_level(int level) {
    switch (level) {
        case LEVEL_NONE: return "NONE";
        case LEVEL_ERROR: return "ERROR";
        case LEVEL_WARN: return "WARN";
        case LEVEL_INFO: return "INFO";
        case LEVEL_DEBUG: return "DEBUG";
        default: return "TRACE";
    }
}

static char *get_error() {
    if (errno == 0)
        return "None";

    char* err = strerror(errno);
    errno=0;
    return err;
}

int logger_init(int level, FILE* output) {
    if (init) return 0;
    memset(&logger, 0, sizeof (logger));
    logger.level = level;
    logger.output = output;

    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&(logger.mutex), &mattr);

    init = 1;
    return 0;
}

int logger_destroy() {
    if (!init) return 0;
    pthread_mutex_destroy(&(logger.mutex));

    if (logger.strbuf != NULL) {
        free(logger.strbuf);
        logger.strbuf = NULL;
    }

    if (logger.th_name != NULL) {
        int i;
        for (i = 0; i < logger.th_len; i++) {
            if (logger.th_name[i] != NULL) {
                free(logger.th_name[i]);
                logger.th_name[i] = NULL;
            }
        }
        free(logger.th_name);
        logger.th_name = NULL;
    }

    if (logger.th_id != NULL) {
        free(logger.th_id);
        logger.th_id = NULL;
    }

    return 0;
}

static int updateTimestamp() {
    gettimeofday(&(logger.t_tv), NULL);
    logger.t_time = logger.t_tv.tv_sec;
    logger.t_now = logger.t_tv.tv_sec + logger.t_tv.tv_usec * 1E-6;
    logger.t_tm = localtime(&(logger.t_time));
    strftime(logger.t_tmbuf, sizeof (logger.t_tmbuf), "%Y-%m-%d %H:%M:%S", logger.t_tm);
    snprintf(logger.t_str, sizeof (logger.t_str), "%s.%06d", logger.t_tmbuf, (int)logger.t_tv.tv_usec);
    return 0;
}

static int makeMessage(const char* format, va_list vargs) {
    int n;

    while (1) {
        va_list tmpargs;
        va_copy(tmpargs, vargs);

        /* Try to print in the allocated space */
        n = vsnprintf(logger.strbuf, logger.strbuflen, format, vargs);

        va_end(tmpargs);

        /* Check error code */
        if (n < 0) return -1;

        /* If that worked, return the string */
        if (n < logger.strbuflen) return 0;

        /* Else try again with more space */
        logger.strbuflen = n + 1; /* Precisely what is needed */

        if ((logger.strbuf = realloc(logger.strbuf, logger.strbuflen)) == NULL) {
            return -1;
        }
    }
}

int logger_write(int level, const char* file, int line, struct timeval * time, const char* format, ...) {
    if (!init) return -1;

    if (level > logger.level) return 0;

    pthread_mutex_lock(&logger.mutex);

    updateTimestamp();

    double tdiff;
    if (time != NULL) {
        tdiff = time->tv_sec + time->tv_usec * 1E-6;
        tdiff = logger.t_now - tdiff;
    }

    va_list vargs;
    va_start(vargs, format);
    makeMessage(format, vargs);
    va_end(vargs);

    char* tmp_file = strdup(file);
    char* filename = basename(tmp_file);

    if (errno != 0) {
        fprintf(logger.output, "[%s] [%s] [%5s] [%s:%d] [ERRNO: %s] %s\r\n", logger.t_str, logger_get_thread_name(), get_level(level), filename, line, get_error(), logger.strbuf);
    } else if (time != NULL) {
        fprintf(logger.output, "[%s] [%s] [%5s] [%s:%d] [TIME: %lf] %s\r\n", logger.t_str, logger_get_thread_name(), get_level(level), filename, line, tdiff, logger.strbuf);
    } else {
        fprintf(logger.output, "[%s] [%s] [%5s] [%s:%d] %s\r\n", logger.t_str, logger_get_thread_name(), get_level(level), filename, line, logger.strbuf);
    }

    free(tmp_file);

    // Clean error
    errno = 0;

    pthread_mutex_unlock(&(logger.mutex));

    return 0;
}
