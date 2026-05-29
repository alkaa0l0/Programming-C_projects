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
        "  --threads N        число потоков (>=1)\n"
        "  --no-case          не сохранять регистр исходного слова\n"
        "  --alternatives     показывать все значения многозначных слов\n"
        "  --help             эта справка\n",
        prog, prog);
}

/* Проверить, что файл существует и доступен для чтения. */
static int file_readable(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    fclose(f);
    return 1;
}

/* Проверить, что по пути можно создать/открыть файл для записи. */
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
    const char *positional[3] = {NULL, NULL, NULL};
    int pos_count = 0;

    /* --- Разбор аргументов командной строки --- */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--config") == 0) {
            if (i + 1 >= argc) { fprintf(stderr, "Ошибка: --config требует имя файла\n"); return 1; }
            config_path = argv[++i];
        } else if (strcmp(argv[i], "--log-level") == 0) {
            if (i + 1 >= argc) { fprintf(stderr, "Ошибка: --log-level требует значение\n"); return 1; }
            strncpy(cfg.log_level, argv[++i], sizeof(cfg.log_level) - 1);
        } else if (strcmp(argv[i], "--threads") == 0) {
            if (i + 1 >= argc) { fprintf(stderr, "Ошибка: --threads требует число\n"); return 1; }
            cfg.threads = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--no-case") == 0) {
            cfg.preserve_case = 0;
        } else if (strcmp(argv[i], "--alternatives") == 0) {
            cfg.show_alternatives = 1;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Ошибка: неизвестная опция '%s'\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        } else {
            if (pos_count < 3) positional[pos_count++] = argv[i];
            else { fprintf(stderr, "Ошибка: лишний аргумент '%s'\n", argv[i]); return 1; }
        }
    }

    /* Конфиг читаем до применения позиционных аргументов, чтобы CLI имел приоритет. */
    if (config_path) {
        if (config_load(&cfg, config_path) != 0) {
            fprintf(stderr, "Ошибка: не удалось прочитать конфигурацию '%s'\n", config_path);
            return 1;
        }
    }
    /* Позиционные аргументы переопределяют конфиг. */
    if (pos_count >= 1) strncpy(cfg.input,      positional[0], sizeof(cfg.input) - 1);
    if (pos_count >= 2) strncpy(cfg.dictionary, positional[1], sizeof(cfg.dictionary) - 1);
    if (pos_count >= 3) strncpy(cfg.output,     positional[2], sizeof(cfg.output) - 1);

    /* --- Инициализация логгера --- */
    LogLevel lvl = log_level_from_string(cfg.log_level);
    if (log_init(cfg.log_file, lvl) != 0) {
        fprintf(stderr, "Предупреждение: не удалось открыть лог-файл '%s', "
                        "логирование отключено\n", cfg.log_file);
    }
    log_msg(LOG_INFO, "Запуск translate");

    /* --- Валидация входных данных --- */
    int ok = 1;
    if (cfg.input[0] == '\0' || cfg.dictionary[0] == '\0' || cfg.output[0] == '\0') {
        fprintf(stderr, "Ошибка: не заданы все три файла (вход, словарь, выход).\n");
        log_msg(LOG_ERROR, "Не заданы обязательные пути к файлам");
        print_usage(argv[0]);
        ok = 0;
    }
    if (ok && !file_readable(cfg.input)) {
        fprintf(stderr, "Ошибка: входной файл '%s' не существует или недоступен для чтения.\n", cfg.input);
        log_msg(LOG_ERROR, "Входной файл недоступен: %s", cfg.input);
        ok = 0;
    }
    if (ok && !file_readable(cfg.dictionary)) {
        fprintf(stderr, "Ошибка: файл словаря '%s' не существует или недоступен для чтения.\n", cfg.dictionary);
        log_msg(LOG_ERROR, "Файл словаря недоступен: %s", cfg.dictionary);
        ok = 0;
    }
    if (ok && !file_writable(cfg.output)) {
        fprintf(stderr, "Ошибка: невозможно создать выходной файл '%s'.\n", cfg.output);
        log_msg(LOG_ERROR, "Выходной файл недоступен для записи: %s", cfg.output);
        ok = 0;
    }
    if (ok && cfg.threads < 1) {
        fprintf(stderr, "Ошибка: число потоков должно быть >= 1 (указано %d).\n", cfg.threads);
        log_msg(LOG_ERROR, "Некорректное число потоков: %d", cfg.threads);
        ok = 0;
    }
    if (!ok) { log_close(); return 1; }

    /* --- Загрузка словаря --- */
    Dictionary *dict = dict_create();
    if (!dict) {
        fprintf(stderr, "Ошибка: нехватка памяти при создании словаря.\n");
        log_close();
        return 2;
    }
    size_t bad = 0;
    long loaded = dict_load(dict, cfg.dictionary, &bad);
    if (loaded < 0) {
        fprintf(stderr, "Ошибка: не удалось загрузить словарь '%s'.\n", cfg.dictionary);
        dict_free(dict);
        log_close();
        return 2;
    }
    if (loaded == 0) {
        fprintf(stderr, "Предупреждение: словарь пуст — текст будет скопирован без изменений.\n");
        log_msg(LOG_WARNING, "Словарь пуст");
    }
    if (bad > 0) {
        fprintf(stderr, "Предупреждение: пропущено некорректных строк словаря: %zu\n", bad);
    }

    /* --- Перевод --- */
    TranslatorOptions opts;
    opts.preserve_case = cfg.preserve_case;
    opts.show_alternatives = cfg.show_alternatives;
    opts.threads = cfg.threads;

    TranslateStats stats;
    int rc = translate_file(cfg.input, cfg.output, dict, &opts, &stats);
    if (rc != 0) {
        fprintf(stderr, "Ошибка: перевод не выполнен (код %d).\n", rc);
        dict_free(dict);
        log_close();
        return 3;
    }

    printf("Перевод завершён.\n");
    printf("  Словарь:        %ld слов\n", loaded);
    printf("  Всего слов:     %zu\n", stats.total_words);
    printf("  Переведено:     %zu\n", stats.translated);
    printf("  Без перевода:   %zu\n", stats.untranslated);
    printf("  Результат:      %s\n", cfg.output);

    dict_free(dict);
    log_close();
    return 0;
}
