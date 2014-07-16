#
# Makefile.am -- Makefile for library benchmarks
#
# Copyright (C) 2014, Computing Systems Laboratory (CSLab), NTUA
# Copyright (C) 2014, Vasileios Karakasis
# All rights reserved.
#
# This file is distributed under the BSD License. See LICENSE.txt for details.
#

AM_DEFAULT_VERBOSITY = 0
ACLOCAL_AMFLAGS = -I m4check

AM_CPPFLAGS = \
	-I$(top_builddir)/include \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/examples/examples \
	$(AX_CPPFLAGS)
AM_CXXFLAGS = $(AX_CXXFLAGS)
AM_LDFLAGS	= \
	-L$(top_builddir)/src/internals/.libs \
	$(AX_LDFLAGS)

noinst_PROGRAMS = mmf_example csr_example reordering_example

mmf_example_SOURCES = \
	mmf_example.c

csr_example_SOURCES = \
	csr_example.c

reordering_example_SOURCES = \
	reordering_example.c

mmf_example_LDADD = \
	$(top_builddir)/src/api/.libs/libsparsex.la \
	$(CLANG_LIBS) \
	$(LLVM_LIBS)

csr_example_LDADD = \
	$(top_builddir)/src/api/.libs/libsparsex.la \
	$(CLANG_LIBS) \
	$(LLVM_LIBS)

reordering_example_LDADD = \
	$(top_builddir)/src/api/.libs/libsparsex.la \
	$(CLANG_LIBS) \
	$(LLVM_LIBS)