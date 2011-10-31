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

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "error.h"

char location[ERRORBUF] = "\0";
char function[ERRORBUF] = "\0";

void fatal_error(const char *details, const char *f) {
    fprintf(stderr, "%s in %s() - %s\n", details, f, strerror(errno));
    exit(RERROR);
}

char *getlocation() {
    return location;
}

char *getfunction() {
    return function;
}

void setlocation(const char *l, const char *f) {
    strncpy(location, l, ERRORBUF);
    strncpy(function, f, ERRORBUF);
}
