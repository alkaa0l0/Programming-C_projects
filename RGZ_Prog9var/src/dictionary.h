#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <stddef.h>

/*
 * Одна статья словаря: исходное слово (в нижнем регистре) и список переводов.
 * Несколько переводов поддерживают многозначные слова.
 */
typedef struct DictEntry {
    char  *source;          /* ключ: слово-оригинал, нормализованный в нижний регистр */
    char **translations;    /* массив строк-переводов                                  */
    size_t trans_count;     /* количество переводов                                    */
    struct DictEntry *next; /* следующая статья в цепочке хеш-таблицы                   */
} DictEntry;

/* Словарь на основе хеш-таблицы с разрешением коллизий цепочками. */
typedef struct {
    DictEntry **buckets;    /* массив указателей на цепочки */
    size_t bucket_count;    /* размер таблицы               */
    size_t size;            /* число уникальных слов        */
} Dictionary;

/* Создать пустой словарь. Возвращает NULL при нехватке памяти. */
Dictionary *dict_create(void);

/*
 * Загрузить словарь из файла.
 * Формат строки:  слово = перевод
 *                 слово = перевод1 | перевод2 | перевод3   (многозначное)
 * Пустые строки и строки, начинающиеся с '#', игнорируются.
 * bad_lines (может быть NULL) - сюда пишется число некорректных строк.
 * Возвращает число загруженных статей, либо -1 при ошибке открытия файла.
 */
long dict_load(Dictionary *dict, const char *path, size_t *bad_lines);

/*
 * Найти переводы слова (поиск без учёта регистра).
 * Возвращает указатель на статью или NULL, если слово не найдено.
 */
const DictEntry *dict_lookup(const Dictionary *dict, const char *word);

/* Освободить всю память словаря. */
void dict_free(Dictionary *dict);

#endif /* DICTIONARY_H */
