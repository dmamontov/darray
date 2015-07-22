#include "php.h"
#include "zend_interfaces.h"
#include "slobel_darray.h"
#include "ds/darray.h"

zend_class_entry *slobel_darray_ce;
zend_object_handlers slobel_darray_handlers;

typedef struct slobel_darray {
    zend_object std;
    slobel_ds_darray *array;
} slobel_darray;

typedef struct _slobel_darray_iterator_data {
    zval *object_zval;
    slobel_darray *object;
    size_t offset;
    zval *current;
} slobel_darray_iterator_data;

static inline long zval_to_long(zval *zv) {
    if (Z_TYPE_P(zv) == IS_LONG) {
        return Z_LVAL_P(zv);
    } else {
        zval tmp = *zv;
        zval_copy_ctor(&tmp);
        convert_to_long(&tmp);
        return Z_LVAL(tmp);
    }
}

static void slobel_darray_free_object_storage(slobel_darray *intern TSRMLS_DC) {
    zend_object_std_dtor(&intern->std TSRMLS_CC);

    if (intern->array) {
        slobel_ds_darray_destroy(intern->array);
    }

    efree(intern);
}

zend_object_value slobel_darray_create_object(zend_class_entry *class_type TSRMLS_DC) {
    zend_object_value retval;

    slobel_darray *intern = emalloc(sizeof(slobel_darray));
    memset(intern, 0, sizeof(slobel_darray));

    zend_object_std_init(&intern->std, class_type TSRMLS_CC);
    object_properties_init(&intern->std, class_type);

    retval.handle = zend_objects_store_put(
        intern,
        (zend_objects_store_dtor_t) zend_objects_destroy_object,
        (zend_objects_free_object_storage_t) slobel_darray_free_object_storage,
        NULL TSRMLS_CC
    );

    retval.handlers = &slobel_darray_handlers;

    return retval;
}


static zend_object_value slobel_darray_clone(zval *object TSRMLS_DC) {
    slobel_darray *old_object = zend_object_store_get_object(object TSRMLS_CC);

    zend_object_value new_object_val = slobel_darray_create_object(Z_OBJCE_P(object) TSRMLS_CC);
    slobel_darray *new_object = zend_object_store_get_object_by_handle(new_object_val.handle TSRMLS_CC);

    zend_objects_clone_members(
        &new_object->std, new_object_val,
        &old_object->std, Z_OBJ_HANDLE_P(object) TSRMLS_CC
    );

    new_object->array = slobel_ds_darray_clone(old_object->array);

    if (!new_object->array) {
        zend_throw_exception(NULL, "Failed to clone slobel_darray", 0 TSRMLS_CC);
    }

    return new_object_val;
}

static zval *slobel_darray_read_dimension(zval *object, zval *zv_offset, int type TSRMLS_DC) {
    slobel_darray *intern = zend_object_store_get_object(object TSRMLS_CC);
    
    if (intern->std.ce->parent) {
        return zend_get_std_object_handlers()->read_dimension(object, zv_offset, type TSRMLS_CC);
    }

    if (!zv_offset) {
        zend_throw_exception(NULL, "Cannot append to a slobel_darray", 0 TSRMLS_CC);
        return NULL;
    }

    long offset = zval_to_long(zv_offset);
    if (offset < 0 || offset > slobel_ds_darray_length(intern->array)) {
        zend_throw_exception(NULL, "Offset out of range", 0 TSRMLS_CC);
        return NULL;
    }

    zval *return_value;
    zval *value = slobel_ds_darray_get(intern->array, offset);

    if (value) {

        if (type == BP_VAR_W) {
            return_value = value;
            Z_SET_ISREF_P(return_value);
        } else {
            MAKE_STD_ZVAL(return_value);
            ZVAL_ZVAL(return_value, value, 1, 0);
            Z_DELREF_P(return_value);
        }
    } else {
        MAKE_STD_ZVAL(return_value);
        ZVAL_NULL(return_value);
        Z_DELREF_P(return_value);
    }

    return return_value;
}


static void slobel_darray_write_dimension(zval *object, zval *zv_offset, zval *value TSRMLS_DC) {
    slobel_darray *intern = zend_object_store_get_object(object TSRMLS_CC);

    if (intern->std.ce->parent) {
        return zend_get_std_object_handlers()->write_dimension(object, zv_offset, value TSRMLS_CC);
    }


    if (!zv_offset) {
        zend_throw_exception(NULL, "Cannot append to a slobel_darray", 0 TSRMLS_CC);
    }

    long offset = zval_to_long(zv_offset);
    if (offset < 0) {
        zend_throw_exception(NULL, "Offset out of range", 0 TSRMLS_CC);
    }

    zval *saved_val = slobel_ds_darray_set(intern->array, (size_t)offset, value);
    if (saved_val == NULL) {
        zend_throw_exception(NULL, "Error occured during dimension write", 0 TSRMLS_CC);
    }
}


static int slobel_darray_has_dimension(zval *object, zval *zv_offset, int check_empty TSRMLS_DC) {
    slobel_darray *intern = zend_object_store_get_object(object TSRMLS_CC);

    if (intern->std.ce->parent) {
        return zend_get_std_object_handlers()->has_dimension(object, zv_offset, check_empty TSRMLS_CC);
    }

    long offset = zval_to_long(zv_offset);
    if (offset < 0 || offset > slobel_ds_darray_length(intern->array)) {
        return 0;
    }

    if (check_empty) {
        zval *value = slobel_ds_darray_get(intern->array, offset);
        if (value == NULL) {
            return 0;
        }

        return zend_is_true(value);
    }

}

static void slobel_darray_unset_dimension(zval *object, zval *zv_offset TSRMLS_DC) {
    slobel_darray *intern = zend_object_store_get_object(object TSRMLS_CC);

    if (intern->std.ce->parent) {
        return zend_get_std_object_handlers()->unset_dimension(object, zv_offset TSRMLS_CC);
    }

    long offset = zval_to_long(zv_offset);
    if (offset < 0 || offset > slobel_ds_darray_length(intern->array)) {
        zend_throw_exception(NULL, "Offset out of range", 0 TSRMLS_CC);
    }

    slobel_ds_darray_unset(intern->array, offset);
}

int slobel_darray_count_elements(zval *object, long *count TSRMLS_DC) {
    slobel_darray *intern = zend_object_store_get_object(object TSRMLS_CC);

    if (intern->std.ce->parent) {
        return zend_get_std_object_handlers()->count_elements(object, count TSRMLS_CC);
    }

    if (intern && intern->array) {
        *count = (long)slobel_ds_darray_count(intern->array);
        return SUCCESS;
    } else {
        *count = 0;
        return FAILURE;
    }
}


static void slobel_darray_iterator_dtor(zend_object_iterator *intern TSRMLS_DC) {
    slobel_darray_iterator_data *data = (slobel_darray_iterator_data *)intern->data;

    if (data->current != NULL) {
        zval_ptr_dtor(&data->current);
    }

    zval_ptr_dtor((zval **)&data->object_zval);
    efree(data);
    efree(intern);
}

static int slobel_darray_iterator_valid(zend_object_iterator *intern TSRMLS_DC) {
    slobel_darray_iterator_data *data = (slobel_darray_iterator_data *)intern->data;

    return slobel_ds_darray_length(data->object->array) > data->offset ? SUCCESS : FAILURE;
}

static void slobel_darray_iterator_get_current_data(zend_object_iterator *intern, zval ***data TSRMLS_DC) {
    slobel_darray_iterator_data *iter_data = (slobel_darray_iterator_data *)intern->data;

    if (iter_data->current != NULL) {
        zval_ptr_dtor(&iter_data->current);
        iter_data->current = NULL;
    }

    if (iter_data->offset < slobel_ds_darray_length(iter_data->object->array)) {
        zval *value = slobel_ds_darray_get(iter_data->object->array, iter_data->offset);
        if (value != NULL) {
            MAKE_STD_ZVAL(iter_data->current);
            ZVAL_ZVAL(iter_data->current, value, 1, 0);

            *data = &iter_data->current;
        } else {
            *data = NULL;
        }

    } else {
        *data = NULL;
    }
}


#if ZEND_MODULE_API_NO >= 20121212
static void slobel_darray_iterator_get_current_key(zend_object_iterator *intern, zval *key TSRMLS_DC) {
    slobel_darray_iterator_data *data = (slobel_darray_iterator_data *) intern->data;
    ZVAL_LONG(key, data->offset);
}
#else
static int slobel_darray_iterator_get_current_key(
    zend_object_iterator *intern, char **str_key, uint *str_key_len, ulong *int_key TSRMLS_DC
) {
    slobel_darray_iterator_data *data = (slobel_darray_iterator_data *) intern->data;

    *int_key = (ulong) data->offset;
    return HASH_KEY_IS_LONG;
}
#endif

static void slobel_darray_iterator_move_forward(zend_object_iterator *intern TSRMLS_DC) {
    slobel_darray_iterator_data *data = (slobel_darray_iterator_data *) intern->data;

    data->offset++;
}

static void slobel_darray_iterator_rewind(zend_object_iterator *intern TSRMLS_DC)
{
    slobel_darray_iterator_data *data = (slobel_darray_iterator_data *) intern->data;

    data->offset = 0;
    data->current = NULL;
}

static zend_object_iterator_funcs slobel_darray_iterator_funcs = {
    slobel_darray_iterator_dtor,
    slobel_darray_iterator_valid,
    slobel_darray_iterator_get_current_data,
    slobel_darray_iterator_get_current_key,
    slobel_darray_iterator_move_forward,
    slobel_darray_iterator_rewind,
    NULL
};

zend_object_iterator *slobel_darray_get_iterator(zend_class_entry *ce, zval *object, int by_ref TSRMLS_DC) {
    zend_object_iterator *iter;
    slobel_darray_iterator_data *iter_data;

    if (by_ref) {
        zend_throw_exception(NULL, "UPS, no by reference iteration!", 0 TSRMLS_CC);
        return NULL;
    }

    iter = emalloc(sizeof(zend_object_iterator));
    iter->funcs = &slobel_darray_iterator_funcs;

    iter_data = emalloc(sizeof(slobel_darray_iterator_data));
    iter_data->object_zval = object;
    Z_ADDREF_P(object);

    iter_data->object = zend_object_store_get_object(object TSRMLS_CC);
    iter_data->offset = 0;
    iter_data->current = NULL;

    iter->data = iter_data;

    return iter;
}


PHP_METHOD(slobel_darray, __construct)
{
    slobel_darray *intern;
    long size = 0;
    long capacity = 0;

    zend_error_handling error_handling;

    zend_replace_error_handling(EH_THROW, NULL, &error_handling TSRMLS_CC);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &size, &capacity) == FAILURE) {
        zend_restore_error_handling(&error_handling TSRMLS_CC);
        return;
    }
    zend_restore_error_handling(&error_handling TSRMLS_CC);

    if (size <= 0) {
        zend_throw_exception(NULL, "Array size must be positive", 0 TSRMLS_CC);
        return;
    }

    if (capacity < 0) {
        zend_throw_exception(NULL, "Array capacity must be positive or 0", 0 TSRMLS_CC);
        return;
    }

    intern = zend_object_store_get_object(getThis() TSRMLS_CC);

    intern->array = slobel_ds_darray_create((size_t)size, (size_t)capacity);
    if (!intern->array) {
        zend_throw_exception(NULL, "Failed to allocate array", 0 TSRMLS_CC);
    }

    return;
}


PHP_METHOD(slobel_darray, count)
{
    slobel_darray *intern;
    long count;

    intern = zend_object_store_get_object(getThis() TSRMLS_CC);
    count = (long)slobel_ds_darray_count(intern->array);

    ZVAL_LONG(return_value, count);
}

PHP_METHOD(slobel_darray, length)
{
    slobel_darray *intern;
    long length;

    intern = zend_object_store_get_object(getThis() TSRMLS_CC);
    length = (long) slobel_ds_darray_length(intern->array);

    ZVAL_LONG(return_value, length);
}

PHP_METHOD(slobel_darray, offsetSet)
{
    slobel_darray *intern;
    zval *val;
    long index;


    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lz", &index, &val) == FAILURE) {
        zend_throw_exception(NULL, "Failed to parse arguments", 0 TSRMLS_CC);
        return;
    }

    intern = zend_object_store_get_object(getThis() TSRMLS_CC);
    slobel_ds_darray_set(intern->array, (size_t)index, val);

}

PHP_METHOD(slobel_darray, offsetUnset)
{
    slobel_darray *intern;
    long index;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) == FAILURE) {
        zend_throw_exception(NULL, "Invalid index passed", 0 TSRMLS_CC);
        return;
    }


    intern = zend_object_store_get_object(getThis() TSRMLS_CC);
    slobel_ds_darray_unset(intern->array, (size_t)index);
}

PHP_METHOD(slobel_darray, offsetGet)
{
    slobel_darray *intern;
    long index;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) == FAILURE) {
        zend_throw_exception(NULL, "Invalid index passed", 0 TSRMLS_CC);
        return;
    }


    intern = zend_object_store_get_object(getThis() TSRMLS_CC);
    zval *val = slobel_ds_darray_get(intern->array, (size_t)index);

    if (val) {
        ZVAL_ZVAL(return_value, val, 1, 0);
    } else {
        ZVAL_NULL(return_value);
    }
}

PHP_METHOD(slobel_darray, offsetExists)
{
    slobel_darray *intern;
    long index;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index) == FAILURE) {
        zend_throw_exception(NULL, "Invalid index passed", 0 TSRMLS_CC);
        return;
    }


    intern = zend_object_store_get_object(getThis() TSRMLS_CC);
    zval *val = slobel_ds_darray_get(intern->array, (size_t)index);
    if (val) {
        ZVAL_TRUE(return_value);
    } else {
        ZVAL_FALSE(return_value);
    }
}



PHP_METHOD(slobel_darray, filter)
{
    slobel_darray *intern;
    slobel_darray *newArray;

    zval *retArray;
    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fci_cache = empty_fcall_info_cache;

    int have_callback = 0;
    if (ZEND_NUM_ARGS() > 0) {
        have_callback = 1;

        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|f", &fci, &fci_cache) == FAILURE) {
            zend_throw_exception(NULL, "Invalid callback passed", 0 TSRMLS_CC);
            return;
        }

    }

    intern = zend_object_store_get_object(getThis() TSRMLS_CC);

    MAKE_STD_ZVAL(retArray);
    object_init_ex(retArray, slobel_darray_ce);
    newArray = zend_object_store_get_object(retArray TSRMLS_CC);
    newArray->array = slobel_ds_darray_create(slobel_ds_darray_min_length(intern->array), slobel_ds_darray_capacity(intern->array));

    if (!newArray->array) {
        zend_throw_exception(NULL, "Failed to allocate new array", 0 TSRMLS_CC);
        zval_ptr_dtor(&retArray);
        return;
    }

    zval *val;
    size_t i = 0;
    size_t newIndex = 0;
    if (!have_callback) {
        for (i = 0; i < slobel_ds_darray_length(intern->array); i++) {
            val = slobel_ds_darray_get(intern->array, i);
            if (val != NULL && zend_is_true(val)) {
                slobel_ds_darray_set(newArray->array, newIndex++, val);
            }
        }
    } else {
        zval **args[1];
        zval *retval;

        fci.no_separation = 1;
        fci.retval_ptr_ptr = &retval;
        fci.param_count = 1;

        for (i = 0; i < slobel_ds_darray_length(intern->array); i++) {

            val = slobel_ds_darray_get(intern->array, i);
            if (val != NULL) {
                args[0] = &val;
                fci.params = args;
                if (zend_call_function(&fci, &fci_cache TSRMLS_CC) == SUCCESS && retval) {
                    if (zend_is_true(retval)) {
                        slobel_ds_darray_set(newArray->array, newIndex++, val);
                    }
                    zval_ptr_dtor(&retval);

                } else {
                    php_error_docref(NULL TSRMLS_CC, E_WARNING, "An error occurred while invoking the filter callback");
                    return;
                }

            }
        }

    }

    RETVAL_ZVAL(retArray, 1, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_void, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_construct, 0, 0, 1)
ZEND_ARG_INFO(0, size)
ZEND_ARG_INFO(0, capacity)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_slobel_darray_offset, 0, 0, 1)
ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_slobel_darray_offset_value, 0, 0, 2)
ZEND_ARG_INFO(0, offset)
ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_jarray_filter, 0, 0, 1)
ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()

const zend_function_entry slobel_darray_functions[] = {
    PHP_ME(slobel_darray, __construct, arginfo_construct, ZEND_ACC_PUBLIC)
    PHP_ME(slobel_darray, offsetSet, arginfo_slobel_darray_offset_value, ZEND_ACC_PUBLIC)
    PHP_ME(slobel_darray, offsetGet, arginfo_slobel_darray_offset, ZEND_ACC_PUBLIC)
    PHP_ME(slobel_darray, offsetUnset, arginfo_slobel_darray_offset, ZEND_ACC_PUBLIC)
    PHP_ME(slobel_darray, offsetExists, arginfo_slobel_darray_offset, ZEND_ACC_PUBLIC)
    PHP_ME(slobel_darray, filter, arginfo_jarray_filter, ZEND_ACC_PUBLIC)
    PHP_ME(slobel_darray, count, arginfo_void, ZEND_ACC_PUBLIC)
    PHP_ME(slobel_darray, length, arginfo_void, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

void slobel_darray_init(TSRMLS_D)
{
    zend_class_entry tmp_ce;
    INIT_CLASS_ENTRY(tmp_ce, "SLOBEL\\DArray", slobel_darray_functions);

    slobel_darray_ce = zend_register_internal_class(&tmp_ce TSRMLS_CC);
    slobel_darray_ce->create_object = slobel_darray_create_object;
    slobel_darray_ce->get_iterator = slobel_darray_get_iterator;
    slobel_darray_ce->iterator_funcs.funcs = &slobel_darray_iterator_funcs;
    memcpy(&slobel_darray_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

    zend_class_implements(slobel_darray_ce TSRMLS_CC, 2, zend_ce_arrayaccess, zend_ce_traversable);

    slobel_darray_handlers.has_dimension   = slobel_darray_has_dimension;
    slobel_darray_handlers.read_dimension  = slobel_darray_read_dimension;
    slobel_darray_handlers.write_dimension = slobel_darray_write_dimension;
    slobel_darray_handlers.unset_dimension = slobel_darray_unset_dimension;
    slobel_darray_handlers.count_elements  = slobel_darray_count_elements;
    slobel_darray_handlers.clone_obj = slobel_darray_clone;

    return;
}