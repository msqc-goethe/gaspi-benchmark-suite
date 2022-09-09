#include "arraylist.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "utility.h"

void arraylist_freeElementDefault(void *o) {
	if (o != NULL) {
		free(o);
	}
}

void arraylist_freeNoop(void *o) {
	UNUSED(o);
}

arraylist_t * arraylist_create(void) {
	arraylist_t * list = (arraylist_t *) safeMalloc(sizeof(arraylist_t));
	list->length = 0;
	list->capacity = 0;
	list->data = NULL;
	return list;
}

arraylist_t * arraylist_delete(arraylist_t *list) {
	if (list != NULL) {
		if (list->data != NULL) {
			free(list->data);
			list->data = NULL;
		}
		free(list);
	}
	return NULL;
}

arraylist_t * arraylist_deleteFull(arraylist_t *list, arraylist_free_func f) {
	if (list != NULL) {
		unsigned int i;
		for (i = 0; i < list->length; ++i) {
			void *elem = (list->data)[i];
			f(elem);
		}
		list = arraylist_delete(list);
	}
	return NULL;
}

void arraylist_add(arraylist_t *list, void *element) {
	assert(list != NULL);
	if ((list->length + 1) > list->capacity) {
		list->data = realloc(list->data, sizeof(*(list->data)) * (list->capacity + ARRAYLIST_INCREASE_CAPACITY_NUM));
		checkAlloc(list->data);
		list->capacity += ARRAYLIST_INCREASE_CAPACITY_NUM;
	}
	(list->data)[(list->length)++] = element;
}

void *arraylist_get(arraylist_t *list, unsigned int idx) {
	assert(list != NULL);
	if (idx < (list->length)) {
		return (list->data)[idx];
	}
	assert(false);
	return NULL;
}

unsigned int arraylist_getLength(arraylist_t *list) {
	assert(list != NULL);
	return list->length;
}

boolean arraylist_isEmpty(arraylist_t *list) {
	assert(list != NULL);
	return (arraylist_getLength(list) == 0);
}

void arraylist_printString(arraylist_t *list) {
	unsigned int i;
	assert(list != NULL);
	for (i = 0; i < arraylist_getLength(list); ++i) {
		printf("[%d] -> %s\n", i, (char *) arraylist_get(list, i));
	}
}