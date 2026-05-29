#include "utf8util.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int u8_is_word_byte(unsigned char b) {
    if (b >= 0x80) return 1;            /* старший бит -> часть UTF-8 символа */
    return isalpha(b) ? 1 : 0;         /* латинская буква */
}

/*
 * Прочитать один символ (code point) из UTF-8 строки.
 * s     - указатель на начало символа
 * cp    - сюда пишется код символа
 * bytes - сколько байт занял символ
 * Возвращает 1 при успехе, 0 в конце строки.
 */
static int u8_next(const char *s, size_t len, size_t pos,
                   unsigned int *cp, size_t *bytes) {
    if (pos >= len) return 0;
    unsigned char c = (unsigned char)s[pos];
    if (c < 0x80) {                 /* 1 байт */
        *cp = c; *bytes = 1;
    } else if ((c & 0xE0) == 0xC0 && pos + 1 < len) {  /* 2 байта */
        *cp = ((c & 0x1F) << 6) | ((unsigned char)s[pos + 1] & 0x3F);
        *bytes = 2;
    } else if ((c & 0xF0) == 0xE0 && pos + 2 < len) {  /* 3 байта */
        *cp = ((c & 0x0F) << 12) | (((unsigned char)s[pos + 1] & 0x3F) << 6)
              | ((unsigned char)s[pos + 2] & 0x3F);
        *bytes = 3;
    } else {                        /* всё остальное трактуем как 1 байт */
        *cp = c; *bytes = 1;
    }
    return 1;
}

/* Является ли code point буквой (латиница или кириллица) */
static int cp_is_letter(unsigned int cp) {
    if (cp < 0x80) return isalpha((int)cp) ? 1 : 0;
    if (cp >= 0x0400 && cp <= 0x04FF) return 1;   /* кириллица */
    return 0;
}

/* Заглавная ли буква */
static int cp_is_upper(unsigned int cp) {
    if (cp < 0x80) return isupper((int)cp) ? 1 : 0;
    if (cp == 0x0401) return 1;                    /* Ё */
    if (cp >= 0x0410 && cp <= 0x042F) return 1;    /* А..Я */
    return 0;
}

/* Перевести code point в верхний регистр */
static unsigned int cp_to_upper(unsigned int cp) {
    if (cp < 0x80) return (unsigned int)toupper((int)cp);
    if (cp == 0x0451) return 0x0401;               /* ё -> Ё */
    if (cp >= 0x0430 && cp <= 0x044F) return cp - 0x20;
    return cp;
}

/* Перевести code point в нижний регистр */
static unsigned int cp_to_lower(unsigned int cp) {
    if (cp < 0x80) return (unsigned int)tolower((int)cp);
    if (cp == 0x0401) return 0x0451;               /* Ё -> ё */
    if (cp >= 0x0410 && cp <= 0x042F) return cp + 0x20;
    return cp;
}

/* Записать code point обратно в UTF-8. Возвращает число записанных байт. */
static size_t cp_write(unsigned int cp, char *out) {
    if (cp < 0x80) {
        out[0] = (char)cp; return 1;
    } else if (cp < 0x800) {
        out[0] = (char)(0xC0 | (cp >> 6));
        out[1] = (char)(0x80 | (cp & 0x3F));
        return 2;
    } else {
        out[0] = (char)(0xE0 | (cp >> 12));
        out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[2] = (char)(0x80 | (cp & 0x3F));
        return 3;
    }
}

WordCase u8_detect_case(const char *word, size_t len) {
    size_t pos = 0, bytes;
    unsigned int cp;
    int letters = 0, uppers = 0, first_upper = 0, idx = 0;

    while (u8_next(word, len, pos, &cp, &bytes)) {
        if (cp_is_letter(cp)) {
            letters++;
            if (cp_is_upper(cp)) {
                uppers++;
                if (idx == 0) first_upper = 1;
            }
            idx++;
        }
        pos += bytes;
    }

    if (letters == 0) return CASE_OTHER;
    if (uppers == letters && letters > 1) return CASE_UPPER;  /* ПРИВЕТ */
    if (first_upper && uppers == 1) return CASE_TITLE;        /* Привет */
    if (first_upper && letters == 1) return CASE_TITLE;       /* одна буква 'Я' */
    return CASE_OTHER;
}

char *u8_apply_case(const char *src, WordCase wc) {
    size_t len = strlen(src);
    /* запас: UTF-8 символ занимает максимум 3 байта в нашем диапазоне */
    char *dst = (char *)malloc(len * 3 + 1);
    if (!dst) return NULL;

    if (wc == CASE_OTHER) {                 /* копируем как есть */
        memcpy(dst, src, len + 1);
        return dst;
    }

    size_t pos = 0, out = 0, bytes;
    unsigned int cp;
    int seen_letter = 0;

    while (u8_next(src, len, pos, &cp, &bytes)) {
        unsigned int res = cp;
        if (cp_is_letter(cp)) {
            if (wc == CASE_UPPER) {
                res = cp_to_upper(cp);
            } else if (wc == CASE_TITLE && !seen_letter) {
                res = cp_to_upper(cp);      /* только первую букву */
            }
            seen_letter = 1;
        }
        out += cp_write(res, dst + out);
        pos += bytes;
    }
    dst[out] = '\0';
    return dst;
}

char *u8_to_lower(const char *src) {
    size_t len = strlen(src);
    char *dst = (char *)malloc(len * 3 + 1);
    if (!dst) return NULL;

    size_t pos = 0, out = 0, bytes;
    unsigned int cp;
    while (u8_next(src, len, pos, &cp, &bytes)) {
        unsigned int res = cp_is_letter(cp) ? cp_to_lower(cp) : cp;
        out += cp_write(res, dst + out);
        pos += bytes;
    }
    dst[out] = '\0';
    return dst;
}
