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
	-I$(top_srcdir)/src/bench \
	$(AX_CPPFLAGS)
AM_CXXFLAGS = $(AX_CXXFLAGS)
AM_LDFLAGS = \
	-L$(top_builddir)/src/api/.libs \
	$(AX_LDFLAGS)

noinst_HEADERS = \
	Bench.hpp \
	BenchUtil.hpp \
	MklModule.hpp \
	Mmf.hpp \
	PoskiModule.hpp \
	SparsexModule.hpp \
	Timer.hpp

noinst_PROGRAMS = bench_spmv
bench_spmv_SOURCES = \
	Bench.cpp \
	main.cpp \
	Timer.cpp \
	SparsexModule.cpp

if SPX_BUILD_BENCH_MKL
AM_CPPFLAGS += $(MKL_CPPFLAGS)
AM_CXXFLAGS += $(MKL_CXXFLAGS)
AM_LDFLAGS += $(MKL_LDFLAGS)
bench_spmv_SOURCES += MklModule.cpp
endif

if SPX_BUILD_BENCH_POSKI
AM_CPPFLAGS += $(POSKI_CPPFLAGS)
AM_CXXFLAGS += $(POSKI_CXXFLAGS)
AM_LDFLAGS += $(POSKI_LDFLAGS)
bench_spmv_SOURCES += PoskiModule.cpp
endif

bench_spmv_LDFLAGS = -rpath $(LLVM_LIBDIR)
bench_spmv_LDADD = \
	$(top_builddir)/src/api/.libs/libsparsex.la

if SPX_BUILD_BENCH_MKL
bench_spmv_LDADD += $(MKL_LIBS)
endif

if SPX_BUILD_BENCH_POSKI
bench_spmv_LDADD += $(POSKI_LIBS)
endif
