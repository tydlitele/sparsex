#ifndef __PRFCNT_H__
#define __PRFCNT_H__

#if defined(CPU_AMD)
	#include "prfcnt_amd.h"
#elif defined(CPU_XEON)
	#include "prfcnt_xeon.h"
#elif defined(CPU_CORE)
	#include "prfcnt_core.h"
#else
	#warning "no prfcnt implementation found"
	#include "prfcnt_nop.h"
#endif

#endif
