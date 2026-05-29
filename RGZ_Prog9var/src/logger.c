#include "logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

static FILE *g_log_file = NULL;
static LogLevel g_min_level = LOG_INFO;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

static const char *level_name(LogLevel level) {
    switch (level) {
        case LOG_ERROR:   return "ERROR";
        case LOG_WARNING: return "WARNING";
        default:          return "INFO";
    }
}

int log_init(const char *path, LogLevel min_level) {
    g_min_level = min_level;
    if (path == NULL || path[0] == '\0') {
        g_log_file = NULL;          /* логирование отключено */
        return 0;
    }
    g_log_file = fopen(path, "a");
    if (!g_log_file) return -1;
    return 0;
}

LogLevel log_level_from_string(const char *s) {
    if (s == NULL) return LOG_INFO;
    if (strcmp(s, "ERROR") == 0)   return LOG_ERROR;
    if (strcmp(s, "WARNING") == 0) return LOG_WARNING;
    return LOG_INFO;
}

void log_msg(LogLevel level, const char *fmt, ...) {
    if (g_log_file == NULL) return;
    if (level < g_min_level) return;

    pthread_mutex_lock(&g_lock);

    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);
    char ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &tm_now);

    fprintf(g_log_file, "[%s] [%s] ", ts, level_name(level));

    va_list ap;
    va_start(ap, fmt);
    vfprintf(g_log_file, fmt, ap);
    va_end(ap);

    fputc('\n', g_log_file);
    fflush(g_log_file);

    pthread_mutex_unlock(&g_lock);
}

void log_close(void) {
    pthread_mutex_lock(&g_lock);
    if (g_log_file) {
        fclose(g_log_file);
        g_log_file = NULL;
    }
    pthread_mutex_unlock(&g_lock);
}
