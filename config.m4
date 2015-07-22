PHP_ARG_ENABLE(slobel, Enable slobel extension,[ --enable-slobel  Enable slobel support])

if test "$PHP_SLOBEL" = "yes"; then
  AC_DEFINE(HAVE_SLOBEL, 1, [Whether you have slobel])
  PHP_NEW_EXTENSION(slobel, slobel.c ds/darray.c slobel_darray.c, $ext_shared)
fi