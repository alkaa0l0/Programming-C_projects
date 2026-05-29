#include "config.h"
#include "logger.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char *trim(char *s) {
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;
    if (*s == '\0') return s;
    char *end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n'))
        *end-- = '\0';
    return s;
}

void config_defaults(Config *cfg) {
    cfg->input[0] = '\0';
    cfg->dictionary[0] = '\0';
    cfg->output[0] = '\0';
    strcpy(cfg->log_file, "translate.log");
    strcpy(cfg->log_level, "INFO");
    cfg->preserve_case = 1;
    cfg->show_alternatives = 0;
    cfg->threads = 1;
}

static void set_str(char *dst, size_t cap, const char *val) {
    strncpy(dst, val, cap - 1);
    dst[cap - 1] = '\0';
}

int config_load(Config *cfg, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        log_msg(LOG_ERROR, "Не удалось открыть конфигурационный файл: %s", path);
        return -1;
    }
    char line[2048];
    size_t lineno = 0;
    while (fgets(line, sizeof(line), f)) {
        lineno++;
        char *p = trim(line);
        if (*p == '\0' || *p == '#') continue;

        char *eq = strchr(p, '=');
        if (!eq) {
            log_msg(LOG_WARNING, "Конфиг, строка %zu без '=': %s", lineno, p);
            continue;
        }
        *eq = '\0';
        char *key = trim(p);
        char *val = trim(eq + 1);

        if      (strcmp(key, "input") == 0)             set_str(cfg->input, sizeof(cfg->input), val);
        else if (strcmp(key, "dictionary") == 0)        set_str(cfg->dictionary, sizeof(cfg->dictionary), val);
        else if (strcmp(key, "output") == 0)            set_str(cfg->output, sizeof(cfg->output), val);
        else if (strcmp(key, "log_file") == 0)          set_str(cfg->log_file, sizeof(cfg->log_file), val);
        else if (strcmp(key, "log_level") == 0)         set_str(cfg->log_level, sizeof(cfg->log_level), val);
        else if (strcmp(key, "preserve_case") == 0)     cfg->preserve_case = atoi(val);
        else if (strcmp(key, "show_alternatives") == 0) cfg->show_alternatives = atoi(val);
        else if (strcmp(key, "threads") == 0)           cfg->threads = atoi(val);
        else log_msg(LOG_WARNING, "Конфиг, неизвестный ключ '%s' (строка %zu)", key, lineno);
    }
    fclose(f);
    return 0;
}
