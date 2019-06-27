/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2015-2019 Sartura Ltd.
 *
 * Author: Domagoj Pintaric <domagoj.pintaric@sartura.hr>
 *
 * https://www.sartura.hr/
 */

#ifndef LIST_H_ONCE
#define LIST_H_ONCE

#include <stddef.h>

typedef struct list_s list_t;
typedef struct list_node_s list_node_t;
typedef struct list_iterator_s list_iterator_t;

typedef enum {
	LIST_SUCCESS = 0,
	LIST_FAILURE_ARGUMENTS = -1,
	LIST_FAILURE_MEMORY = -2,
	LIST_FAILURE_EMPTY = -3,
	LIST_ITERATOR_FAILURE_END = -4,
} list_rc;

typedef enum {
	LIST_OPT_HEAD = 0x1, // start from head
	LIST_OPT_TAIL,		 // start from tail
} list_opt;

// list API
list_rc list_new(list_t **list);
list_rc list_destroy(list_t *list);
list_rc list_size_get(list_t *list, size_t *size);
list_rc list_insert(list_t *list, list_opt opt, list_node_t *list_node);
list_rc list_peek(list_t *list, list_opt opt, list_node_t **list_node);
list_rc list_remove(list_t *list, list_node_t *list_node);

// list node API
list_rc list_node_new(list_node_t **list_node, void *data);
list_rc list_node_destroy(list_node_t *list_node, void (*list_node_data_free_cb)(void *data));
list_rc list_node_data_get(list_node_t *list_node, void **data);

// list iterator API
list_rc list_iterator_new(list_t *list, list_opt opt, list_iterator_t **list_iterator);
list_rc list_iterator_destroy(list_iterator_t *list_iterator);
list_rc list_iterator_next(list_iterator_t *list_iterator, list_node_t **list_node_next);

#endif /* LIST_H_ONCE */