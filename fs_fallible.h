#ifndef __FS_FALLIBLE__
#define __FS_FALLIBLE__

#include <stdio.h>

#include "utils.h"

#define fflush_(buffer) if (fflush(buffer)) { FATAL("fflush() != 0"); }
#define fputc_(sym, buffer) if (fputc(sym, buffer) == EOF) { FATAL("fputc() == EOF"); }
#define fseek_(buffer, where, from) if (fseek(buffer, where, from)) { FATAL("fseek() != 0"); }

#endif
