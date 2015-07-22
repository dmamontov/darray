#ifndef PHP_SLOBEL_H
#define PHP_SLOBEL_H 1

#define PHP_SLOBEL_VERSION "0.1"
#define PHP_SLOBEL_EXTNAME "slobel"

extern zend_module_entry slobel_module_entry;
#define phpext_slobel_ptr &slobel_module_entry

#ifdef PHP_WIN32
# define PHP_DARRAY_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
# define PHP_DARRAY_API __attribute__ ((visibility("default")))
#else
# define PHP_DARRAY_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#endif

