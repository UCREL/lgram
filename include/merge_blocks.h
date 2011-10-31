#ifndef _MERGE_BLOCKS_H_
#define _MERGE_BLOCKS_H_

#include "window.h"

int blocks_store(window_t *);
bool blocks_require_merge();
int blocks_merge(char *, char **);
void blocks_clear();

#endif

