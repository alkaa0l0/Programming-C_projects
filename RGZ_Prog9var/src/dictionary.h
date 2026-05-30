#ifndef DICTIONARY_H
#define DICTIONARY_H

#define MAX_TRANSLATIONS 8   /* максимум значений у одного слова (многозначность) */

typedef struct {
    char *source;                                   
    char *translations[MAX_TRANSLATIONS]; 
    int   trans_count;                                
} Entry;

/* Словарь — динамический массив статей. */
typedef struct {
    Entry *entries;              
    int    count;      
    int    capacity;   
} Dictionary;

/* Создать пустой словарь. Возвращает NULL при нехватке памяти. */
Dictionary *dict_create(void);

int dict_load(Dictionary *d, const char *path, int *bad_lines);

const Entry *dict_find(const Dictionary *d, const char *word);

void dict_free(Dictionary *d);

#endif
