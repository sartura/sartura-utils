/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2015-2019 Sartura Ltd.
 *
 * Author: Luka Perkov <luka.perkov@sartura.hr>
 *
 * https://www.sartura.hr/
 */

#ifndef DEBUG_H_ONCE
#define DEBUG_H_ONCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define C_RED "\x1B[31m"
#define C_YEL "\x1B[33m"
#define C_CLR "\x1B[0m"

#ifdef LOG
#define _log(fmt, ...)                                                                                                                               \
	do {                                                                                                                                             \
		fprintf(stderr, "[%u] [LOG] %s (%d): " fmt "\n", (unsigned) time(NULL), __FILENAME__, __LINE__, ##__VA_ARGS__);                              \
	} while (0)
#else
#define _log(fmt, ...)                                                                                                                               \
	do {                                                                                                                                             \
	} while (0)
#endif

#ifdef WARNING
#define _warning(fmt, ...)                                                                                                                           \
	do {                                                                                                                                             \
		fprintf(stderr,                                                                                                                              \
				"[%u] %s[WRN] %s (%d): " fmt "%s\n",                                                                                                 \
				(unsigned) time(NULL),                                                                                                               \
				isatty(STDERR_FILENO) ? C_YEL : "",                                                                                                  \
				__FILENAME__,                                                                                                                        \
				__LINE__,                                                                                                                            \
				##__VA_ARGS__,                                                                                                                       \
				isatty(STDERR_FILENO) ? C_CLR : "");                                                                                                 \
	} while (0)
#else
#define _warning(fmt, ...)                                                                                                                           \
	do {                                                                                                                                             \
	} while (0)
#endif

#ifdef ERROR
#define _error(fmt, ...)                                                                                                                             \
	do {                                                                                                                                             \
		fprintf(stderr,                                                                                                                              \
				"[%u] %s[ERR] %s (%d): " fmt "%s\n",                                                                                                 \
				(unsigned) time(NULL),                                                                                                               \
				isatty(STDERR_FILENO) ? C_RED : "",                                                                                                  \
				__FILENAME__,                                                                                                                        \
				__LINE__,                                                                                                                            \
				##__VA_ARGS__,                                                                                                                       \
				isatty(STDERR_FILENO) ? C_CLR : "");                                                                                                 \
	} while (0)
#else
#define _error(fmt, ...)                                                                                                                             \
	do {                                                                                                                                             \
	} while (0)
#endif

#ifdef DEBUG1
#define _debug(fmt, ...)                                                                                                                             \
	do {                                                                                                                                             \
		fprintf(stderr, "[%u] [DBG] %s (%d): " fmt "\n", (unsigned) time(NULL), __FILENAME__, __LINE__, ##__VA_ARGS__);                              \
	} while (0)
#else
#define _debug(fmt, ...)                                                                                                                             \
	do {                                                                                                                                             \
	} while (0)
#endif

#ifdef DEBUG2
#define __debug(fmt, ...)                                                                                                                            \
	do {                                                                                                                                             \
		fprintf(stderr, "[%u] [DBG] %s (%d): " fmt "\n", (unsigned) time(NULL), __FILENAME__, __LINE__, ##__VA_ARGS__);                              \
	} while (0)
#else
#define __debug(fmt, ...)                                                                                                                            \
	do {                                                                                                                                             \
	} while (0)
#endif

#ifdef DEBUG3
#define ___debug(fmt, ...)                                                                                                                           \
	do {                                                                                                                                             \
		fprintf(stderr, "[%u] [DBG] %s (%d): " fmt "\n", (unsigned) time(NULL), __FILENAME__, __LINE__, ##__VA_ARGS__);                              \
	} while (0)
#else
#define ___debug(fmt, ...)                                                                                                                           \
	do {                                                                                                                                             \
	} while (0)
#endif

#endif /* DEBUG_H_ONCE */
