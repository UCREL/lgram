#ifndef _LGRAM_H_
#define _LGRAM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <limits.h>

#if __unix__ || MAC_OS_X
#include <unistd.h>
#include <stdbool.h>
#elif WIN32 
#include "win32.h"
#endif

// this can only be a single digit. If changed to > 10
// the query building code in sqlite.c must be updated
#define NGRAM_MAX 9
#define BLOCK_MIN 1
// size of buffer in bytes
#define BLOCK_DEF 65535   
#define TMP_DB_NAME "tmplgdbXXXXXX"

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

#endif
