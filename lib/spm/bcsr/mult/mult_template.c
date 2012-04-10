/*
 * ${src_filename} -- BCSR ${r}x${c} multiplication method
 *
 * Copyright (C) ${current_year}, Computing Systems Laboratory (CSLab), NTUA
 * Copyright (C) ${current_year}, Automatically generated by ${generator_prog}
 * All rights reserved.
 *
 * This file is distributed under the BSD License. See LICENSE.txt for details.
 */

#include "mult/multiply.h"

void SPM_BCSR_NAME(_multiply_${r}x${c}) (void *spm, VECTOR_TYPE *in, VECTOR_TYPE *out)
{
	SPM_BCSR_TYPE *mat = (SPM_BCSR_TYPE *) spm;
	ELEM_TYPE *y = out->elements;
	ELEM_TYPE *x = in->elements;
	ELEM_TYPE *bvalues = mat->bvalues;
	SPM_CRS_IDX_TYPE *brow_ptr = mat->brow_ptr;
	SPM_CRS_IDX_TYPE *bcol_ind = mat->bcol_ind;
	const SPM_CRS_IDX_TYPE r_start = 0;
	const SPM_CRS_IDX_TYPE r_end = mat->nrows;

	SPM_CRS_IDX_TYPE i, i_, j, j_;
	for (i = r_start, i_ = r_start / ${r}; i < r_end; i += ${r}, i_++) {
${y_init_hook}
		for (j = brow_ptr[i_], j_ = j / (${r}*${c});
			 j < brow_ptr[i_+1]; j += ${r}*${c}, j_++) {
			SPM_CRS_IDX_TYPE x_start = bcol_ind[j_];
${loop_body_hook}
		}

${y_assign_hook}
	}

	return;
}

void SPM_BCSR_MT_NAME(_multiply_${r}x${c}) (void *spm, VECTOR_TYPE *in, VECTOR_TYPE *out)
{
	SPM_BCSR_MT_TYPE *bcsr_mt = (SPM_BCSR_MT_TYPE *) spm;
	ELEM_TYPE *y = out->elements;
	ELEM_TYPE *x = in->elements;
	ELEM_TYPE *bvalues = bcsr_mt->bcsr->bvalues;
	SPM_CRS_IDX_TYPE *brow_ptr = bcsr_mt->bcsr->brow_ptr;
	SPM_CRS_IDX_TYPE *bcol_ind = bcsr_mt->bcsr->bcol_ind;
	const SPM_CRS_IDX_TYPE r_start = bcsr_mt->row_start;
	const SPM_CRS_IDX_TYPE r_end = bcsr_mt->row_end;

	SPM_CRS_IDX_TYPE i, i_, j, j_;
	for (i = r_start, i_ = r_start / ${r}; i < r_end; i += ${r}, i_++) {
${y_init_hook}
		for (j = brow_ptr[i_], j_ = j / (${r}*${c});
			 j < brow_ptr[i_+1]; j += ${r}*${c}, j_++) {
			SPM_CRS_IDX_TYPE x_start = bcol_ind[j_];
${loop_body_hook}
		}

${y_assign_hook}
	}

	return;
}
