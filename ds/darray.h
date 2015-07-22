#ifndef PHP_SLOBEL_DS_DARRAY_H
#define PHP_SLOBEL_DS_DARRAY_H 1

#include "php.h"

typedef  struct slobel_ds_darray {
	size_t count;
	size_t length;
	size_t min_length;
	size_t capacity;
    void *elements;
} slobel_ds_darray;

slobel_ds_darray *slobel_ds_darray_create(size_t size, size_t capacity);
slobel_ds_darray *slobel_ds_darray_clone(slobel_ds_darray *array);
void slobel_ds_darray_destroy(slobel_ds_darray *array);
zval *slobel_ds_darray_get(slobel_ds_darray *array, size_t index);
zval *slobel_ds_darray_set(slobel_ds_darray *array, size_t index, zval *value);
void slobel_ds_darray_unset(slobel_ds_darray *array, size_t index);

#define slobel_ds_darray_length(array) ((array)->length)
#define slobel_ds_darray_min_length(array) ((array)->min_length)
#define slobel_ds_darray_capacity(array) ((array)->capacity)
#define slobel_ds_darray_count(array) ((array)->count)
#define slobel_ds_darray_first(array) ((zval *)(array)->elements)

#endif
