/* lgram - an efficient ngram builder from Lancaster University
   Copyright (C) 2011 Edward J. L. Bell

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h>
#include "lgram.h"
#include "window.h"
#include "error.h"
#include "options.h"

/* The purpose of these functions is to join blocks which span buffers */
char *block_store[NGRAM_MAX-1];
int store_count = 0;

void blocks_clear() {
    for(int i = 0; i < store_count; i++)
        free(block_store[i]);
    store_count = 0;
}

bool blocks_require_merge() {
    return store_count > 0;
}

/* replace stored blocks if current window size
 * is equal or exceeds maximum ngram size */
int blocks_store(window_t *w) {
    unsigned long i = 0;

    if (w->wc >= opts.nend)  {
        blocks_clear();
        i = w->wc-opts.nend+1;
    }

    for(; i < w->wc; i++) 
        block_store[store_count++] = strdup(w->words[i]);
    return 0;
}

// take all stored blocks and merge with current block
int blocks_merge(char *block, char **newblock) {
    unsigned long total_len = 0;

    for(int i = 0; i < store_count; i++) 
        total_len += strlen(block_store[i])+1;
    total_len += strlen(block);

    if((*newblock = (char *)malloc(sizeof(char)*total_len+1)) == NULL) {
        setlocation(getlocation(), __func__);
        return RERROR;
    }

    memset(*newblock, '\0', sizeof(char)*total_len+1);

    for(int i = 0; i < store_count; i++) {
        strcat(*newblock, block_store[i]);
        strcat(*newblock, " ");
    }

    strcat(*newblock, block);
    strcat(*newblock, "\0");
    return total_len;
}
