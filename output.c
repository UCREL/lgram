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

#include <stdarg.h>
#include "lgram.h"
#include "output.h"
#if WIN32
#include "sqlite3.h"
#endif
#include "options.h"

int lgprintf(error_code c, const char *format, ...) {
    extern struct useropts opts;

    char dbuf[BUFSIZ];
    int len = 0;

    switch(c) {
        case LG_ERROR:
            len = snprintf(dbuf, BUFSIZ, "ERROR: ");
            break;
        case LG_WARN:
            len = snprintf(dbuf, BUFSIZ, "WARNING: ");
            break;
        case LG_INFO:
            len = snprintf(dbuf, BUFSIZ, "INFO: ");
            break;
        default:
            len = 0;
    }

    if(!opts.debug && c == LG_INFO)
        return 0;
    if(opts.quiet && (c == LG_INFO || c == LG_NONE))  
        return 0;

    va_list args;
    int r;

    va_start(args, format);
    vsnprintf(dbuf+len, BUFSIZ-len, format, args);
    if(c == LG_NONE)
        r = printf("%s", dbuf);
    else 
        r = fprintf(stderr, "%s", dbuf);
    va_end(args);
    return r;
}

