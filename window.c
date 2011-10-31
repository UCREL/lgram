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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "window.h"
#include "lgram.h"
#include "error.h"
#include "blocks.h"
#include "output.h"
#include "options.h"
#include "merge_blocks.h"

window_t *w_head = NULL;

// allocate a word and set the word equal to start[0]...start[len]
int alloc_wmem(char **word, char *start, size_t len) {
    if((*word = (char *) malloc(len*sizeof(char)+1)) == NULL) {
        setlocation(LOCATION, __func__);
        return RERROR;
    }

    strncpy(*word, start, len);
    (*word)[len] = '\0';
    return 0;
}

// initialise a window from a block
int win_init(window_t **win, char *block, size_t sz) {
    if(block==NULL || sz < 1 || win == NULL) {
        printf("%s\n", block);
        setlocation(LOCATION, __func__);
        errno = EINVAL;
        return RERROR;
    }

    if((*win = (window_t *)malloc(sizeof(window_t))) == NULL) 
        return RERROR;

    size_t current = 0, len = 0;
    char *start = block, *end = block;
    window_t *w = *win;

    w->next = NULL;
    w->mcount = 0;

    // add an extra word if the block has a boundary characters
    // so that the boundary character can be tokensized
    w->wc = block_match(*(block+sz-1)) ? 2 : 1;

    // compute the number of space seperated words
    for(unsigned int i = 0; i < sz; i++) {
        if(isspace(block[i]))
            w->wc+=1;
    }

    if((w->words = (char **) malloc(sizeof(char *)*w->wc)) == NULL) {
        setlocation(LOCATION, __func__);
        return RERROR;
    }

    // parse words and place them into the w->words structure
    while(end < block+sz) {
        assert(current < w->wc);

        // find a space
        while(!isspace(*end) && end < block+sz) 
            end++;

        // end of block, tokensize boundary character
        if(end > start && block_match(*(end-1)))
            end--;
 
        len = end-start;
        if((alloc_wmem(&w->words[current], start, len)) == RERROR)
            return RERROR;

        // string is just a boundary character so re-alloc
        if(strlen(w->words[current]) == 0) {
            w->wc--;
            free(w->words[current]);
            if((w->words = (char **) realloc(w->words, sizeof(char *)*w->wc)) == NULL) {
                setlocation(LOCATION, __func__);
                return RERROR;
            }
        } else {
            current++;
        }

        // add boundary character to window
        if(block_match(*end)) {
            if((alloc_wmem(&w->words[current], end, 1)) == RERROR)
                return RERROR;
            current++;
        }

        start = ++end;
    }

    assert(current <= w->wc);
    return 0;
}

void win_free(window_t *w) {
    if(w == NULL)
        return;
    for(unsigned int i = 0; i < w->wc; i++)
        free(w->words[i]);
    free(w->words);
    free(w);
}

/* windows are cached in a linked list and then 
 * sync'ed to disk when the current buffer ends */
void win_store_add(window_t *w) {
    if(w_head == NULL) {
        w_head = w;
    } else {
        w->next = w_head;
        w_head = w;
    }
}

/* use the window w to compute and output ngrams. A window is 
 * simply block parsed into words */
unsigned long win_store_sync_helper(window_t *w) {
    unsigned long ngs = 0;
    size_t start = 0, n_end, n, n_init;

    if(w->mcount >= opts.nend)
        start = w->mcount-opts.nstart+1;

    for(; start < w->wc; start++) {
        n_init = opts.nstart;
        if(w->mcount > 0 && w->mcount > start) 
            n_init = MAX(opts.nstart, w->mcount-start+1);
        n_end = MIN(opts.nend, w->wc-start);

        for(n = n_init; n <= n_end; n++) {
            if(sql_log_ngrams(w, start, n)==RERROR)
                return RERROR;
            ngs += 1;
        }
    }
    return ngs;
}

/* iterate through all windows from the current buffer
 * and use the helper function to calculate ngrams */
unsigned long win_store_sync() {
    window_t *w;
    unsigned long ngs = 0;
    long ng = 0;

    // make sure database is open and begin transaction
    if((sql_log_start()) == RERROR)
        return RERROR;

    while(w_head != NULL) {
        if((sql_log_words(w_head)) == RERROR)
            return RERROR;
        if((ng = win_store_sync_helper(w_head)) == RERROR)
            return RERROR;
        w = w_head->next;
        win_free(w_head);
        w_head = w;
        ngs += ng;
    }
    
    // end transaction
    if((sql_log_end()) == RERROR)
        return RERROR;
    return ngs;
}
