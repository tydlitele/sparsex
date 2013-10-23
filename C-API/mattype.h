/**
 * libcsx/mattype.h -- Available matrix types.
 *
 * Copyright (C) 2013, Computing Systems Laboratory (CSLab), NTUA.
 * Copyright (C) 2013, Athena Elafrou
 * All rights reserved.
 *
 * This file is distributed under the BSD License. See LICENSE.txt for details.
 */
#ifndef LIBCSX_MATTYPE_H
#define LIBCSX_MATTYPE_H

#include <inttypes.h>

#ifdef USE_UNSIGNED_INDICES
#   undef index_t
#   define index_t uint32_t
#endif

#ifdef USE_64BIT_INDICES
#   undef index_t
#   define index_t uint64_t
#endif

#ifndef index_t
#define index_t int
#endif

#ifdef USE_SINGLE_PRECISION
#   define value_t float
#else
#   define value_t double
#endif

#ifndef SCALAR
#   define SCALAR double
#endif

#endif // LIBCSX_MATTYPE_H
