/**
 * libcsx/common.h -- Common utilities.
 *
 * Copyright (C) 2013, Computing Systems Laboratory (CSLab), NTUA.
 * Copyright (C) 2013, Athena Elafrou
 * All rights reserved.
 *
 * This file is distributed under the BSD License. See LICENSE.txt for details.
 */
#ifndef LIBCSX_COMMON_H
#define LIBCSX_COMMON_H

#include "error.h"
#include "LoggerUtil.hpp"
#include "mattype.h"

#include <stdlib.h>

#define INVALID_INPUT ((input_t *) NULL)
#define INVALID_MAT ((matrix_t *) NULL)
#define INVALID_VEC ((vector_t *) NULL)
#define INVALID_PERM ((perm_t *) NULL)

static inline int
check_indexing(int base)
{
    return (base == 0 || base == 1);
}

static inline int
check_dim(index_t dim)
{
    return (dim >= 0);
}

/* Logging utilities */
static inline void log_disable_all()
{
    DisableLogging();
}

static inline void log_disable_error()
{
    DisableError();
}

static inline void log_disable_warning()
{
    DisableWarning();
}

static inline void log_disable_info()
{
    DisableInfo();
}

static inline void log_disable_debug()
{
    DisableDebug();
}

static inline void log_enable_all_console()
{
    AlwaysUseConsole();
}

static inline void log_enable_all_file(const char *file)
{
    AlwaysUseFile(file);
}

/**
 *  \brief Library initialization routine.
 */
void libcsx_init();

/**
 *  \brief malloc() wrapper.
 */
#define libcsx_malloc(type, size) \
	(type *)malloc_internal(size, __FILE__, __LINE__, __FUNCTION__)
void *malloc_internal(size_t x, const char *sourcefile, unsigned long lineno,
                      const char *function);

/**
 *  \brief free() wrapper.
 */
#define libcsx_free(object) \
    free_internal(object, __FILE__, __LINE__, __FUNCTION__)
void free_internal(void *ptr, const char *sourcefile, unsigned long lineno,
                   const char *function);

#endif // LIBCSX_COMMON_H
