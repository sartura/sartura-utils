#ifndef MEMORY_H_ONCE
#define MEMORY_H_ONCE

#include <stdlib.h>

#define FREE_SAFE(x)                                                                                                                                 \
	do {                                                                                                                                             \
		free(x);                                                                                                                                     \
		(x) = NULL;                                                                                                                                  \
	} while (0)

void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
void *xcalloc(size_t nmemb, size_t size);
char *xstrdup(const char *s);

#endif /* MEMORY_H_ONCE */