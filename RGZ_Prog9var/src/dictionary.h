#ifndef DICTIONARY_H
#define DICTIONARY_H

#define MAX_TRANSLATIONS 8   /* максимум значений у одного слова (многозначность) */

/* Одна статья словаря: слово-оригинал и его переводы. Это структура (struct). */
typedef struct {
    char *source;                          /* слово в нижнем регистре              */
    char *translations[MAX_TRANSLATIONS];  /* переводы (для многозначных слов)     */
    int   trans_count;                     /* сколько всего переводов              */
} Entry;

/* Словарь — динамический массив статей. */
typedef struct {
    Entry *entries;    /* массив статей            */
    int    count;      /* сколько слов в словаре   */
    int    capacity;   /* сколько памяти выделено  */
} Dictionary;

/* Создать пустой словарь. Возвращает NULL при нехватке памяти. */
Dictionary *dict_create(void);

/*
 * Загрузить словарь из файла. Формат строки:
 *     слово = перевод
 *     слово = перевод1 | перевод2        (многозначное слово)
 * Пустые строки и строки с '#' пропускаются.
 * bad_lines (может быть NULL) — сюда пишется число некорректных строк.
 * Возвращает число загруженных слов, либо -1 при ошибке открытия файла.
 */
int dict_load(Dictionary *d, const char *path, int *bad_lines);

/* Найти слово (без учёта регистра). Возвращает статью или NULL. */
const Entry *dict_find(const Dictionary *d, const char *word);

/* Освободить память словаря. */
void dict_free(Dictionary *d);

#endif /* DICTIONARY_H */
