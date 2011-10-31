#ifndef _ERROR_H_
#define _ERROR_H_

#include <errno.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define LOCATION __FILE__ ":" TOSTRING(__LINE__)

#define RERROR -1
#define ERRORBUF 1024

void fatal_error(const char *, const char *);
void setlocation(const char *, const char *);
char *getlocation();
char *getfunction();

#endif
