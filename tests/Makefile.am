# Copyright (C) 2006-2013 Free Software Foundation, Inc.
# This Makefile.am is free software; the Free Software Foundation
# gives unlimited permission to copy, distribute and modify it.

TESTS = check_ghost_data check_ghost_parser
check_PROGRAMS = check_ghost_data check_ghost_parser

check_ghost_data_SOURCES = check_ghost_data.c $(top_builddir)/src/ghost_data.h
check_ghost_data_CFLAGS = @CHECK_CFLAGS@
check_ghost_data_LDADD = $(top_builddir)/src/ghost_data.o @CHECK_LIBS@ -lxcb

check_ghost_parser_SOURCES = check_ghost_parser.c 
check_ghost_parser_CFLAGS = @CHECK_CFLAGS@
check_ghost_parser_LDADD = $(top_builddir)/src/ghost_data.o @CHECK_LIBS@ -lxcb

