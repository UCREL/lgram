#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#ifdef __unix__
#include <sqlite3.h>
#endif

#include "window.h"

#define LG_NONE 0
#define LG_INFO 1
#define LG_ERROR 2
#define LG_WARN 3

typedef int error_code;

int lgprintf(error_code, const char *, ...);
int sql_init();
int sql_log_words(window_t *); 
int sql_log_ngrams(window_t *, int, int); 
int sql_log_start();
int sql_log_end();
int sql_out();
int sql_close(); 
#endif
