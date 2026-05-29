#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "dictionary.h"

/* Регистр слова */
typedef enum {
    CASE_LOWER,   /* строчное / смешанное — перевод не меняем */
    CASE_TITLE,   /* Первая буква заглавная (Привет)          */
    CASE_UPPER    /* ВСЕ ЗАГЛАВНЫЕ (ПРИВЕТ)                   */
} WordCase;

/* --- Функции работы с регистром (UTF-8: латиница и кириллица) --- */

/* Привести строку к нижнему регистру (нужно free). */
char *str_to_lower(const char *s);

/* Определить регистр слова длиной len байт. */
WordCase detect_case(const char *word, int len);

/* Применить регистр c к строке s (нужно free). */
char *apply_case(const char *s, WordCase c);

/* --- Перевод --- */

/* Параметры перевода */
typedef struct {
    int preserve_case;      /* 1 — сохранять регистр исходного слова          */
    int show_alternatives;  /* 1 — показывать все значения многозначного слова */
} Options;

/* Статистика по результату */
typedef struct {
    int total;          /* всего слов        */
    int translated;     /* переведено        */
    int untranslated;   /* без перевода       */
} Stats;

/*
 * Перевести текст из in в out по словарю d.
 * Форматирование (пробелы, переносы строк, знаки препинания) сохраняется.
 * Возвращает 0 при успехе, отрицательный код при ошибке.
 */
int translate_file(const char *in, const char *out,
                   const Dictionary *d, const Options *opt, Stats *st);

#endif /* TRANSLATOR_H */
