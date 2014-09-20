# Copyright (C) 2006-2013 Free Software Foundation, Inc.
# This configure.ac script is free software; the Free Software Foundation
# gives unlimited permission to copy, distribute and modify it.

AC_INIT([ghost], [1.0], [matt.juntunen@hotmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
 Makefile
 src/Makefile
 tests/Makefile
])

#make sure xcb is installed
AC_CHECK_LIB([xcb],[xcb_connect], [], [AC_MSG_ERROR([XCB library was not found!])])

# make sure check is installed
PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])

AC_OUTPUT
