#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
    LOG_INFO = 0,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;


int log_init(const char *path, LogLevel min_level);

/* Преобразовать строку "INFO"/"WARNING"/"ERROR" в уровень. */
LogLevel log_level_from_string(const char *s);

void log_msg(LogLevel level, const char *fmt, ...);

void log_close(void);

#endif 
