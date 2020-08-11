#
#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

REPOSITORY_REVISION=`echo $REPOSITORY_REVISION`


AC_PREREQ(0.01)
AC_INIT(template, 0.1.0, chjang@nablecomm.com)
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([revision.h])

AC_DEFINE_UNQUOTED([TEMPLATE_VERSION], "TEMPLATE v0.1.0", [Define to the version of this package])
AC_DEFINE_UNQUOTED([REPOSITORY_REVISION], "$REPOSITORY_REVISION", [Define to the revision of this package])

## core 수보다 thread 수가 2배 이상 많아질 경우 경합에 의한 cpu 사용율이 높아지므로
## 일반적인 상황인 thread 수를 많이 생성하여 사용하는 상황을 고려하여 TBB SYNC를 off 한다.
# AC_DEFINE([USE_THREAD_POOL_TBB_QUEUE_SYNC], [1], [Define to use tbb::concurrent_bounded_queue::pop on ThreadPool])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lxml2':
AC_CHECK_LIB([rt], [clock_gettime])
AC_CHECK_LIB([pthread], [pthread_create])

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([stdlib.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_SETPGRP
AC_TYPE_SIGNAL
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([gethostname memset])

AC_CONFIG_FILES([ template/ifm/Makefile \
                  template/hsm/Makefile \
                  template/tum/Makefile \
                  template/main/Makefile \
                  template/Makefile \
				      Makefile])

AC_OUTPUT

