/*
 * UNIT-тесты без внешних библиотек.
 */
#include "translator.h"
#include "dictionary.h"
#include "config.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failed = 0, g_total = 0;

#define CHECK(cond, msg) do {                              \
    g_total++;                                             \
    if (cond) printf("  [ OK ] %s\n", msg);                \
    else { printf("  [FAIL] %s (%s:%d)\n", msg, __FILE__, __LINE__); g_failed++; } \
} while (0)

#define CHECK_STR(got, exp, msg) do {                      \
    g_total++;                                             \
    if (strcmp((got), (exp)) == 0) printf("  [ OK ] %s\n", msg); \
    else { printf("  [FAIL] %s  ожидалось='%s' получено='%s'\n", msg, exp, got); g_failed++; } \
} while (0)

/* --- Регистр --- */
static void test_case(void) {
    printf("== Регистр ==\n");
    CHECK(detect_case("привет", 12) == CASE_LOWER, "строчное -> LOWER");
    CHECK(detect_case("Привет", 13) == CASE_TITLE, "Заглавная -> TITLE");
    CHECK(detect_case("ПРИВЕТ", 12) == CASE_UPPER, "ВЕРХНИЙ -> UPPER");
    CHECK(detect_case("Hello", 5)   == CASE_TITLE, "Latin Title");
    CHECK(detect_case("HELLO", 5)   == CASE_UPPER, "LATIN UPPER");

    char *a = apply_case("hello", CASE_TITLE);  CHECK_STR(a, "Hello", "TITLE к hello");   free(a);
    char *b = apply_case("hello", CASE_UPPER);  CHECK_STR(b, "HELLO", "UPPER к hello");   free(b);
    char *c = apply_case("мир", CASE_UPPER);    CHECK_STR(c, "МИР", "UPPER к мир");       free(c);
    char *d = apply_case("мир", CASE_TITLE);    CHECK_STR(d, "Мир", "TITLE к мир");       free(d);
    char *e = str_to_lower("ПрИвЕт");           CHECK_STR(e, "привет", "to_lower");       free(e);
}

/* --- Словарь --- */
static void test_dict(void) {
    printf("== Словарь ==\n");
    const char *path = "test_dict_tmp.txt";
    FILE *f = fopen(path, "w");
    fprintf(f, "# comment\nпривет = hello\nмир = world | peace\n  кот = cat \nбитая строка\n\n");
    fclose(f);

    Dictionary *d = dict_create();
    int bad = 0;
    int n = dict_load(d, path, &bad);
    CHECK(n == 3, "загружено 3 слова");
    CHECK(bad == 1, "одна некорректная строка");

    const Entry *e = dict_find(d, "ПРИВЕТ");     
    CHECK(e != NULL, "найдено 'ПРИВЕТ' без учёта регистра");
    if (e) CHECK_STR(e->translations[0], "hello", "перевод hello");

    const Entry *m = dict_find(d, "мир");
    CHECK(m && m->trans_count == 2, "многозначное 'мир' -> 2 значения");
    if (m) CHECK_STR(m->translations[1], "peace", "мир[1]=peace");

    CHECK(dict_find(d, "несуществует") == NULL, "несуществующее -> NULL");

    dict_free(d);
    remove(path);
}

/* --- Перевод --- */
static void test_translate(void) {
    printf("== Перевод ==\n");
    const char *dp = "t_dict.txt", *ip = "t_in.txt", *op = "t_out.txt";

    FILE *f = fopen(dp, "w");
    fprintf(f, "привет = hello\nмир = world|peace\nкот = cat\n");
    fclose(f);
    f = fopen(ip, "w");
    fprintf(f, "Привет, МИР!\n  кот\n");
    fclose(f);

    Dictionary *d = dict_create();
    dict_load(d, dp, NULL);

    Options opt = {1, 0};   
    Stats st;
    int rc = translate_file(ip, op, d, &opt, &st);
    CHECK(rc == 0, "translate_file успешен");

    char buf[256] = {0};
    f = fopen(op, "rb");
    buf[fread(buf, 1, sizeof(buf) - 1, f)] = '\0';
    fclose(f);
    CHECK_STR(buf, "Hello, WORLD!\n  cat\n", "регистр + пунктуация + форматирование");
    CHECK(st.translated == 3, "переведено 3 слова");

    /* режим многозначности */
    Options opt2 = {0, 1};
    f = fopen(ip, "w"); fprintf(f, "мир\n"); fclose(f);
    translate_file(ip, op, d, &opt2, &st);
    f = fopen(op, "rb");
    buf[fread(buf, 1, sizeof(buf) - 1, f)] = '\0';
    fclose(f);
    CHECK_STR(buf, "world(peace)\n", "альтернативы многозначного слова");

    dict_free(d);
    remove(dp); remove(ip); remove(op);
}

/* --- Конфиг --- */
static void test_config(void) {
    printf("== Конфиг ==\n");
    const char *path = "t_cfg.ini";
    FILE *f = fopen(path, "w");
    fprintf(f, "# cfg\ninput = in.txt\ndictionary = d.txt\noutput = o.txt\npreserve_case = 0\nbad_key=1\n");
    fclose(f);

    Config cfg;
    config_defaults(&cfg);
    int rc = config_load(&cfg, path);
    CHECK(rc == 0, "config_load успешен");
    CHECK_STR(cfg.input, "in.txt", "input прочитан");
    CHECK(cfg.preserve_case == 0, "preserve_case = 0");

    remove(path);
}

int main(int argc, char *argv[]) {
    log_init("test.log", LOG_ERROR);
    const char *g = (argc > 1) ? argv[1] : "all";

    if (!strcmp(g, "case")      || !strcmp(g, "all")) test_case();
    if (!strcmp(g, "dict")      || !strcmp(g, "all")) test_dict();
    if (!strcmp(g, "translate") || !strcmp(g, "all")) test_translate();
    if (!strcmp(g, "config")    || !strcmp(g, "all")) test_config();

    log_close();
    remove("test.log");
    printf("\nИтог: пройдено %d из %d\n", g_total - g_failed, g_total);
    return g_failed ? 1 : 0;
}
