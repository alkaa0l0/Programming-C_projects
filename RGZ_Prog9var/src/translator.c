#include "translator.h"
#include "utf8util.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/* --- Простой растущий строковый буфер --- */
typedef struct {
    char  *data;
    size_t len;
    size_t cap;
} StrBuf;

static int sb_init(StrBuf *b) {
    b->cap = 256;
    b->len = 0;
    b->data = (char *)malloc(b->cap);
    if (!b->data) return -1;
    b->data[0] = '\0';
    return 0;
}

static int sb_append(StrBuf *b, const char *s, size_t n) {
    if (b->len + n + 1 > b->cap) {
        size_t ncap = b->cap;
        while (b->len + n + 1 > ncap) ncap *= 2;
        char *tmp = (char *)realloc(b->data, ncap);
        if (!tmp) return -1;
        b->data = tmp;
        b->cap = ncap;
    }
    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
    return 0;
}

/* --- Перевод одного слова в буфер --- */
static void translate_word(StrBuf *out, const char *word, size_t wlen,
                           const Dictionary *dict,
                           const TranslatorOptions *opts,
                           TranslateStats *st) {
    /* временная C-строка слова */
    char *w = (char *)malloc(wlen + 1);
    if (!w) return;
    memcpy(w, word, wlen);
    w[wlen] = '\0';

    st->total_words++;

    const DictEntry *e = dict_lookup(dict, w);
    if (!e) {                                   /* нет в словаре -> как есть */
        sb_append(out, word, wlen);
        st->untranslated++;
        log_msg(LOG_INFO, "Слово не найдено в словаре: %s", w);
        free(w);
        return;
    }

    WordCase wc = opts->preserve_case
                  ? u8_detect_case(word, wlen)
                  : CASE_OTHER;

    /* основной перевод */
    char *primary = u8_apply_case(e->translations[0], wc);
    if (primary) {
        sb_append(out, primary, strlen(primary));
        free(primary);
    } else {
        sb_append(out, e->translations[0], strlen(e->translations[0]));
    }

    /* альтернативные значения многозначного слова */
    if (opts->show_alternatives && e->trans_count > 1) {
        sb_append(out, "(", 1);
        for (size_t i = 1; i < e->trans_count; i++) {
            if (i > 1) sb_append(out, "|", 1);
            char *alt = u8_apply_case(e->translations[i], wc);
            const char *s = alt ? alt : e->translations[i];
            sb_append(out, s, strlen(s));
            free(alt);
        }
        sb_append(out, ")", 1);
    }

    st->translated++;
    free(w);
}

/* Перевести одну строку (сегмент текста), сохраняя все разделители. */
static char *translate_line(const char *line, const Dictionary *dict,
                            const TranslatorOptions *opts,
                            TranslateStats *st) {
    StrBuf out;
    if (sb_init(&out) != 0) return NULL;

    size_t i = 0, len = strlen(line);
    while (i < len) {
        if (u8_is_word_byte((unsigned char)line[i])) {
            size_t start = i;
            while (i < len && u8_is_word_byte((unsigned char)line[i])) i++;
            translate_word(&out, line + start, i - start, dict, opts, st);
        } else {
            /* разделитель (пробел, пунктуация, перенос строки) - копируем как есть */
            sb_append(&out, line + i, 1);
            i++;
        }
    }
    return out.data;
}

/* --- Многопоточность --- */
typedef struct {
    char **lines;
    char **results;
    size_t start, end;
    const Dictionary *dict;
    const TranslatorOptions *opts;
    TranslateStats stats;     /* локальная статистика потока */
} ThreadArg;

static void *worker(void *arg) {
    ThreadArg *a = (ThreadArg *)arg;
    memset(&a->stats, 0, sizeof(a->stats));
    for (size_t i = a->start; i < a->end; i++) {
        a->results[i] = translate_line(a->lines[i], a->dict, a->opts, &a->stats);
    }
    return NULL;
}

/* Прочитать файл целиком в буфер. *out_size - размер. */
static char *read_whole_file(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 0) { fclose(f); return NULL; }
    char *buf = (char *)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    buf[rd] = '\0';
    fclose(f);
    *out_size = rd;
    return buf;
}

/*
 * Разбить буфер на сегменты-строки. Каждый сегмент включает завершающий '\n'
 * (кроме, возможно, последнего). Это позволяет точно восстановить текст.
 */
static char **split_lines(const char *buf, size_t size, size_t *count) {
    size_t cap = 64, n = 0;
    char **lines = (char **)malloc(cap * sizeof(char *));
    if (!lines) return NULL;

    size_t start = 0;
    for (size_t i = 0; i <= size; i++) {
        if (i == size || buf[i] == '\n') {
            if (i == size && start == size && n > 0) break; /* нет хвоста */
            size_t seg_len = (i < size) ? (i - start + 1) : (i - start);
            char *seg = (char *)malloc(seg_len + 1);
            if (!seg) { for (size_t k=0;k<n;k++) free(lines[k]); free(lines); return NULL; }
            memcpy(seg, buf + start, seg_len);
            seg[seg_len] = '\0';
            if (n == cap) {
                cap *= 2;
                char **t = (char **)realloc(lines, cap * sizeof(char *));
                if (!t) { for (size_t k=0;k<n;k++) free(lines[k]); free(lines); return NULL; }
                lines = t;
            }
            lines[n++] = seg;
            start = i + 1;
            if (i == size) break;
        }
    }
    *count = n;
    return lines;
}

int translate_file(const char *in_path, const char *out_path,
                   const Dictionary *dict, const TranslatorOptions *opts,
                   TranslateStats *stats) {
    size_t size = 0;
    char *buf = read_whole_file(in_path, &size);
    if (!buf) {
        log_msg(LOG_ERROR, "Не удалось прочитать входной файл: %s", in_path);
        return -1;
    }

    size_t nlines = 0;
    char **lines = split_lines(buf, size, &nlines);
    free(buf);
    if (!lines) {
        log_msg(LOG_ERROR, "Нехватка памяти при разборе текста");
        return -2;
    }

    char **results = (char **)calloc(nlines ? nlines : 1, sizeof(char *));
    if (!results) {
        for (size_t i = 0; i < nlines; i++) free(lines[i]);
        free(lines);
        return -2;
    }

    int nthreads = opts->threads;
    if (nthreads < 1) nthreads = 1;
    if ((size_t)nthreads > nlines) nthreads = (nlines > 0) ? (int)nlines : 1;

    log_msg(LOG_INFO, "Перевод %zu строк в %d поток(ах)", nlines, nthreads);

    pthread_t *tids = (pthread_t *)malloc(sizeof(pthread_t) * nthreads);
    ThreadArg *args = (ThreadArg *)malloc(sizeof(ThreadArg) * nthreads);
    if (!tids || !args) {
        free(tids); free(args);
        for (size_t i = 0; i < nlines; i++) free(lines[i]);
        free(lines); free(results);
        return -2;
    }

    size_t per = (nlines + nthreads - 1) / (size_t)nthreads;  /* строк на поток */
    for (int t = 0; t < nthreads; t++) {
        args[t].lines = lines;
        args[t].results = results;
        args[t].start = (size_t)t * per;
        args[t].end = args[t].start + per;
        if (args[t].end > nlines) args[t].end = nlines;
        if (args[t].start > nlines) args[t].start = nlines;
        args[t].dict = dict;
        args[t].opts = opts;
        pthread_create(&tids[t], NULL, worker, &args[t]);
    }

    TranslateStats total = {0, 0, 0};
    for (int t = 0; t < nthreads; t++) {
        pthread_join(tids[t], NULL);
        total.total_words  += args[t].stats.total_words;
        total.translated   += args[t].stats.translated;
        total.untranslated += args[t].stats.untranslated;
    }
    free(tids);
    free(args);

    /* Записываем результат в выходной файл по порядку строк. */
    FILE *out = fopen(out_path, "wb");
    if (!out) {
        log_msg(LOG_ERROR, "Не удалось создать выходной файл: %s", out_path);
        for (size_t i = 0; i < nlines; i++) { free(lines[i]); free(results[i]); }
        free(lines); free(results);
        return -3;
    }
    for (size_t i = 0; i < nlines; i++) {
        if (results[i]) fputs(results[i], out);
    }
    fclose(out);

    for (size_t i = 0; i < nlines; i++) { free(lines[i]); free(results[i]); }
    free(lines);
    free(results);

    if (stats) *stats = total;
    log_msg(LOG_INFO, "Готово. Слов: %zu, переведено: %zu, без перевода: %zu",
            total.total_words, total.translated, total.untranslated);
    return 0;
}
