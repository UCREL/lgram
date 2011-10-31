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

#include "lgram.h"
#include "error.h"
#include "merge_blocks.h"
#include "blocks.h"
#include "window.h"
#include "output.h"
#include "options.h"
#include "regex.h"

/* A block is a sequence that does not transverse boundary letters. 
 * For more details on boundary lettters see sep in preprocess.c */
long data_to_blocks(char *data, size_t sz, char ***blocks) {
    long block_n;

    // compute the number of blocks in data. 
    block_n = block_number(data, sz);

    if((*blocks = (char **) malloc(sizeof(char *)*block_n)) == NULL) {
        setlocation(LOCATION, __func__);
        return RERROR;
    }

    // populate blocks from tokenised data
    if((block_n = blocks_compute(data, sz, *blocks, block_n))==RERROR) {
        setlocation(LOCATION, __func__);
        return RERROR;
    }

    return block_n;
}

/* Convert blocks into windows whilst ensuring blocks which
 * transverse buffers are accounted for */
int process_block(char *block) {
    long sz, bsz;
    char *regex_b, *b = NULL;
    bool alloced = false;
    window_t *w = NULL;
    extern int store_count;

    /* previous block didn't terminate so we need to merge it 
     * with the current block */
    if(blocks_require_merge()) {
        if((sz = blocks_merge(block, &b)) == RERROR)
            return RERROR;
        alloced = true;
    } else {
        b = block;
        sz = strlen(block);
    }

    regex_b = regex_creplace(b);
    bsz = strlen(regex_b);

    if(win_init(&w, regex_b, bsz)==RERROR) {
        setlocation(LOCATION, __func__);
        return RERROR;
    }

    free(regex_b);

    // need this for ngram computation over blocks
    w->mcount = store_count;
    lgprintf(LG_INFO, "merged window size: %d\n", w->wc);
    win_store_add(w);

    /* this block does not terminate so we need to store it so
     * that blocks from the next buffer can be merged with it. */
    if(!block_match(b[sz-1])) 
        blocks_store(w);
    else 
        blocks_clear();

    if(alloced) {
        free(b);
        alloced = false;
    }
    return 0;
}


