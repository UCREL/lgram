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

#include <ctype.h>
#include "error.h"
#include "lgram.h"
#include "options.h"

// check for a block boundary character
bool block_match(char m) {
    for(int i = 0; opts.boundary[i] != '\0'; i++) {
        if(m == opts.boundary[i])
            return true;
    }
    return false;
}

// count the number of blocks within a buffer
size_t block_number(char *buffer, size_t sz) {
    if(buffer == NULL || sz < 1) {
        setlocation(LOCATION, __func__);
        errno = EINVAL;
        return false;
    }

    size_t count = 0;
    size_t j;

    for(j = 0; j < sz; j++) {
        if(block_match(buffer[j])) 
            count+=1;
    }

    if(!block_match(buffer[j-1]))
        count+=1;

    return count;
}

// split a buffer into blocks
int blocks_compute(char *inbuffer, size_t sz, char **outbuffer, long block_n) {
    if(inbuffer == NULL || outbuffer == NULL || sz < 1 || block_n < 1) {
        setlocation(LOCATION, __func__);
        errno = EINVAL;
        return RERROR;
    }

    long biter = 0, bk_sz = 0;
    char *start, *end = inbuffer;

    while(end < inbuffer+sz) {
        assert(biter <= block_n);

        while(isspace(*end) && *end != '\0') 
            end++;
        if(end > inbuffer+sz)
            continue;
        start = end;

        // find the end of the current block
        while(!block_match(*end) && *end != '\0')
            end++;

        bk_sz = end-start+1;
        assert(bk_sz <= opts.bufsz);
        if(bk_sz <= 0)
            break;

        // copy the block into the output buffer
        if((outbuffer[biter] = (char *) malloc(sizeof(char)*bk_sz+1)) == NULL) {
            setlocation(LOCATION, __func__);
            return RERROR;
        }

        strncpy(outbuffer[biter], start, bk_sz);
        outbuffer[biter][bk_sz] = '\0';
        biter++;
        end++;
        
    }
    return biter;
}

void block_free(char **blocks, int block_n) {
    if(blocks == NULL || block_n < 1) 
        return;
    for(int i = 0; i < block_n; i++) 
       free(blocks[i]);
}
