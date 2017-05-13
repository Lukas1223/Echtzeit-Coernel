#ifndef ERRORS_H_
#define ERRORS_H_

#include <stdio.h>		// fprintf, stderr 
#include <errno.h>		// errno
#include <string.h>		// strerror
#include <stdlib.h>		// abort

#define errno_abort(text) do { \
	fprintf(stderr, "%s at \"%s\":%d: %s\n", \
			text, __FILE__, __LINE__, strerror(errno)); \
	abort(); \
} while (0);
#define err_abort(code, text) do { \
	fprintf(stderr, "%s at \"%s\":%d: %s\n", \
			text, __FILE__, __LINE__, strerror(code)); \
	abort(); \
} while (0);

#endif /*ERRORS_H_*/
