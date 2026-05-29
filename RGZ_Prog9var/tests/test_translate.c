/*
 * Минималистичный набор UNIT-тестов без внешних зависимостей.
 * Запуск:  test_translate [группа]
 * группы:  utf8 | dict | translate | config | all (по умолчанию)
 * Возвращает 0, если все тесты пройдены, иначе 1.
 */
#include "utf8util.h"
#include "dictionary.h"
#include "translator.h"
#include "config.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failed = 0;
static int g_total  = 0;

#define CHECK(cond, msg) do {                                  \
    g_total++;                                                 \
    if (cond) {                                                \
        printf("  [ OK ] %s\n", msg);                          \
    } else {                                                   \
        printf("  [FAIL] %s  (%s:%d)\n", msg, __FILE__, __LINE__); \
        g_failed++;                                            \
    }                                                          \
} while (0)

#define CHECK_STR(got, exp, msg) do {                          \
    g_total++;                                                 \
    if (strcmp((got), (exp)) == 0) {                           \
        printf("  [ OK ] %s\n", msg);                          \
    } else {                                                   \
        printf("  [FAIL] %s  ожидалось='%s' получено='%s'\n",  \
               msg, (exp), (got));                             \
        g_failed++;                                            \
    }                                                          \
} while (0)

/* --- Тесты UTF-8 / регистр --- */
static void test_utf8(void) {
    printf("== UTF-8 / регистр ==\n");

    CHECK(u8_detect_case("привет", strlen("привет")) == CASE_OTHER, "строчное -> CASE_OTHER");
    CHECK(u8_detect_case("Привет", strlen("Привет")) == CASE_TITLE, "Заглавная -> CASE_TITLE");
    CHECK(u8_detect_case("ПРИВЕТ", strlen("ПРИВЕТ")) == CASE_UPPER, "ВЕРХНИЙ -> CASE_UPPER");
    CHECK(u8_detect_case("hello",  strlen("hello"))  == CASE_OTHER, "latin lower -> CASE_OTHER");
    CHECK(u8_detect_case("Hello",  strlen("Hello"))  == CASE_TITLE, "Latin Title -> CASE_TITLE");
    CHECK(u8_detect_case("HELLO",  strlen("HELLO"))  == CASE_UPPER, "LATIN UPPER -> CASE_UPPER");

    char *a = u8_apply_case("hello", CASE_TITLE);
    CHECK_STR(a, "Hello", "apply TITLE к 'hello'");
    free(a);

    char *b = u8_apply_case("hello", CASE_UPPER);
    CHECK_STR(b, "HELLO", "apply UPPER к 'hello'");
    free(b);

    char *c = u8_apply_case("мир", CASE_UPPER);
    CHECK_STR(c, "МИР", "apply UPPER к 'мир' (кириллица)");
    free(c);

    char *d = u8_apply_case("мир", CASE_TITLE);
    CHECK_STR(d, "Мир", "apply TITLE к 'мир' (кириллица)");
    free(d);

    char *e = u8_to_lower("ПрИвЕт");
    CHECK_STR(e, "привет", "to_lower 'ПрИвЕт'");
    free(e);
}

/* --- Тесты словаря --- */
static void test_dict(void) {
    printf("== Словарь ==\n");

    /* временный файл словаря */
    const char *path = "test_dict_tmp.txt";
    FILE *f = fopen(path, "w");
    fprintf(f,
        "# комментарий\n"
        "привет = hello\n"
        "мир = world | peace\n"
        "  кот  =  cat  \n"
        "битая строка без знака\n"
        "\n");
    fclose(f);

    Dictionary *d = dict_create();
    size_t bad = 0;
    long n = dict_load(d, path, &bad);

    CHECK(n == 3, "загружено 3 слова");
    CHECK(bad == 1, "одна некорректная строка");

    const DictEntry *e = dict_lookup(d, "привет");
    CHECK(e != NULL, "найдено 'привет'");
    if (e) CHECK_STR(e->translations[0], "hello", "перевод 'привет' = hello");

    const DictEntry *m = dict_lookup(d, "МИР");      /* регистр не важен */
    CHECK(m != NULL, "найдено 'МИР' без учёта регистра");
    if (m) {
        CHECK(m->trans_count == 2, "многозначное 'мир' -> 2 значения");
        CHECK_STR(m->translations[0], "world", "мир[0]=world");
        CHECK_STR(m->translations[1], "peace", "мир[1]=peace");
    }

    const DictEntry *k = dict_lookup(d, "кот");
    CHECK(k != NULL, "пробелы вокруг ключа/значения обрезаны");
    if (k) CHECK_STR(k->translations[0], "cat", "перевод 'кот' = cat");

    CHECK(dict_lookup(d, "нетслова") == NULL, "несуществующее слово -> NULL");

    dict_free(d);
    remove(path);
}

/* --- Тесты перевода (end-to-end) --- */
static void test_translate(void) {
    printf("== Перевод текста ==\n");

    const char *dpath = "test_tr_dict.txt";
    const char *ipath = "test_tr_in.txt";
    const char *opath = "test_tr_out.txt";

    FILE *f = fopen(dpath, "w");
    fprintf(f, "привет = hello\nмир = world|peace\nкот = cat\n");
    fclose(f);

    /* проверяем: регистр, пунктуация, перенос строки, многозначность */
    f = fopen(ipath, "w");
    fprintf(f, "Привет, МИР!\n  кот\n");
    fclose(f);

    Dictionary *d = dict_create();
    dict_load(d, dpath, NULL);

    TranslatorOptions opts = {1, 0, 1}; /* сохранять регистр, без альтернатив, 1 поток */
    TranslateStats st;
    int rc = translate_file(ipath, opath, d, &opts, &st);
    CHECK(rc == 0, "translate_file завершился успешно");

    /* читаем результат */
    char buf[512] = {0};
    f = fopen(opath, "rb");
    size_t rd = fread(buf, 1, sizeof(buf) - 1, f);
    buf[rd] = '\0';
    fclose(f);

    CHECK_STR(buf, "Hello, WORLD!\n  cat\n",
              "сохранены регистр, пунктуация и форматирование");
    CHECK(st.total_words == 3, "посчитано 3 слова");
    CHECK(st.translated == 3, "переведено 3 слова");

    /* проверка режима альтернатив */
    TranslatorOptions opts2 = {0, 1, 2}; /* без регистра, альтернативы, 2 потока */
    f = fopen(ipath, "w");
    fprintf(f, "мир\n");
    fclose(f);
    translate_file(ipath, opath, d, &opts2, &st);
    f = fopen(opath, "rb");
    rd = fread(buf, 1, sizeof(buf) - 1, f);
    buf[rd] = '\0';
    fclose(f);
    CHECK_STR(buf, "world(peace)\n", "режим многозначности выводит альтернативы");

    dict_free(d);
    remove(dpath); remove(ipath); remove(opath);
}

/* --- Тесты конфигурации --- */
static void test_config(void) {
    printf("== Конфигурация ==\n");

    const char *path = "test_cfg_tmp.ini";
    FILE *f = fopen(path, "w");
    fprintf(f,
        "# конфиг\n"
        "input = in.txt\n"
        "dictionary = dict.txt\n"
        "output = out.txt\n"
        "threads = 4\n"
        "preserve_case = 0\n"
        "unknown_key = 123\n");
    fclose(f);

    Config cfg;
    config_defaults(&cfg);
    int rc = config_load(&cfg, path);

    CHECK(rc == 0, "config_load успешен");
    CHECK_STR(cfg.input, "in.txt", "input прочитан");
    CHECK_STR(cfg.dictionary, "dict.txt", "dictionary прочитан");
    CHECK(cfg.threads == 4, "threads = 4");
    CHECK(cfg.preserve_case == 0, "preserve_case = 0");

    remove(path);
}

int main(int argc, char *argv[]) {
    /* логирование в отдельный файл, чтобы не засорять вывод тестов */
    log_init("test.log", LOG_ERROR);

    const char *group = (argc > 1) ? argv[1] : "all";

    if (!strcmp(group, "utf8")      || !strcmp(group, "all")) test_utf8();
    if (!strcmp(group, "dict")      || !strcmp(group, "all")) test_dict();
    if (!strcmp(group, "translate") || !strcmp(group, "all")) test_translate();
    if (!strcmp(group, "config")    || !strcmp(group, "all")) test_config();

    log_close();
    remove("test.log");

    printf("\nИтог: пройдено %d из %d\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
