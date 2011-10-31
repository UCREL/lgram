#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "lgram.h"

typedef struct window_structure  {
    char **words;
    struct window_structure *next;
    unsigned int wc;
    unsigned int mcount;
} window_t;

int win_init(window_t **, char *, size_t); 
char *win_ngram(window_t *, int, int, char *, size_t);
void win_free(window_t *);

void win_store_add(window_t *w);
unsigned long win_store_sync();

#endif
