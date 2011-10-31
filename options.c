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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <string>

#if __unix__ || MAC_OS_X
#include <unistd.h>
#include <getopt.h>
#elif WIN32
#include "win32.h"
#else
#error "Unknown operating system"
#endif

#include "lgram.h"
#include "error.h"
#include "options.h"
#include "usage.h"
#include "output.h"
#include "regex.h"

struct useropts opts;

void opts_init() {
    opts.nstart = 1;
    opts.nend = 4;
    opts.bufsz = BLOCK_DEF;
    opts.input = NULL;
    opts.input_n = 0;
    opts.input_file = NULL;
    opts.output = NULL;
    opts.output_by_n = NULL;
    opts.output_fds = NULL;
    opts.database = NULL;
    opts.boundary = ".,?!:;";
    opts.filter = NULL;

    opts.debug = false;
    opts.quiet = false;
    opts.preserve_case = false;
    opts.ign_bound = false;
    opts.no_stdout = false;
    opts.tmp_db = true;
}

int open_outfiles() {
    int fdidx, nfiles = opts.nend-opts.nstart+1;
    char namebuf[BUFSIZ];

    if((opts.output_fds = (int *) malloc(sizeof(int *)*nfiles)) == NULL) {
        perror("malloc()");
        return RERROR;
    }

    for(int i = opts.nstart; i <= opts.nend; i++)  {
        fdidx = i-opts.nstart;
        if(!opts.output_by_n)
            snprintf(namebuf, BUFSIZ, "%s", opts.output);
        else
            snprintf(namebuf, BUFSIZ, "%s-%d", opts.output_by_n, i);

        if(!opts.output_by_n && fdidx > 0) {
            opts.output_fds[fdidx] = opts.output_fds[0];
            continue;
        }

        opts.output_fds[fdidx] = open(namebuf, O_WRONLY|O_EXCL|O_CREAT, S_IRUSR|S_IWUSR);

        if(opts.output_fds[fdidx] == -1) {
            lgprintf(LG_ERROR, "%s (%s)\n", strerror(errno), namebuf);
            setlocation(LOCATION, __func__);
            return RERROR;
        }
    }
    return 0;
}

void opts_free() {
    if(opts.input) 
        free(opts.input);

    if(opts.output != NULL) {
        int nfiles = opts.output_by_n ? opts.nend-opts.nstart+1 : 1;
        for(int i = 0; i < nfiles; i++)  
            close(opts.output_fds[i]);
        free(opts.output_fds);
    }
}

int opts_check() {
    setlocation(LOCATION, __func__);
    if(opts.nstart < 1 || opts.nstart > NGRAM_MAX ||
       opts.nend < 1 || opts.nend > NGRAM_MAX) {
        lgprintf(LG_ERROR, "ngram order must be in the range 1-%d\n", NGRAM_MAX);
        lgprintf(LG_ERROR, "lgram got %d-%d\n", opts.nstart, opts.nend);
        errno = EINVAL;
        return RERROR;
    }

    if(opts.nend < opts.nstart) {
        lgprintf(LG_ERROR, "for m-n (m < n) ngram range got %d-%d\n", 
                           opts.nstart, opts.nend);
        errno = EINVAL;
        return RERROR;
    }

    if(opts.input && opts.input_file) {
        lgprintf(LG_ERROR, "cannot use -f when input files have been specified\n");
        errno = EINVAL;
        return RERROR;
    }

    if(opts.input == NULL && opts.input_file == NULL) {
        lgprintf(LG_ERROR, "no input files have been specified\n");
        errno = EINVAL;
        return RERROR;
    }

    if(opts.output || opts.output_by_n) {
        if(open_outfiles() == RERROR) {
            lgprintf(LG_ERROR, "cannot open output files\n");
            errno = EINVAL;
            return RERROR;
        }
    }

    if(opts.debug && opts.quiet) {
        lgprintf(LG_WARN, "debug mode and quiet mode? disabling quiet mode...\n");
    }

    if(opts.output_by_n && opts.output) 
        lgprintf(LG_WARN, "both output and output-by-n enabled...\n");

    if(opts.tmp_db && opts.no_stdout && !opts.output && !opts.output_by_n) 
        lgprintf(LG_WARN, "using temporary db with no output options enabled..\n");


    if(opts.regex_f) {
        if(regex_init(opts.regex_f) == RERROR)  {
            lgprintf(LG_ERROR, "cannot build regex list\n");
            perror("build_regex_list()");
            errno = EINVAL;
            return RERROR;
        }
    }
    mand_regex_init();

    return 0;
}

int opts_parse(int argc, char **argv) {
	int c, index = 0;
    char *opstr = "DqhBsuvn:b:d:o:O:f:r:";

#if __unix__ || MAC_OS_X
    const struct option lopt[] = {
        {"debug", no_argument, NULL, 'D'},
        {"quiet", no_argument, NULL, 'q'},
        {"help", no_argument, NULL, 'h'},
        {"no-stdout", no_argument, NULL, 's'},
        {"version", no_argument, NULL, 'v'},
        {"preserve-case", no_argument, NULL, 'u'},
        {"ignore-boundary", no_argument, NULL, 'B'},
        {"ngram", required_argument, NULL, 'n'},
        {"database", required_argument, NULL, 'd'},
        {"output", required_argument, NULL, 'o'},
        {"output-by-n", required_argument, NULL, 'O'},
        {"boundary", required_argument, NULL, 'b'},
        {"file-list", required_argument, NULL, 'f'},
        {0, 0, 0, 0}
    };
    while((c = getopt_long(argc, argv, opstr, lopt, &index)) != -1) {
#elif WIN32
	extern char *optarg;
	extern int optind;
	while((c = getopt(argc, argv, opstr)) != -1) {
#endif
        switch(c) {
            case 'D':
                opts.debug = true;
                break;
            case 'q':
                opts.quiet = true;
                opts.no_stdout = true;
                break;
            case 's':
                opts.no_stdout = true;
                break;
            case 'u':
                opts.preserve_case = true;
                break;
            case 'B':
                opts.ign_bound = true;
                break;
            case 'v':
                printf("%.2f\n", _VERSION_);
                exit(0);
                break;
            case 'h':
                usage(argv[0]);
                exit(0);
                break;

            case 'n':
                if(strlen(optarg) == 1 && isdigit(optarg[0]))
                    opts.nend = atoi(optarg);
                else if(strlen(optarg) == 3 && isdigit(optarg[0]) && 
                  isdigit(optarg[2])) {
                    opts.nstart = optarg[0]-48; 
                    opts.nend = optarg[2]-48;
                } else 
                    lgprintf(LG_ERROR, "cannot use ngram range "
                             "%s because it is outside the range " 
                             "of 0 < n < 10 using default of 1-4\n", optarg);
                break;
            case 'b':
                opts.boundary = optarg;
                break;
            case 'd':
                opts.database = optarg;
                opts.tmp_db = false;
                break;
            case 'f':
                opts.input_file = optarg;
                break;
            case 'r':
                opts.regex_f = optarg;
                break;
            case 'o':
                opts.output = optarg;
                break;
             case 'O':
                opts.output_by_n = optarg;
                break;
                

            case '?':
                break;
            default:
                lgprintf(LG_ERROR, "unknown options %c\n", c);
        }
    }

    opts.input_n = argc-optind;

    if(optind < argc) {
        if((opts.input = (char **)malloc(sizeof(char *)*(argc-optind+1))) == NULL)
            fatal_error(LOCATION, "malloc");

        int i = 0;
        while (optind < argc)
            opts.input[i++] = argv[optind++];
        opts.input[i] = '\0';
    }

    return opts_check();
}

