/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2015-2019 Sartura Ltd.
 *
 * Author: Domagoj Pintaric <domagoj.pintaric@sartura.hr>
 *
 * https://www.sartura.hr/
 */

#include <stddef.h>
#include <stdlib.h>

#include "list.h"

struct list_s {
	size_t size;
	list_node_t *head;
	list_node_t *tail;
};

struct list_node_s {
	void *data;
	list_node_t *next;
	list_node_t *previous;
};

struct list_iterator_s {
	list_node_t *list_node_current;
	list_opt opt;
};

list_rc list_new(list_t **list)
{
	if (list == NULL) {
		return LIST_FAILURE_ARGUMENTS;
	}

	*list = calloc(1, sizeof(list_t));
	if (*list == NULL) {
		return LIST_FAILURE_MEMORY;
	}

	(*list)->size = 0;
	(*list)->head = NULL;
	(*list)->tail = NULL;

	return LIST_SUCCESS;
}

list_rc list_destroy(list_t *list)
{
	if (list == NULL) {
		return LIST_FAILURE_ARGUMENTS;
	}

	free(list);

	return LIST_SUCCESS;
}

list_rc list_size_get(list_t *list, size_t *size)
{
	if (list == NULL || size == NULL) {
		return LIST_FAILURE_ARGUMENTS;
	}

	*size = list->size;

	return LIST_SUCCESS;
}

list_rc list_insert(list_t *list, list_opt opt, list_node_t *list_node)
{
	if (list == NULL || (opt != LIST_OPT_HEAD && opt != LIST_OPT_TAIL) || list_node == NULL) {
		return LIST_FAILURE_ARGUMENTS;
	}

	// list is empty
	if (list->head == NULL && list->tail == NULL) {
		list->head = list_node;
		list->tail = list_node;
		list->size++;

		return LIST_SUCCESS;
	}

	list_node_t *tmp = opt & LIST_OPT_HEAD ? list->head : list->tail;

	list_node->next = opt & LIST_OPT_HEAD ? tmp : NULL;
	list_node->previous = opt & LIST_OPT_HEAD ? NULL : tmp;

	switch (opt) {
		case LIST_OPT_HEAD:
			tmp->previous = list_node;
			list->head = list_node;
			break;
		case LIST_OPT_TAIL:
			tmp->next = list_node;
			list->tail = list_node;
			break;
	}

	list->size++;

	return LIST_SUCCESS;
}

list_rc list_peek(list_t *list, list_opt opt, list_node_t **list_node)
{
	if (list == NULL || (opt != LIST_OPT_HEAD && opt != LIST_OPT_TAIL) || list_node == NULL) {
		return LIST_FAILURE_ARGUMENTS;
	}

	if (list->head == NULL && list->tail == NULL) {
		return LIST_FAILURE_EMPTY;
	}

	*list_node = opt & LIST_OPT_HEAD ? list->head : list->tail;

	return LIST_SUCCESS;
}

list_rc list_remove(list_t *list, list_node_t *list_node)
{
	if (list == NULL || list_node == NULL) {
		return LIST_FAILURE_ARGUMENTS;
	}

	// unlink from list
	if (list_node->previous != NULL) {
		list_node->previous->next = list_node->next;
	} else {
		list->head = list_node->next;
	}

	if (list_node->next != NULL) {
		list_node->next->previous = list_node->previous;
	} else {
		list->tail = list_node->previous;
	}

	list->size--;

	return LIST_SUCCESS;
}

list_rc list_node_new(list_node_t **list_node, void *data)
{
	if (list_node == NULL || data == NULL) {
		return LIST_FAILURE_ARGUMENTS;
	}

	*list_node = calloc(1, sizeof(list_node_t));
	if (*list_node == NULL) {
		return LIST_FAILURE_MEMORY;
	}

	(*list_node)->data = data;
	(*list_node)->previous = NULL;
	(*list_node)->next = NULL;

	return LIST_SUCCESS;
}

list_rc list_node_destroy(list_node_t *list_node, void (*list_node_data_free_cb)(void *data))
{
	if (list_node == NULL) {
		return LIST_FAILURE_ARGUMENTS;
	}

	if (list_node_data_free_cb) {
		list_node_data_free_cb(list_node->data);
	}
	list_node->data = NULL;
	list_node->previous = NULL;
	list_node->next = NULL;
	free(list_node);

	return LIST_SUCCESS;
}

list_rc list_node_data_get(list_node_t *list_node, void **data)
{
	if (list_node == NULL || data == NULL) {
		return LIST_FAILURE_ARGUMENTS;
	}

	*data = list_node->data;

	return LIST_SUCCESS;
}

list_rc list_iterator_new(list_t *list, list_opt opt, list_iterator_t **list_iterator)
{
	if (list == NULL || (opt != LIST_OPT_HEAD && opt != LIST_OPT_TAIL) || list_iterator == NULL) {
		return LIST_FAILURE_ARGUMENTS;
	}

	*list_iterator = calloc(1, sizeof(list_iterator_t));
	if (*list_iterator == NULL) {
		return LIST_FAILURE_MEMORY;
	}

	if (list_peek(list, opt, &((*list_iterator)->list_node_current)) == LIST_FAILURE_EMPTY) {
		list_iterator_destroy(*list_iterator);
		return LIST_FAILURE_EMPTY;
	}
	(*list_iterator)->opt = opt;

	return LIST_SUCCESS;
}

list_rc list_iterator_destroy(list_iterator_t *list_iterator)
{
	if (list_iterator == NULL) {
		return LIST_FAILURE_ARGUMENTS;
	}

	free(list_iterator);

	return LIST_SUCCESS;
}

list_rc list_iterator_next(list_iterator_t *list_iterator, list_node_t **list_node_next)
{
	if (list_iterator == NULL || list_node_next == NULL) {
		return LIST_FAILURE_ARGUMENTS;
	}

	*list_node_next = list_iterator->list_node_current;
	if (*list_node_next == NULL) {
		return LIST_ITERATOR_FAILURE_END;
	}

	list_iterator->list_node_current =
		list_iterator->opt == LIST_OPT_HEAD ? list_iterator->list_node_current->next : list_iterator->list_node_current->previous;

	return LIST_SUCCESS;
}