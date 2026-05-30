#include "dictionary.h"
#include "translator.h"   /* str_to_lower */
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static char *trim(char *s) {
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;
    if (*s == '\0') return s;
    char *end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n'))
        *end-- = '\0';
    return s;
}

Dictionary *dict_create(void) {
    Dictionary *d = malloc(sizeof(Dictionary));
    if (!d) return NULL;
    d->count = 0;
    d->capacity = 16;
    d->entries = malloc(sizeof(Entry) * d->capacity);
    if (!d->entries) { free(d); return NULL; }
    return d;
}

/* Добавить новое слово с одним переводом. */
static Entry *dict_add_word(Dictionary *d, const char *source) {
    if (d->count == d->capacity) {                 
        int ncap = d->capacity * 2;
        Entry *tmp = realloc(d->entries, sizeof(Entry) * ncap);
        if (!tmp) return NULL;
        d->entries = tmp;
        d->capacity = ncap;
    }
    Entry *e = &d->entries[d->count];
    e->source = strdup(source);
    e->trans_count = 0;
    if (!e->source) return NULL;
    d->count++;
    return e;
}

/* Найти изменяемую статью по ключу (внутренняя). */
static Entry *find_mut(Dictionary *d, const char *key) {
    for (int i = 0; i < d->count; i++)
        if (strcmp(d->entries[i].source, key) == 0)
            return &d->entries[i];
    return NULL;
}

int dict_load(Dictionary *d, const char *path, int *bad_lines) {
    FILE *f = fopen(path, "r");
    if (!f) {
        log_msg(LOG_ERROR, "Не удалось открыть файл словаря: %s", path);
        return -1;
    }

    char line[2048];
    int bad = 0, lineno = 0;

    while (fgets(line, sizeof(line), f)) {
        lineno++;
        char *p = trim(line);
        if (*p == '\0' || *p == '#') continue;       

        char *eq = strchr(p, '=');
        if (!eq) {
            log_msg(LOG_WARNING, "Строка %d словаря без '=', пропущена", lineno);
            bad++;
            continue;
        }
        *eq = '\0';
        char *key_raw = trim(p);
        char *val_raw = trim(eq + 1);
        if (*key_raw == '\0' || *val_raw == '\0') {
            log_msg(LOG_WARNING, "Строка %d словаря с пустым ключом/значением", lineno);
            bad++;
            continue;
        }

        char *key = str_to_lower(key_raw);          
        if (!key) { fclose(f); return -1; }

        Entry *e = find_mut(d, key);                 
        if (!e) e = dict_add_word(d, key);
        free(key);
        if (!e) { log_msg(LOG_ERROR, "Нехватка памяти"); fclose(f); return -1; }

        /* Разбираем переводы, разделённые '|' */
        char *tok = strtok(val_raw, "|");
        while (tok) {
            char *t = trim(tok);
            if (*t != '\0' && e->trans_count < MAX_TRANSLATIONS) {
                e->translations[e->trans_count] = strdup(t);
                if (e->translations[e->trans_count]) e->trans_count++;
            }
            tok = strtok(NULL, "|");
        }
    }

    fclose(f);
    if (bad_lines) *bad_lines = bad;
    log_msg(LOG_INFO, "Словарь загружен: %d слов, %d некорректных строк", d->count, bad);
    return d->count;
}

const Entry *dict_find(const Dictionary *d, const char *word) {
    if (!d || !word) return NULL;
    char *key = str_to_lower(word);
    if (!key) return NULL;
    const Entry *result = NULL;
    for (int i = 0; i < d->count; i++) {
        if (strcmp(d->entries[i].source, key) == 0) {
            result = &d->entries[i];
            break;
        }
    }
    free(key);
    return result;
}

void dict_free(Dictionary *d) {
    if (!d) return;
    for (int i = 0; i < d->count; i++) {
        free(d->entries[i].source);
        for (int j = 0; j < d->entries[i].trans_count; j++)
            free(d->entries[i].translations[j]);
    }
    free(d->entries);
    free(d);
}
