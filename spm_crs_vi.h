#ifndef __SPM_CRS_VI_H__
#define __SPM_CRS_VI_H__

#include <inttypes.h>
#include "spmv_method.h"
#include "macros.h"

//#define SPM_CRS_VI_VIDX_TYPE uint8_t

#define _NAME(val_type, ci_bits, vi_bits, name) \
	spm_crs ## ci_bits ## _vi ## vi_bits ## _ ## val_type ## name

#define DECLARE_CRS_VI(val_type, ci_bits, vi_bits)        \
	typedef struct {                                  \
		val_type            *values;              \
		UINT_TYPE(ci_bits)  *col_ind, *row_ptr;   \
		UINT_TYPE(vi_bits)  *val_ind;             \
		uint64_t            nz, nrows, ncols, nv; \
	} _NAME(val_type, ci_bits, vi_bits, _t);          \
	                                                  \
	void *                                            \
	_NAME(val_type, ci_bits, vi_bits, _init_mmf)(     \
		char *mmf_file,                           \
		unsigned long *rows_nr,                   \
		unsigned long *cols_nr,                   \
		unsigned long *nz_nr                      \
	);                                                \
	                                                  \
	void                                              \
	_NAME(val_type, ci_bits, vi_bits, _destroy)(      \
		void *m                                   \
	);                                                \
	                                                  \
	uint64_t                                          \
	_NAME(val_type, ci_bits, vi_bits, _size)(         \
		void *m                                   \
	);                                                \
	                                                  \
	spmv_ ## val_type ## _fn_t                        \
	_NAME(val_type, ci_bits, vi_bits, _multiply);     \


DECLARE_CRS_VI(double, 32, 8)
DECLARE_CRS_VI(double, 32, 16)
DECLARE_CRS_VI(double, 32, 32)

DECLARE_CRS_VI(float, 32, 8)
DECLARE_CRS_VI(float, 32, 16)
DECLARE_CRS_VI(float, 32, 32)

DECLARE_CRS_VI(double, 64, 8)
DECLARE_CRS_VI(double, 64, 16)
DECLARE_CRS_VI(double, 64, 32)

DECLARE_CRS_VI(float, 64, 8)
DECLARE_CRS_VI(float, 64, 16)
DECLARE_CRS_VI(float, 64, 32)

#undef _NAME
#undef DECLARE_CRS_VI

#define SPM_CRSVI_CI_TYPE UINT_TYPE(SPM_CRSVI_CI_BITS)
#define SPM_CRSVI_VI_TYPE UINT_TYPE(SPM_CRSVI_VI_BITS)

#define SPM_CRSVI_NAME(name) \
	CON7(spm_crs, SPM_CRSVI_CI_BITS, _vi, SPM_CRSVI_VI_BITS, _, ELEM_TYPE, name)
#define SPM_CRSVI_TYPE SPM_CRSVI_NAME(_t)


#endif
