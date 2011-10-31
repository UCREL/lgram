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
#include <sys/types.h>

#if __unix__ || MAC_OS_X
#include <unistd.h>
#elif WIN32 
#include "win32.h"
#else
#error "Unknown operating system"
#endif

#include "lgram.h"
#include "error.h"
#include "blocks.h"
#include "output.h"
#include "options.h"

#define MATCH(x) (isspace((x)) || block_match((x)))
static FILE *fp = NULL;

/* aligns buffer and file to space seperated entries so
 * that we do not read half words and such */
int seek_buffer(int fd, char *buffer, int sz) {
    int last_index = sz;
    off_t os;

    while(!MATCH(buffer[last_index]) && last_index--) {}

    if(last_index < 1) {
        last_index = sz;
    } else {
        if((os = lseek(fd, last_index-sz, SEEK_CUR)) == RERROR) {
            setlocation(LOCATION, __func__);
            return -1;
        }
        lgprintf(LG_INFO, "seeking to offset: %d\n", os);
    }
    buffer[last_index] = '\0';
    return last_index;
}

char *next_input() {
    static unsigned long idx = 0;
    static char buffer[BUFSIZ];

    if(!opts.input_file) {
        if(idx < opts.input_n) {
            return opts.input[idx++];
        } 
        return NULL;
    }

    if(fp == NULL) {
        if((fp = fopen(opts.input_file, "r")) == NULL)
            fatal_error(LOCATION, "fopen()");
    }

    if((fgets(buffer, BUFSIZ, fp))==NULL)
        return NULL;
    buffer[strlen(buffer)-1] = '\0';
    return buffer;
}

void close_input() {
    if(fp)
        fclose(fp);
}
