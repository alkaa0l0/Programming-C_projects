#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "dictionary.h"

/* Регистр слова */
typedef enum {
    CASE_LOWER,   
    CASE_TITLE,   
    CASE_UPPER   
} WordCase;

/* --- Функции работы с регистром (UTF-8: латиница и кириллица) --- */

char *str_to_lower(const char *s);

WordCase detect_case(const char *word, int len);

char *apply_case(const char *s, WordCase c);

/* --- Перевод --- */
typedef struct {
    int preserve_case;              
    int show_alternatives;  
} Options;

/* Статистика по результату */
typedef struct {
    int total;          
    int translated;     
    int untranslated;  
} Stats;


int translate_file(const char *in, const char *out,
                   const Dictionary *d, const Options *opt, Stats *st);

#endif 
