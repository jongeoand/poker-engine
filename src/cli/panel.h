#ifndef PANEL_H_
#define PANEL_H_

#include <stdint.h>
#include <stdbool.h>

#include "render.h"

typedef struct {
    char**   lines;       /* heap array of heap-allocated strings (no trailing \n) */
    int32_t* widths;      /* parallel: display-column width of each line           */
    int32_t  line_count;  /* number of lines currently stored                      */
    int32_t  capacity;    /* allocated length of lines[] and widths[]              */
    int32_t  width;       /* cached max display width across all lines             */
} TextPanel;

/* Lifecycle */
TextPanel* panel_create(void);
void       panel_free(TextPanel* p);    /* frees internals + struct */

/* Building */
bool panel_add_line(TextPanel* p, const char* line);

/* Querying */
int32_t panel_width(const TextPanel* p);
int32_t panel_height(const TextPanel* p);

/* In-place layout */
void panel_pad_width(TextPanel* p, int32_t width);
void panel_pad_height(TextPanel* p, int32_t height);

/* Composition — non-consuming (inputs unchanged) */
TextPanel* panel_stack(const TextPanel* above, const TextPanel* below);
TextPanel* panel_join(const TextPanel* left, const TextPanel* right, int32_t gap);

/* Composition — consuming (inputs freed after use) */
TextPanel* panel_stack_consume(TextPanel* above, TextPanel* below);
TextPanel* panel_join_consume(TextPanel* left, TextPanel* right, int32_t gap);

/* Output */
void panel_print(const TextPanel* p, Renderer* r);

#endif
