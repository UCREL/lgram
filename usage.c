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

char *usage_str[][4] = { 
    {0,0,0,"basic options"},
    {"-n", "--ngram", "<start-end|end>", "order of ngram to compute [1-4]"},
    {0,0,0,"file IO"},
    {"-f", "--file-list", "<filename>", "a file containing a list of files to process"},
    {"-d", "--database", "<dbname>", "save to a permanent db [temporary db]"},
    {"-o", "--output-file", "<filename>", "save ngrams to file"},
    {"-O", "--output-by-n", "<filename>", "save ngrams to n files"},
    {0,0,0,"preprocessing"},
    {"-r", "--regex--file", "<filename>", "apply regex file to each block (see README)"},
    {"-b", "--boundary", "<str>", "characters of str are boundary characters [.,?!:;]"},
    {"-B", "--ignore-boundary", "", "ignore ngrams containing boundary characters"},
    {"-u", "--preserve-case", "", "preserve case [lower case]"},
    {0,0,0,"other"},
    {"-s", "--no-stdout", "", "do not print ngrams"},
    {"-q", "--quiet", "", "do not print anything except errors"},
    {"-D", "--debug", "", "activate debug mode"},
    {"-v", "--version", "", "display version and exit"},
    {"-h", "--help", "", "this message"},
    {0,0,0,0}
};

char *pad(char *str, int max, char *nstr) {
    assert(strlen(str)+max+1 < BUFSIZ);
    strcpy(nstr, str);
    int space = max-strlen(str)+1;
    while(space--) 
        strcat(nstr, " ");
    return nstr;
}

void usage(char *name) {
    int tmp, max_long = 0, max_arg = 0;
    for(int i = 0; usage_str[i][3] != NULL; i++) {
        // skip headers
        if(usage_str[i][1] == NULL)
            continue;
        tmp = strlen(usage_str[i][1]);
        if(tmp > max_long)
            max_long = tmp;
        tmp = strlen(usage_str[i][2]);
        if(tmp > max_arg)
            max_arg = tmp;
    }

#if __unix__ || MAC_OS_X
    char pad_l[BUFSIZ];
#endif
	char pad_a[BUFSIZ];

    printf("%s-%.2f [options] file,file,...\n", name, _VERSION_);
    for(int i = 0; usage_str[i][3] != NULL; i++) {
        if(usage_str[i][0] == NULL && usage_str[i][3] != NULL)
            printf("\n   %s\n", usage_str[i][3]);
        else
#if __unix__ || MAC_OX_X
            printf("    %s %s %s %s\n", 
                   usage_str[i][0], pad(usage_str[i][1],max_long,pad_l),
                   pad(usage_str[i][2],max_arg,pad_a), usage_str[i][3]);
#elif WIN32
            printf("    %s %s %s\n", 
                   usage_str[i][0], pad(usage_str[i][2],max_arg,pad_a), 
                   usage_str[i][3]);
#else
#error "Unknow System"
#endif

    }
    printf("\n");
}
