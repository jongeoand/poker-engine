#ifndef RENDER_TYPES_H_
#define RENDER_TYPES_H_

typedef enum {
	RENDER_STATE,   /* dominant ComboState per cell */
	RENDER_PURITY,  /* purity/fraction of dominant state */
	RENDER_DRAW,    /* behind-live (draw) fraction */
	RENDER_SUIT,    /* dominant SuitClass per cell */
	RENDER_FLUSH,   /* flush-draw/made fraction */
} RenderMode;

typedef enum { SYMSET_ASCII, SYMSET_UNICODE } SymSet;
typedef enum { CELL_1, CELL_2, CELL_4 }      CellWidth;

#endif
