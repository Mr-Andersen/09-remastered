#ifndef __UTILS_H__
#define __UTILS_H__

#define ERR(err) fprintf(stderr, "ERROR (%s:%i): %s\n", __FILE__, __LINE__, err);
#define FATAL(err) fprintf(stderr, "FATAL (%s:%i): %s\n", __FILE__, __LINE__, err); exit(1);

// Assert Non Zero (for asserting `malloc` output, for example)
#define ANZ(expr, msg) if ((#expr) == 0) { FATAL(#msg); }

#endif
