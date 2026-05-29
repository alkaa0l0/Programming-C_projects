#ifndef LOGGER_H
#define LOGGER_H

/* Уровни логирования (по возрастанию важности) */
typedef enum {
    LOG_INFO = 0,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

/*
 * Инициализировать логгер: открыть файл path и установить минимальный
 * уровень min_level (сообщения ниже уровня не записываются).
 * Возвращает 0 при успехе, -1 при ошибке открытия файла.
 */
int  log_init(const char *path, LogLevel min_level);

/* Преобразовать строку ("INFO"/"WARNING"/"ERROR") в уровень. */
LogLevel log_level_from_string(const char *s);

/* Записать сообщение с указанным уровнем (формат printf). */
void log_msg(LogLevel level, const char *fmt, ...);

/* Закрыть лог-файл и освободить ресурсы. */
void log_close(void);

#endif /* LOGGER_H */
