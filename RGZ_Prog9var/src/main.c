#include "config.h"
#include "dictionary.h"
#include "translator.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char *prog) {
    fprintf(stderr,
        "Использование:\n"
        "  %s <input.txt> <dictionary.txt> <output.txt> [опции]\n"
        "  %s --config <config.ini> [опции]\n\n"
        "Опции:\n"
        "  --config FILE      файл конфигурации (KEY=VALUE)\n"
        "  --log-level LEVEL  уровень лога: INFO | WARNING | ERROR\n"
        "  --no-case          не сохранять регистр исходного слова\n"
        "  --alternatives     показывать все значения многозначных слов\n"
        "  --help             эта справка\n",
        prog, prog);
}

/* Можно ли прочитать файл? */
static int file_readable(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    fclose(f);
    return 1;
}

/* Можно ли создать/записать файл? */
static int file_writable(const char *path) {
    FILE *f = fopen(path, "a");
    if (!f) return 0;
    fclose(f);
    return 1;
}

int main(int argc, char *argv[]) {
    Config cfg;
    config_defaults(&cfg);

    const char *config_path = NULL;
    const char *pos[3] = {NULL, NULL, NULL};
    int pos_count = 0;

    /* --- Разбор аргументов командной строки --- */
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            print_usage(argv[0]);
            return 0;
        } else if (!strcmp(argv[i], "--config")) {
            if (i + 1 >= argc) { fprintf(stderr, "Ошибка: --config требует имя файла\n"); return 1; }
            config_path = argv[++i];
        } else if (!strcmp(argv[i], "--log-level")) {
            if (i + 1 >= argc) { fprintf(stderr, "Ошибка: --log-level требует значение\n"); return 1; }
            strncpy(cfg.log_level, argv[++i], sizeof(cfg.log_level) - 1);
        } else if (!strcmp(argv[i], "--no-case")) {
            cfg.preserve_case = 0;
        } else if (!strcmp(argv[i], "--alternatives")) {
            cfg.show_alternatives = 1;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Ошибка: неизвестная опция '%s'\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        } else {
            if (pos_count < 3) pos[pos_count++] = argv[i];
            else { fprintf(stderr, "Ошибка: лишний аргумент '%s'\n", argv[i]); return 1; }
        }
    }

    /* Сначала конфиг, затем позиционные аргументы (у них приоритет). */
    if (config_path && config_load(&cfg, config_path) != 0) {
        fprintf(stderr, "Ошибка: не удалось прочитать конфигурацию '%s'\n", config_path);
        return 1;
    }
    if (pos_count >= 1) strncpy(cfg.input,      pos[0], sizeof(cfg.input) - 1);
    if (pos_count >= 2) strncpy(cfg.dictionary, pos[1], sizeof(cfg.dictionary) - 1);
    if (pos_count >= 3) strncpy(cfg.output,     pos[2], sizeof(cfg.output) - 1);

    /* --- Логгер --- */
    LogLevel lvl = log_level_from_string(cfg.log_level);
    if (log_init(cfg.log_file, lvl) != 0)
        fprintf(stderr, "Предупреждение: не удалось открыть лог-файл '%s'\n", cfg.log_file);
    log_msg(LOG_INFO, "Запуск translate");

    /* --- Проверка входных данных --- */
    int ok = 1;
    if (cfg.input[0] == '\0' || cfg.dictionary[0] == '\0' || cfg.output[0] == '\0') {
        fprintf(stderr, "Ошибка: не заданы все три файла (вход, словарь, выход).\n");
        log_msg(LOG_ERROR, "Не заданы обязательные файлы");
        print_usage(argv[0]);
        ok = 0;
    }
    if (ok && !file_readable(cfg.input)) {
        fprintf(stderr, "Ошибка: входной файл '%s' не найден или недоступен.\n", cfg.input);
        log_msg(LOG_ERROR, "Входной файл недоступен: %s", cfg.input);
        ok = 0;
    }
    if (ok && !file_readable(cfg.dictionary)) {
        fprintf(stderr, "Ошибка: файл словаря '%s' не найден или недоступен.\n", cfg.dictionary);
        log_msg(LOG_ERROR, "Файл словаря недоступен: %s", cfg.dictionary);
        ok = 0;
    }
    if (ok && !file_writable(cfg.output)) {
        fprintf(stderr, "Ошибка: невозможно создать выходной файл '%s'.\n", cfg.output);
        log_msg(LOG_ERROR, "Выходной файл недоступен для записи: %s", cfg.output);
        ok = 0;
    }
    if (!ok) { log_close(); return 1; }

    /* --- Загрузка словаря --- */
    Dictionary *dict = dict_create();
    if (!dict) { fprintf(stderr, "Ошибка: нехватка памяти.\n"); log_close(); return 2; }

    int bad = 0;
    int loaded = dict_load(dict, cfg.dictionary, &bad);
    if (loaded < 0) {
        fprintf(stderr, "Ошибка: не удалось загрузить словарь '%s'.\n", cfg.dictionary);
        dict_free(dict);
        log_close();
        return 2;
    }
    if (loaded == 0)
        fprintf(stderr, "Предупреждение: словарь пуст — текст будет скопирован без изменений.\n");
    if (bad > 0)
        fprintf(stderr, "Предупреждение: пропущено некорректных строк словаря: %d\n", bad);

    /* --- Перевод --- */
    Options opt = { cfg.preserve_case, cfg.show_alternatives };
    Stats st;
    int rc = translate_file(cfg.input, cfg.output, dict, &opt, &st);
    if (rc != 0) {
        fprintf(stderr, "Ошибка: перевод не выполнен (код %d).\n", rc);
        dict_free(dict);
        log_close();
        return 3;
    }

    printf("Перевод завершён.\n");
    printf("  Словарь:      %d слов\n", loaded);
    printf("  Всего слов:   %d\n", st.total);
    printf("  Переведено:   %d\n", st.translated);
    printf("  Без перевода: %d\n", st.untranslated);
    printf("  Результат:    %s\n", cfg.output);

    dict_free(dict);
    log_close();
    return 0;
}
