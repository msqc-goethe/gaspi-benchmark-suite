#ifndef __ARRAYLIST_H__
#define __ARRAYLIST_H__

#include "utility.h"

#define ARRAYLIST_INCREASE_CAPACITY_NUM 16

typedef struct arraylist_s {
	void **data;
	unsigned int length;
	unsigned int capacity;
} arraylist_t;

typedef void (*arraylist_free_func)(void *);

arraylist_t * arraylist_create(void);
arraylist_t * arraylist_delete(arraylist_t *list);
arraylist_t * arraylist_deleteFull(arraylist_t *list, arraylist_free_func f);
void arraylist_add(arraylist_t *list, void *element);
void *arraylist_get(arraylist_t *list, unsigned int idx);
unsigned int arraylist_getLength(arraylist_t *list);
boolean arraylist_isEmpty(arraylist_t *list);
void arraylist_printString(arraylist_t *list);
void arraylist_freeElementDefault(void *o);
void arraylist_freeNoop(void *o);



#endif