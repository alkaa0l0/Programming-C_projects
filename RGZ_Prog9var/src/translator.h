#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "dictionary.h"
#include <stddef.h>

/* Параметры перевода */
typedef struct {
    int    preserve_case;     /* 1 - сохранять регистр исходного слова            */
    int    show_alternatives; /* 1 - показывать все значения многозначного слова  */
    int    threads;           /* число рабочих потоков (>=1)                       */
} TranslatorOptions;

/* Статистика по результату перевода */
typedef struct {
    size_t total_words;       /* всего слов в тексте          */
    size_t translated;        /* переведено                   */
    size_t untranslated;      /* оставлено без перевода        */
} TranslateStats;

/*
 * Перевести текст из in_path, используя словарь dict, и записать в out_path.
 * Форматирование (пробелы, переносы строк, пунктуация) сохраняется.
 * stats (может быть NULL) - сюда пишется статистика.
 * Возвращает 0 при успехе, отрицательный код при ошибке.
 */
int translate_file(const char *in_path, const char *out_path,
                   const Dictionary *dict, const TranslatorOptions *opts,
                   TranslateStats *stats);

#endif /* TRANSLATOR_H */
