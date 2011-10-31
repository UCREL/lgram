#ifndef _WIN32_H_
#define _WIN32_H_

#include <windows.h>
#include <io.h>

#define _VERSION_ 0.01
#undef strdup
#define strdup _strdup
#define snprintf _snprintf
#define lseek _lseek
#define close _close
#define open _open
#define read _read
#define write _write
#define S_IRUSR _S_IREAD
#define S_IWUSR _S_IWRITE

#define __func__ __FUNCTION__

int mkstemp (char *);
int getopt(int , char *[], const char *);

#endif