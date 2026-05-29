#include "dictionary.h"
#include "utf8util.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_BUCKETS 1024

/* Хеш-функция djb2 */
static unsigned long hash_str(const char *s) {
    unsigned long h = 5381;
    int c;
    while ((c = (unsigned char)*s++))
        h = ((h << 5) + h) + (unsigned long)c;
    return h;
}

/* Обрезать пробелы по краям строки (на месте). Возвращает указатель на начало. */
static char *trim(char *s) {
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;
    if (*s == '\0') return s;
    char *end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n'))
        *end-- = '\0';
    return s;
}

Dictionary *dict_create(void) {
    Dictionary *d = (Dictionary *)malloc(sizeof(Dictionary));
    if (!d) return NULL;
    d->bucket_count = INITIAL_BUCKETS;
    d->size = 0;
    d->buckets = (DictEntry **)calloc(d->bucket_count, sizeof(DictEntry *));
    if (!d->buckets) {
        free(d);
        return NULL;
    }
    return d;
}

/* Добавить перевод к существующей статье. */
static int entry_add_translation(DictEntry *e, const char *tr) {
    char **tmp = (char **)realloc(e->translations,
                                  (e->trans_count + 1) * sizeof(char *));
    if (!tmp) return -1;
    e->translations = tmp;
    e->translations[e->trans_count] = strdup(tr);
    if (!e->translations[e->trans_count]) return -1;
    e->trans_count++;
    return 0;
}

/* Найти изменяемую статью (внутренняя). */
static DictEntry *find_entry(Dictionary *d, const char *key) {
    unsigned long idx = hash_str(key) % d->bucket_count;
    DictEntry *e = d->buckets[idx];
    while (e) {
        if (strcmp(e->source, key) == 0) return e;
        e = e->next;
    }
    return NULL;
}

/* Добавить пару (source -> translation). source уже в нижнем регистре. */
static int dict_put(Dictionary *d, const char *source, const char *translation) {
    DictEntry *e = find_entry(d, source);
    if (e) {                                  /* слово уже есть -> доп. значение */
        return entry_add_translation(e, translation);
    }
    e = (DictEntry *)malloc(sizeof(DictEntry));
    if (!e) return -1;
    e->source = strdup(source);
    e->translations = NULL;
    e->trans_count = 0;
    e->next = NULL;
    if (!e->source || entry_add_translation(e, translation) != 0) {
        free(e->source);
        free(e);
        return -1;
    }
    unsigned long idx = hash_str(source) % d->bucket_count;
    e->next = d->buckets[idx];
    d->buckets[idx] = e;
    d->size++;
    return 0;
}

long dict_load(Dictionary *dict, const char *path, size_t *bad_lines) {
    FILE *f = fopen(path, "r");
    if (!f) {
        log_msg(LOG_ERROR, "Не удалось открыть файл словаря: %s", path);
        return -1;
    }

    char line[4096];
    long loaded = 0;
    size_t bad = 0, lineno = 0;

    while (fgets(line, sizeof(line), f)) {
        lineno++;
        char *p = trim(line);
        if (*p == '\0' || *p == '#') continue;       /* пустая строка / комментарий */

        char *eq = strchr(p, '=');
        if (!eq) {
            log_msg(LOG_WARNING, "Строка %zu словаря без '=', пропущена: %s",
                    lineno, p);
            bad++;
            continue;
        }
        *eq = '\0';
        char *key_raw = trim(p);
        char *val_raw = trim(eq + 1);
        if (*key_raw == '\0' || *val_raw == '\0') {
            log_msg(LOG_WARNING, "Строка %zu словаря с пустым ключом/значением",
                    lineno);
            bad++;
            continue;
        }

        char *key = u8_to_lower(key_raw);            /* нормализуем ключ */
        if (!key) { fclose(f); return -1; }

        /* Разбираем значения, разделённые '|' (многозначные слова). */
        int added = 0;
        char *save = val_raw;
        char *tok;
        while ((tok = strsep(&save, "|")) != NULL) {
            char *t = trim(tok);
            if (*t == '\0') continue;
            if (dict_put(dict, key, t) != 0) {
                log_msg(LOG_ERROR, "Нехватка памяти при загрузке словаря");
                free(key);
                fclose(f);
                return -1;
            }
            added++;
        }
        free(key);
        if (added > 0) loaded++; else bad++;
    }

    fclose(f);
    if (bad_lines) *bad_lines = bad;
    log_msg(LOG_INFO, "Словарь загружен: %ld слов, %zu некорректных строк",
            loaded, bad);
    return loaded;
}

const DictEntry *dict_lookup(const Dictionary *dict, const char *word) {
    if (!dict || !word) return NULL;
    char *key = u8_to_lower(word);
    if (!key) return NULL;
    unsigned long idx = hash_str(key) % dict->bucket_count;
    DictEntry *e = dict->buckets[idx];
    while (e) {
        if (strcmp(e->source, key) == 0) {
            free(key);
            return e;
        }
        e = e->next;
    }
    free(key);
    return NULL;
}

void dict_free(Dictionary *dict) {
    if (!dict) return;
    for (size_t i = 0; i < dict->bucket_count; i++) {
        DictEntry *e = dict->buckets[i];
        while (e) {
            DictEntry *next = e->next;
            for (size_t j = 0; j < e->trans_count; j++)
                free(e->translations[j]);
            free(e->translations);
            free(e->source);
            free(e);
            e = next;
        }
    }
    free(dict->buckets);
    free(dict);
}
