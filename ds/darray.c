#include "ds/darray.h"
#include "php.h"

#define ELEM_SIZE (sizeof(zval))

static inline int _slobel_ds_darray_expand(slobel_ds_darray *array, size_t index) {
    if (array && array->capacity > 0) {
        size_t capacity = array->capacity;
        size_t max_elements = array->length;
        size_t expand_count;
        if (index) {
            expand_count = ((index + 1) / capacity) * capacity + capacity;
        } else {
            expand_count = (max_elements + capacity);
        }

        zval *elements;
        if (max_elements == 0 && !array->elements) {
            elements = (zval *)emalloc(ELEM_SIZE * expand_count);
        } else {
            elements = (zval *)erealloc((void *)array->elements, ELEM_SIZE * expand_count);
        }

        if (elements) {
            zval *ptr = (elements + max_elements);
            memset(ptr, 0, array->capacity * ELEM_SIZE);

            array->length = expand_count;
            array->elements = elements;

            return 1;
        }

        return 0;
    }

    return 0;
}


slobel_ds_darray *slobel_ds_darray_create(size_t size, size_t capacity) {
    slobel_ds_darray *array = emalloc(sizeof(slobel_ds_darray));
    if (!array) {
        return NULL;
    }

    array->length = 0;
    array->min_length = size;
    array->capacity = size;
    array->count = 0;
    array->elements = NULL;

    if (size > 0 && !_slobel_ds_darray_expand(array, 0)) {
        efree(array);

        return NULL;
    }

    array->length = size;
    array->capacity = capacity;

    return array;
}


void slobel_ds_darray_destroy(slobel_ds_darray *array) {
    if (!array) {
        return;
    }

    if (array->length > 0) {
        zval *elem = (zval *)array->elements;
        while (array->length--) {
            if (elem != NULL && Z_REFCOUNT_P(elem) > 0) {
                zval_dtor(elem);
            }
            elem++;
        }
    }

    if (array->elements) {
        efree(array->elements);
    }

    efree(array);
}

slobel_ds_darray *slobel_ds_darray_clone(slobel_ds_darray *array) {
    if (!array) {
        return NULL;
    }

    slobel_ds_darray *new_array = emalloc(sizeof(slobel_ds_darray));
    if (!new_array) {
        return NULL;
    }

    new_array->count = array->count;
    new_array->length = array->length;
    new_array->min_length = array->min_length;
    new_array->capacity = array->capacity;
    new_array->elements = (zval *)emalloc(ELEM_SIZE * array->length);
    if (!new_array->elements) {
        efree(new_array);

        return NULL;
    }

    memcpy(new_array->elements, array->elements, ELEM_SIZE * array->length);
    size_t index;
    for (index = 0; index < array->length; index++) {
        zval *elem = (zval *)new_array->elements + index;
        if (elem != NULL && Z_REFCOUNT_P(elem) > 0) {
            zval_copy_ctor(elem);
        }
    }



    return new_array;

}


zval *slobel_ds_darray_get(slobel_ds_darray *array, size_t index) {
    if (!array || array->length < (index + 1)) {
        return NULL;
    }

    zval *elem = (zval *)(array->elements) + index;
    if (!elem || Z_TYPE_P(elem) == IS_NULL) {
        return NULL;
    }

    Z_UNSET_ISREF_P(elem);
    return elem;
}


void slobel_ds_darray_unset(slobel_ds_darray *array, size_t index) {
    if (!array || array->length < (index + 1)) {
        return;
    }

    zval *elem = (zval *)array->elements + index;
    if (elem != NULL && Z_REFCOUNT_P(elem) > 0) {
        if (Z_TYPE_P(elem) != IS_NULL) {
            array->count--;
        }

        zval_dtor(elem);
        *elem = (zval) {0};
    }

}


zval *slobel_ds_darray_set(slobel_ds_darray *array, size_t index, zval *value) {
    if (!array) {
        return;
    }

    if ((index + 1) > array->length) {
        if (array->capacity == 0) {
            return NULL;
        }

        if (!_slobel_ds_darray_expand(array, index)) {
            return NULL;
        }
    }
    zval *elem = (zval *)array->elements + index;
    int prev_is_not_null = 0;
    if (Z_REFCOUNT_P(elem) > 0 && Z_TYPE_P(elem)) {
        zval_dtor(elem);
        prev_is_not_null = 1;
    }

    elem->value = value->value;
    elem->type  = value->type;
    elem->refcount__gc = 1;
    elem->is_ref__gc = 0;
    zval_copy_ctor(elem);

    if (prev_is_not_null && Z_TYPE_P(elem) == IS_NULL) {
        array->count--;
    }
    else if (!prev_is_not_null && Z_TYPE_P(elem) != IS_NULL) {
        array->count++;
    }


    return elem;
}
