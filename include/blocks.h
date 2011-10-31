#ifndef _BLOCKS_H_
#define _BLOCKS_H_

#include "lgram.h"

size_t block_number(char *, size_t);
int blocks_compute(char *, size_t, char **, long);
void block_free(char **, int);
bool block_match(char);

#endif
