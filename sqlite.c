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


#if __unix__ || MAX_OS_X
#include <unistd.h>
#include <sqlite3.h>
#elif WIN32 
#include "win32.h"
#include "sqlite3.h"
#else
#error "Unknown operating system"
#endif

#include "lgram.h"
#include "error.h"
#include "window.h"
#include "output.h"
#include "options.h"
#include "blocks.h"

bool has_init = false;
sqlite3 *sqfd;

int execute_query_op(char *q, bool ignore) {
    if(!has_init)  {
        errno = EINVAL;
        return RERROR;
    }

    char *error = NULL;
    sqlite3_exec(sqfd, q, 0, 0, &error);

    if(error != NULL) {
        if(!ignore) {
            lgprintf(LG_ERROR, "%s\n", error);
            lgprintf(LG_ERROR, "statement = %s\n", q);
        }

        sqlite3_free(error);

        if(!ignore) {
            errno = EINVAL;
            setlocation(LOCATION, "execute_query");
        }
        return RERROR;
    }
    return 0;
}

int execute_query(char *q) {
    return execute_query_op(q, false);
}

int build_template(char *fmt, int s, int e, char *b, size_t sz) {
    assert(strlen(fmt)+e-s < sz);
    size_t len = 0;
    for(int i = s; i <= e; i++) 
        len += snprintf(b+len, sz, fmt, i);
    return len;
}

int sql_buildtables(bool ignore) {
    if(!has_init) {
        errno = EINVAL;
        fatal_error(LOCATION, __func__);
        return RERROR;
    }

    char q[BUFSIZ], n_cols[BUFSIZ];
    size_t len, len_tpl;

    // create table of words
    snprintf(q, BUFSIZ, "CREATE TABLE IF NOT EXISTS words "
                "(id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "word TEXT NOT NULL UNIQUE);"
                "CREATE UNIQUE INDEX idx_words ON words (word)");
    if(execute_query_op(q, ignore)==RERROR && !ignore) 
        return RERROR;

    for(int i = opts.nstart; i <= opts.nend; i++) {
        len_tpl = build_template("n%d INTEGER,", 1, i, n_cols, BUFSIZ);
        n_cols[len_tpl-1]='\0';
        len = snprintf(q, BUFSIZ, "CREATE TABLE ngram%d "
                          "(%s)",i,n_cols);
        if(execute_query_op(q, ignore)==RERROR && !ignore) 
            return RERROR;
    }
    return 0;
}

int sql_temp_db_name() {
    char tpname[] = TMP_DB_NAME;
    int fd;

    // create random temp file
    if((fd = mkstemp(tpname)) == RERROR) {
        setlocation(LOCATION, "mkstemp");
        return RERROR;
    }

    close(fd);

    // save name in options structure
    if((opts.database = (char *) malloc(strlen(tpname)+1)) == NULL) {
        setlocation(LOCATION, "malloc");
        return RERROR;
    }

    strcpy(opts.database, tpname);
    return 0;
}

int sql_init() {
    if(has_init) 
        return 0;

    bool db_exists = false;

    // generate tmp db name
    if(!opts.database) {
        if(sql_temp_db_name() == RERROR) 
            return RERROR;
    } else {
        // check if user specified db exists
        int ret = sqlite3_open_v2(opts.database, &sqfd, 
                          SQLITE_OPEN_READWRITE, NULL);
        if(ret == SQLITE_CANTOPEN) {
            ; // db doesn't exist
        } else if(ret == SQLITE_OK) {
            db_exists = true;
            sqlite3_close(sqfd);
        } else {
            setlocation(LOCATION, "sqlite3_open");
            return RERROR;
        }
    }

    if(sqlite3_open(opts.database, &sqfd) != SQLITE_OK) {
        setlocation(LOCATION, "sqlite3_open");
        return RERROR;
    }

    has_init = true;
    // a few optimisations
    if(execute_query("PRAGMA locking_mode=EXCLUSIVE")==RERROR)
        return RERROR;
    if(execute_query("PRAGMA synchronous=OFF")==RERROR)
        return RERROR;
    if(execute_query("PRAGMA count_changes=OFF")==RERROR)
        return RERROR;
    if(execute_query("PRAGMA cache_size=20000")==RERROR)
        return RERROR;
    if(execute_query("PRAGMA automatic_index=OFF")==RERROR)
        return RERROR;
    if(execute_query("PRAGMA temp_store=MEMORY")==RERROR)
        return RERROR;

    if(sql_buildtables(db_exists) == RERROR)
        return RERROR;

    return 0;
}

inline int build_select(int n, window_t *w, int start, char **buf) {
    if((*buf = (char *) malloc(n*500*sizeof(char)))== NULL)
        return RERROR;

    char *b = *buf;
    size_t len = 0;
    
    len += build_template("w%d.id,", start, start+n-1, b+len, n*500);
    len += sprintf((--b)+len, " FROM ");
    len +=build_template("words as w%d,", start, start+n-1, b+len, n*500);
    len += sprintf((--b)+len, " WHERE ");

    for(int i = start; i < start+n; i++) {
        char *q = sqlite3_mprintf("%Q", w->words[i]);
        len += sprintf(b+len, "w%d.word=%s AND ", i, q);
        assert(len < n*500*sizeof(char));
        sqlite3_free(q);
    }
    len-=5;
    *(b+len) = '\0';
    return 0;
}

int sql_log_start() {
    if(!has_init) {
        if(sql_init() == RERROR) 
            return RERROR;
    }

    if(execute_query("BEGIN TRANSACTION;") == RERROR)
        return RERROR;
    return 0;
}

int sql_log_end() {
    if(!has_init) {
        errno = EINVAL;
        return RERROR;
    }

    if(execute_query("COMMIT;") == RERROR)
        return RERROR;
    return 0;
}

// insert all types from the windows into the word table
int sql_log_words(window_t *w) {
    if(!has_init) {
        return RERROR;
    }

    char *q = NULL;
    // insert types into word table
    for(size_t i = 0; i < w->wc; i++) {
        q = sqlite3_mprintf("INSERT OR IGNORE INTO words(word)"
                            " VALUES (%Q)", w->words[i]);
        if(execute_query(q) == RERROR) {
            lgprintf(LG_ERROR, "Is %s an lgram database?\n", opts.database);
            return RERROR;
        }
        sqlite3_free(q);
    }
    return 0;
}

// use the word table id's to insert ngram ids into the ngram tables
int sql_log_ngrams(window_t *w, int start, int n) {
    char *q = NULL, *select_stmt = NULL;
    if(!has_init) {
        return RERROR;
    }

    // ignore ngrams which contain boundary characters
    if(opts.ign_bound) {
        for(int i = start; i < start+n; i++) {
            if(block_match(w->words[i][0]))
                return 0;
        }
    }

    // insert ngrams into ngram table based on word table keys
    build_select(n, w, start, &select_stmt);
    q = sqlite3_mprintf("INSERT INTO ngram%d SELECT %s", 
                        n, select_stmt);
    if(execute_query(q) == RERROR) {
        lgprintf(LG_ERROR, "%s cannot hold %dgrams\n", opts.database, n);
        return RERROR;
    }
    sqlite3_free(q);
    free(select_stmt);
    return 0;
}

int out_callback(void *a_param, int argc, char **argv, char **column){
        static char ngram[BUFSIZ];
        size_t len = 0;
        int n = *(int *)a_param;

        for (int i=0; i< argc; i++) 
            len += snprintf(ngram+len, BUFSIZ-len, "%s ", argv[i]);

        len-=1;
        len += snprintf(ngram+len, BUFSIZ-len, "\n");
            
        if(!opts.no_stdout)
            lgprintf(LG_NONE, "%s", ngram);

        if(opts.output || opts.output_by_n) {
            if(write(opts.output_fds[n-opts.nstart], ngram, len)==-1) {
                perror("write()");
            }
        }

    return 0;
}

// after ngram computation has finished output the ngrams
int sql_out() {
    if(!has_init) 
        return 0;

    if(opts.no_stdout && !opts.output && !opts.output_by_n) 
        return 0;

    /* the memory usage skyrockets during the select calls *
     * if we do not close and open the database */
    if(sqlite3_close(sqfd) != SQLITE_OK) {
        setlocation(LOCATION, "sqlite3_close");
        return RERROR;
    }

    if(sqlite3_open(opts.database, &sqfd) != SQLITE_OK) {
        setlocation(LOCATION, "sqlite3_open");
        return RERROR;
    }

    char b[BUFSIZ], *error = NULL;
    size_t len = 0;

    // a little messy :O
    for(int i = opts.nstart; i <= opts.nend; i++) {
        len = snprintf(b+len, BUFSIZ-len, "SELECT DISTINCT ");
        len += build_template("w%d.word,", 1, i, b+len, BUFSIZ-len);
        len += snprintf(b+len, BUFSIZ-len, "COUNT(*) as freq FROM ");
        len += build_template("words as w%d,", 1, i, b+len, BUFSIZ-len);
        len += snprintf(b+len, BUFSIZ-len, "ngram%d WHERE ",i);
        for(int j = 1; j <= i; j++) 
            len += snprintf(b+len, BUFSIZ-len, "w%d.id = n%d AND ",j,j);
        len-=4;
        len += snprintf(b+len, BUFSIZ, "GROUP BY ");
        len += build_template("n%d,", 1, i, b+len,BUFSIZ-len)-1;
        len += snprintf(b+len, BUFSIZ, " ORDER BY freq DESC,");
        len += build_template("w%d.word,", 1, i, b+len,BUFSIZ-len)-1;
        b[len]='\0';

        lgprintf(LG_NONE, "[+] building %dgram frequency list\n", i);

        sqlite3_exec(sqfd, b, out_callback, &i, &error);
        if(error != NULL) {
            lgprintf(LG_ERROR, "%s\n", error);
            sqlite3_free(error);
            errno = EINVAL;
            return RERROR;
        }
    }
    return 0;
}

int sql_close() {
    if(!has_init)
        return 0;

    sqlite3_interrupt(sqfd);
    execute_query_op("VACUUM;", true);

    int ret = sqlite3_close(sqfd);
    has_init = false;

    // if we generated a temp db then remove it
    if(opts.tmp_db) {
        lgprintf(LG_INFO, "removing tmp db %s\n", opts.database);
        if(remove(opts.database) == RERROR)
            return RERROR;

        char buf[BUFSIZ];
        snprintf(buf, BUFSIZ, "%s-journal", opts.database);
        remove(buf);
        free(opts.database);
    }

    if(ret != SQLITE_OK)
        return RERROR;
    return ret;
}
