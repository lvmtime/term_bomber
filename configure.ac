#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# check autotools version
AC_PREREQ([2.65])

# init autotools
AC_INIT([Terminal bomber], [0.3.2], [lvmtime@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# check for required programs
AC_PROG_CC
AC_PROG_INSTALL

# is build environment sane?
AC_CONFIG_SRCDIR([game_engine.c])

# files to generate from templates
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

# check for needed targets and headers
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset mkdir])
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])
AC_CHECK_LIB([ncurses], [initscr])
AC_CHECK_LIB([pthread], [pthread_create])

# make the 'stuff'
AC_OUTPUT
