#include "translator.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ===================== Работа с буквами UTF-8 ===================== */


int is_word_byte(unsigned char b) {
    return (b >= 0x80) || isalpha(b);
}

static unsigned int read_char(const char *s, int len, int pos, int *bytes) {
    unsigned char c = (unsigned char)s[pos];
    if (c < 0x80) {                           
        *bytes = 1;
        return c;
    }
    if ((c & 0xE0) == 0xC0 && pos + 1 < len) { 
        *bytes = 2;
        return ((c & 0x1F) << 6) | ((unsigned char)s[pos + 1] & 0x3F);
    }
    *bytes = 1;
    return c;
}

/* Записать символ обратно в UTF-8. */
static int write_char(unsigned int cp, char *out) {
    if (cp < 0x80) { out[0] = (char)cp; return 1; }
    out[0] = (char)(0xC0 | (cp >> 6));
    out[1] = (char)(0x80 | (cp & 0x3F));
    return 2;
}

static int is_letter(unsigned int cp) {
    if (cp < 0x80) return isalpha((int)cp);
    return (cp >= 0x0400 && cp <= 0x04FF);     
}

static int is_upper_cp(unsigned int cp) {
    if (cp < 0x80) return isupper((int)cp);
    if (cp == 0x0401) return 1;             
    return (cp >= 0x0410 && cp <= 0x042F);     
}

static unsigned int to_upper_cp(unsigned int cp) {
    if (cp < 0x80) return toupper((int)cp);
    if (cp == 0x0451) return 0x0401;          
    if (cp >= 0x0430 && cp <= 0x044F) return cp - 0x20; 
    return cp;
}

static unsigned int to_lower_cp(unsigned int cp) {
    if (cp < 0x80) return tolower((int)cp);
    if (cp == 0x0401) return 0x0451;     
    if (cp >= 0x0410 && cp <= 0x042F) return cp + 0x20; 
    return cp;
}

char *str_to_lower(const char *s) {
    int len = (int)strlen(s);
    char *out = malloc(len * 2 + 1);         
    if (!out) return NULL;
    int pos = 0, o = 0, bytes;
    while (pos < len) {
        unsigned int cp = read_char(s, len, pos, &bytes);
        o += write_char(to_lower_cp(cp), out + o);
        pos += bytes;
    }
    out[o] = '\0';
    return out;
}

WordCase detect_case(const char *word, int len) {
    int letters = 0, uppers = 0, first_upper = 0, pos = 0, bytes, idx = 0;
    while (pos < len) {
        unsigned int cp = read_char(word, len, pos, &bytes);
        if (is_letter(cp)) {
            letters++;
            if (is_upper_cp(cp)) {
                uppers++;
                if (idx == 0) first_upper = 1;
            }
            idx++;
        }
        pos += bytes;
    }
    if (letters == 0) return CASE_LOWER;
    if (uppers == letters && letters > 1) return CASE_UPPER; 
    if (first_upper) return CASE_TITLE;                        
    return CASE_LOWER;
}

char *apply_case(const char *s, WordCase c) {
    int len = (int)strlen(s);
    char *out = malloc(len * 2 + 1);
    if (!out) return NULL;
    int pos = 0, o = 0, bytes, first = 1;
    while (pos < len) {
        unsigned int cp = read_char(s, len, pos, &bytes);
        unsigned int res = cp;
        if (is_letter(cp)) {
            if (c == CASE_UPPER) res = to_upper_cp(cp);
            else if (c == CASE_TITLE && first) res = to_upper_cp(cp);
            first = 0;
        }
        o += write_char(res, out + o);
        pos += bytes;
    }
    out[o] = '\0';
    return out;
}

/* ===================== Перевод текста ===================== */

static void translate_word(FILE *out, const char *word, int len,
                           const Dictionary *d, const Options *opt, Stats *st) {
    char *w = malloc(len + 1);
    if (!w) return;
    memcpy(w, word, len);
    w[len] = '\0';
    st->total++;

    const Entry *e = dict_find(d, w);
    if (!e) {                                  
        fwrite(word, 1, len, out);
        st->untranslated++;
        log_msg(LOG_INFO, "Слово не найдено: %s", w);
        free(w);
        return;
    }

    WordCase wc = opt->preserve_case ? detect_case(word, len) : CASE_LOWER;

    char *primary = apply_case(e->translations[0], wc);
    fputs(primary ? primary : e->translations[0], out);
    free(primary);

    if (opt->show_alternatives && e->trans_count > 1) {
        fputc('(', out);
        for (int i = 1; i < e->trans_count; i++) {
            if (i > 1) fputc('|', out);
            char *alt = apply_case(e->translations[i], wc);
            fputs(alt ? alt : e->translations[i], out);
            free(alt);
        }
        fputc(')', out);
    }

    st->translated++;
    free(w);
}

int translate_file(const char *in, const char *out,
                   const Dictionary *d, const Options *opt, Stats *st) {
    FILE *fin = fopen(in, "rb");
    if (!fin) {
        log_msg(LOG_ERROR, "Не удалось открыть входной файл: %s", in);
        return -1;
    }
    FILE *fout = fopen(out, "wb");
    if (!fout) {
        log_msg(LOG_ERROR, "Не удалось создать выходной файл: %s", out);
        fclose(fin);
        return -2;
    }

    st->total = st->translated = st->untranslated = 0;

    char word[1024];
    int wlen = 0;
    int ch;
    while ((ch = fgetc(fin)) != EOF) {
        if (is_word_byte((unsigned char)ch) && wlen < (int)sizeof(word) - 1) {
            word[wlen++] = (char)ch;           
        } else {
            if (wlen > 0) {                    
                translate_word(fout, word, wlen, d, opt, st);
                wlen = 0;
            }
            fputc(ch, fout);                  
        }
    }
    if (wlen > 0) translate_word(fout, word, wlen, d, opt, st); 

    fclose(fin);
    fclose(fout);
    log_msg(LOG_INFO, "Готово. Слов: %d, переведено: %d, без перевода: %d",
            st->total, st->translated, st->untranslated);
    return 0;
}
