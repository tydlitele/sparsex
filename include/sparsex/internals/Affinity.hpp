/*
 * Affinity.hpp -- Routine that sets the cpu affinity.
 *
 * Copyright (C) 2007-2011, Computing Systems Laboratory (CSLab), NTUA
 * Copyright (C) 2007-2011, Kornilios Kourtis
 * Copyright (C) 2013,      Athena Elafrou
 * All rights reserved.
 *
 * This file is distributed under the BSD License. See LICENSE.txt for details.
 */

#ifndef SPARSEX_INTERNALS_AFFINITY_HPP
#define SPARSEX_INTERNALS_AFFINITY_HPP

#include "sparsex/internals/logger/Logger.hpp"

#include <stdlib.h>
#include <sched.h>

void setaffinity_oncpu(unsigned int cpu);
int get_cpu_count();

#endif // SPARSEX_INTERNALS_AFFINITY_HPP
