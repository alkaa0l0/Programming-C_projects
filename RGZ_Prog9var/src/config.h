#ifndef CONFIG_H
#define CONFIG_H

/* Параметры программы из конфигурационного файла и/или командной строки. */
typedef struct {
    char input[512];        /* путь к исходному тексту   */
    char dictionary[512];   /* путь к словарю            */
    char output[512];       /* путь к результату         */
    char log_file[512];     /* путь к лог-файлу          */
    char log_level[16];     /* INFO / WARNING / ERROR    */
    int  preserve_case;     /* сохранять регистр         */
    int  show_alternatives; /* показывать многозначность */
} Config;

/* Заполнить значениями по умолчанию. */
void config_defaults(Config *cfg);

/*
 * Прочитать конфигурацию из файла формата KEY=VALUE.
 * Строки с '#' и пустые пропускаются, неизвестные ключи -> предупреждение.
 * Возвращает 0 при успехе, -1 если файл не открылся.
 */
int config_load(Config *cfg, const char *path);

#endif /* CONFIG_H */
