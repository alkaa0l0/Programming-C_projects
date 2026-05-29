#ifndef UTF8UTIL_H
#define UTF8UTIL_H

#include <stddef.h>

/*
 * Утилиты для работы с буквами в кодировке UTF-8.
 * Поддерживаются латиница (ASCII) и кириллица (диапазон U+0400..U+04FF, включая Ё/ё).
 */

/* Регистр слова целиком */
typedef enum {
    CASE_OTHER = 0,  /* строчное или смешанное -> перевод не меняем */
    CASE_TITLE,      /* Первая буква заглавная (Привет)            */
    CASE_UPPER       /* ВСЕ БУКВЫ ЗАГЛАВНЫЕ (ПРИВЕТ)               */
} WordCase;

/*
 * Является ли байт b началом/частью буквы.
 * Буквой считаем: ASCII a-z A-Z и любой байт >= 0x80 (часть многобайтного
 * символа UTF-8, что для нашей задачи покрывает кириллицу).
 */
int u8_is_word_byte(unsigned char b);

/* Определить регистр слова, заданного как UTF-8 строка [word, word+len). */
WordCase u8_detect_case(const char *word, size_t len);

/*
 * Применить регистр wc к строке src (UTF-8) и записать результат в dst.
 * Возвращает динамически выделенную строку (нужно free), либо NULL при ошибке.
 */
char *u8_apply_case(const char *src, WordCase wc);

/*
 * Привести строку src (UTF-8) к нижнему регистру.
 * Возвращает динамически выделенную строку (нужно free), либо NULL при ошибке.
 */
char *u8_to_lower(const char *src);

#endif /* UTF8UTIL_H */
