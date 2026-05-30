#include "logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

static FILE *g_log = NULL;
static LogLevel g_min = LOG_INFO;

static const char *level_name(LogLevel level) {
    if (level == LOG_ERROR)   return "ERROR";
    if (level == LOG_WARNING) return "WARNING";
    return "INFO";
}

int log_init(const char *path, LogLevel min_level) {
    g_min = min_level;
    if (!path || path[0] == '\0') { g_log = NULL; return 0; } 
    g_log = fopen(path, "a");
    return g_log ? 0 : -1;
}

LogLevel log_level_from_string(const char *s) {
    if (s && strcmp(s, "ERROR") == 0)   return LOG_ERROR;
    if (s && strcmp(s, "WARNING") == 0) return LOG_WARNING;
    return LOG_INFO;
}

void log_msg(LogLevel level, const char *fmt, ...) {
    if (!g_log || level < g_min) return;

    time_t now = time(NULL);
    char ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(g_log, "[%s] [%s] ", ts, level_name(level));

    va_list ap;
    va_start(ap, fmt);
    vfprintf(g_log, fmt, ap);
    va_end(ap);

    fputc('\n', g_log);
    fflush(g_log);
}

void log_close(void) {
    if (g_log) { fclose(g_log); g_log = NULL; }
}
