#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#if __unix__ || MAC_OS_X
#include <unistd.h>
#elif WIN32 
#include "win32.h"
#endif

struct useropts{    
    unsigned short nstart;
    unsigned short nend;
    unsigned long input_n;
    long bufsz;

    char **input;
    char *input_file;
    char *output;
    char *output_by_n;
    char *database;
    char *regex_f;

    int *output_fds;

    char **filter;
    char *boundary;

    bool debug;
    bool ign_bound;
    bool quiet;
    bool preserve_case;
    bool no_stdout;
    bool tmp_db;

};

extern struct useropts opts;

void opts_init();
int opts_parse(int, char *[]);
void opts_free();
#endif
