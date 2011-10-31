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
#include "preprocess.h"
#include "error.h"
#include "options.h"
#include "lgram.h"

char *trim(char *str) {
    char *end;
    while(isspace(*str)) 
        str++;
    if(*str == '\0')
        return str;
    end = str + strlen(str) - 1;
    while(end > str && (isspace(*end))) 
        end--;
    *(end+1) = '\0';
    return str;
}

bool pp_whitespace(char *buffer, const size_t sz) {
    if(buffer == NULL || sz < 1) 
        return false;

    char tmp[BLOCK_DEF];
    char *q = &tmp[0];
    char *p = buffer;
    size_t bc = 0;

    // trim initial whitespace
    while(isspace(*p)) {
        p++;
        bc++;
    }

    // all whitespace
    if(p == buffer+sz) 
        return false;

    /* keep non-whitespace and first whitespace
       but ignore whitespace sequences */
    while(*p != '\0') {
        if(!isspace(*p) || (isspace(*p) && !isspace(*(p+1))))
            *q++ = *p++;
        else {
            p++;
            bc++;
        }
    }

    *q = '\0';
    strncpy(buffer, tmp, sz-bc+1);
    return true;
}

char pp_control(char c) {
    if(iscntrl(c)) 
        return ' ';
    if(!isascii(c))
        return ' ';
    return c;
}

int preprocess(char **buf, size_t sz) {
    if(buf == NULL || *buf == NULL || sz < 1) {
        setlocation(LOCATION, __func__);
        errno = EINVAL;
        return false;
    }

    char *buffer = *buf;
    for(size_t i = 0; i < sz; i++) {
        buffer[i] = pp_control(buffer[i]);
        if(!opts.preserve_case) 
            buffer[i] = tolower(buffer[i]);
    }

    buffer = trim(buffer);
    sz = strlen(buffer);

    if(!pp_whitespace(buffer,sz))
        return RERROR;
    sz = strlen(buffer);
    return sz;
}
