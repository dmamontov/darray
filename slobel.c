
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_slobel.h"
#include "slobel_darray.h"

const zend_function_entry slobel_functions[] = {
    PHP_FE_END
};

PHP_MINIT_FUNCTION(slobel_init)
{
	slobel_darray_init(TSRMLS_C);
    return SUCCESS;
}


zend_module_entry slobel_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_SLOBEL_EXTNAME,
    slobel_functions,
    PHP_MINIT(slobel_init),
    NULL, //MSHUTDOWN
    NULL, //RINIT
    NULL, //RSHUTDOWN
    NULL, //MINFO
    PHP_SLOBEL_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_SLOBEL
ZEND_GET_MODULE(slobel)
#endif