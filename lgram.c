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
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

#if __unix__ || MAX_OS_X
#include <unistd.h>
#elif WIN32 
#include "win32.h"
#include "sqlite3.h"
#else
#error "Unknown system"
#endif

#include "error.h"
#include "lgram.h"
#include "input.h"
#include "output.h"
#include "blocks.h"
#include "merge_blocks.h"
#include "preprocess.h"
#include "process.h"
#include "options.h"
#include "usage.h"
#include "regex.h"

void handler(int s) {
    lgprintf(LG_NONE, "cleaning up...\n");

    if(opts.output)  {
        lgprintf(LG_INFO, "closing fd's..\n");
        int nfiles = opts.output_by_n ? opts.nend-opts.nstart+1 : 1;
        for(int i = 1; i <= nfiles; i++)  
            close(opts.output_fds[i]);
    }
    close_input();

    lgprintf(LG_INFO, "cleaning up sqlite...\n");
    sql_close();  
    exit(0);
}

unsigned long lgram(char *buffer, long len) {
    char **blocks = NULL;
    long block_n, ng_buf;

    // preprocess and filter data in accordance with option
    if((len = preprocess(&buffer, len)) == RERROR) 
        return 0;

    // split data into blocks
    if((block_n = data_to_blocks(buffer, len, &blocks)) == RERROR)
        fatal_error(getlocation(), getfunction());

    // process blocks into ngrams and store results
    for(long i = 0; i < block_n; i++) {
        lgprintf(LG_INFO, "block[%d] = %s\n", i+1, blocks[i]);
        if(process_block(blocks[i]) == RERROR)
            fatal_error(getlocation(), getfunction());
    }

    // sync stored windows from current buffer to sql
    lgprintf(LG_INFO, "syncing ngrams...\n");
    if((ng_buf = win_store_sync()) == RERROR) 
        fatal_error(getlocation(), getfunction());
    lgprintf(LG_INFO, "%d ngrams synced\n", ng_buf);

    block_free(blocks, block_n);
    free(blocks);
    return ng_buf;
}

int main(int argc, char *argv[]) {
    extern struct useropts opts;
    unsigned long ng_tl = 0, fcount = 0;
    char *c_input = NULL;
    int fd = 0;
    time_t base;
    long sz;

    char buffer[BLOCK_DEF];

    if(argc < 2) {
        usage(argv[0]);
        exit(RERROR);
    }

    signal(SIGINT, handler);
    signal(SIGTERM, handler);
#if __unix__ || MAC_OS_X
	signal(SIGHUP, handler);
    signal(SIGQUIT, handler);
#endif

    opts_init();
	if(opts_parse(argc, argv) == RERROR) 
		fatal_error(getlocation(), "opts_parse");

    base = time(0);

    // main processing loop for each input file
    while((c_input = next_input()) != NULL) {
        if((fd = open(c_input, O_RDONLY, 0)) == RERROR) {
            lgprintf(LG_ERROR, "%s\n", c_input);
            fatal_error(LOCATION, "open");
        }

        fcount++;
        lgprintf(LG_NONE, "[%d] %s ", fcount, c_input);
        fflush(stdout);

        // read input into a buffer and process buffer
        while((sz = read(fd, buffer, opts.bufsz)) > 0) {
            buffer[sz] = '\0';
            // seek to nearest block boundary or whitespace
            if((sz = seek_buffer(fd, buffer, sz)) == RERROR)
                fatal_error(LOCATION, "seek_buffer");
            ng_tl += lgram(buffer, sz);
        }
        lgprintf(LG_NONE, "[%lds]\n", time(0)-base);
    }

    if(sql_out() == RERROR) 
        fatal_error(LOCATION, "sql_out");

    if(sql_close() != SQLITE_OK) 
        fatal_error(LOCATION, "sql_close");

    time_t t = time(0)-base;
    lgprintf(LG_NONE, "\nprocessed %ld ngram%sin %lds\n", ng_tl,
                        ng_tl == 1 ? " " : "s ", t);

    opts_free();
    close_input();
    regex_deinit();
    blocks_clear();
    return 0;
}
