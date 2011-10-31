#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "window.h"

long data_to_blocks(char *, size_t sz, char ***);
int process_block(char *);

#endif
