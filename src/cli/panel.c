#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "panel.h"

/* ---- Internal helpers ---- */

/*
 * Count display columns for a UTF-8 string.
 * ASCII chars are 1 column. Multi-byte sequences (e.g. suit symbols ♣♥♦♠,
 * which encode as 3-byte UTF-8) are still 1 display column each.
 */
static int32_t display_width(const char* s) {
    int32_t cols = 0;
    const unsigned char* p = (const unsigned char*)s;
    while (*p) {
        if      (*p < 0x80) { cols++; p += 1; }  /* ASCII                  */
        else if (*p < 0xE0) { cols++; p += 2; }  /* 2-byte sequence        */
        else if (*p < 0xF0) { cols++; p += 3; }  /* 3-byte (suits live here) */
        else                { cols++; p += 4; }  /* 4-byte sequence        */
    }
    return cols;
}

/*
 * Double the capacity of the lines and widths arrays.
 * Single point of truth — called by panel_add_line, panel_pad_height,
 * and panel_join before any direct insert.
 */
static bool panel_grow(TextPanel* p) {
    int32_t nc   = p->capacity ? p->capacity * 2 : 16;
    char**   nl  = realloc(p->lines,  (size_t)nc * sizeof(char*));
    int32_t* nw  = realloc(p->widths, (size_t)nc * sizeof(int32_t));
    if (!nl || !nw) return false;
    p->lines    = nl;
    p->widths   = nw;
    p->capacity = nc;
    return true;
}

/* ---- Lifecycle ---- */

TextPanel* panel_create(void) {
    TextPanel* p = malloc(sizeof(TextPanel));
    if (!p) return NULL;
    p->capacity   = 16;
    p->line_count = 0;
    p->width      = 0;
    p->lines  = malloc((size_t)p->capacity * sizeof(char*));
    p->widths = malloc((size_t)p->capacity * sizeof(int32_t));
    if (!p->lines || !p->widths) {
        free(p->lines);
        free(p->widths);
        free(p);
        return NULL;
    }
    return p;
}

void panel_free(TextPanel* p) {
    if (!p) return;
    for (int32_t i = 0; i < p->line_count; i++) free(p->lines[i]);
    free(p->lines);
    free(p->widths);
    free(p);
}

/* ---- Building ---- */

bool panel_add_line(TextPanel* p, const char* line) {
    if (p->line_count == p->capacity && !panel_grow(p)) return false;
    char* copy = strdup(line);
    if (!copy) return false;
    int32_t dw = display_width(line);
    p->lines[p->line_count]  = copy;
    p->widths[p->line_count] = dw;
    p->line_count++;
    if (dw > p->width) p->width = dw;
    return true;
}

/* ---- Querying ---- */

int32_t panel_width(const TextPanel* p)  { return p->width; }
int32_t panel_height(const TextPanel* p) { return p->line_count; }

/* ---- In-place layout ---- */

/*
 * Pad every line shorter than `width` with trailing spaces so all lines
 * reach exactly `width` display columns. Lines already at or wider than
 * `width` are untouched. Spaces are ASCII, so column deficit == byte deficit.
 */
void panel_pad_width(TextPanel* p, int32_t width) {
    for (int32_t i = 0; i < p->line_count; i++) {
        if (p->widths[i] >= width) continue;
        int32_t pad       = width - p->widths[i];
        int32_t old_bytes = (int32_t)strlen(p->lines[i]);
        char* s = realloc(p->lines[i], (size_t)(old_bytes + pad + 1));
        if (!s) continue;
        memset(s + old_bytes, ' ', (size_t)pad);
        s[old_bytes + pad] = '\0';
        p->lines[i]  = s;
        p->widths[i] = width;
    }
    if (width > p->width) p->width = width;
}

/*
 * Append blank lines of exactly p->width spaces until line_count == height.
 * Allocates blank strings directly rather than via panel_add_line to avoid
 * a redundant strdup and re-measurement.
 */
void panel_pad_height(TextPanel* p, int32_t height) {
    while (p->line_count < height) {
        if (p->line_count == p->capacity && !panel_grow(p)) return;
        char* blank = malloc((size_t)p->width + 1);
        if (!blank) return;
        memset(blank, ' ', (size_t)p->width);
        blank[p->width] = '\0';
        p->lines[p->line_count]  = blank;
        p->widths[p->line_count] = p->width;
        p->line_count++;
        /* p->width is unchanged */
    }
}

/* ---- Composition — non-consuming ---- */

TextPanel* panel_stack(const TextPanel* above, const TextPanel* below) {
    TextPanel* result = panel_create();
    if (!result) return NULL;
    for (int32_t i = 0; i < above->line_count; i++) panel_add_line(result, above->lines[i]);
    for (int32_t i = 0; i < below->line_count; i++) panel_add_line(result, below->lines[i]);
    return result;
}

/*
 * Join two panels side-by-side with `gap` space columns between them.
 *
 * Left lines are padded on-the-fly to left->width columns (no mutation of
 * input). Height mismatch is handled by treating missing rows as blank:
 *   - missing left row  → left->width spaces
 *   - missing right row → nothing (right side simply absent for that row)
 */
TextPanel* panel_join(const TextPanel* left, const TextPanel* right, int32_t gap) {
    int32_t height = left->line_count > right->line_count
                     ? left->line_count : right->line_count;
    TextPanel* result = panel_create();
    if (!result) return NULL;

    for (int32_t i = 0; i < height; i++) {
        const char* ls     = i < left->line_count  ? left->lines[i]  : NULL;
        int32_t     ldw    = i < left->line_count  ? left->widths[i] : 0;
        int32_t     lpad   = left->width - ldw;
        int32_t     lbytes = ls ? (int32_t)strlen(ls) : 0;

        const char* rs     = i < right->line_count ? right->lines[i] : NULL;
        int32_t     rbytes = rs ? (int32_t)strlen(rs) : 0;

        int32_t total = lbytes + lpad + gap + rbytes + 1;
        char* row = malloc((size_t)total);
        if (!row) break;

        char* cur = row;
        if (ls) { memcpy(cur, ls, (size_t)lbytes); cur += lbytes; }
        else    { memset(cur, ' ', (size_t)left->width); cur += left->width; }
        memset(cur, ' ', (size_t)lpad); cur += lpad;
        memset(cur, ' ', (size_t)gap);  cur += gap;
        if (rs) { memcpy(cur, rs, (size_t)rbytes); cur += rbytes; }
        *cur = '\0';

        /* Direct insert — we already know the display width, no re-measurement */
        if (result->line_count == result->capacity && !panel_grow(result)) {
            free(row);
            break;
        }
        int32_t row_dw = left->width + gap + (rs ? display_width(rs) : 0);
        result->lines[result->line_count]  = row;
        result->widths[result->line_count] = row_dw;
        result->line_count++;
        if (row_dw > result->width) result->width = row_dw;
    }
    return result;
}

/* ---- Composition — consuming ---- */

TextPanel* panel_stack_consume(TextPanel* above, TextPanel* below) {
    TextPanel* result = panel_stack(above, below);
    panel_free(above);
    panel_free(below);
    return result;
}

TextPanel* panel_join_consume(TextPanel* left, TextPanel* right, int32_t gap) {
    TextPanel* result = panel_join(left, right, gap);
    panel_free(left);
    panel_free(right);
    return result;
}

/* ---- Output ---- */

void panel_print(const TextPanel* p, Renderer* r) {
    for (int32_t i = 0; i < p->line_count; i++)
        render_line(r, p->lines[i]);
}
