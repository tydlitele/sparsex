/*
 * bcsr/mult/1x5.c -- BCSR 1x5 multiplication method
 *
 * Copyright (C) 2011, Computing Systems Laboratory (CSLab), NTUA
 * Copyright (C) 2011, Automatically generated by mult_generator.py
 * All rights reserved.
 *
 * This file is distributed under the BSD License. See LICENSE.txt for details.
 */

#include "mult/multiply.h"

void SPM_BCSR_NAME(_multiply_1x5)(void *spm, VECTOR_TYPE *in, VECTOR_TYPE *out)
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
	for (i = r_start, i_ = r_start; i < r_end; i += 1, i_++) {
		register ELEM_TYPE yr0 = 0;
		for (j = brow_ptr[i_], j_ = j / (1*5);
		     j < brow_ptr[i_+1]; j += 1*5, j_++) {
			SPM_CRS_IDX_TYPE x_start = bcol_ind[j_];
			yr0 += bvalues[j]*x[x_start] + bvalues[j+1]*x[x_start+1] + bvalues[j+2]*x[x_start+2] + bvalues[j+3]*x[x_start+3] + bvalues[j+4]*x[x_start+4];
		}

		y[i] = yr0;
	}

	return;
}

void SPM_BCSR_MT_NAME(_multiply_1x5)(void *spm, VECTOR_TYPE *in, VECTOR_TYPE *out)
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
	for (i = r_start, i_ = r_start; i < r_end; i += 1, i_++) {
		register ELEM_TYPE yr0 = 0;
		for (j = brow_ptr[i_], j_ = j / (1*5);
		     j < brow_ptr[i_+1]; j += 1*5, j_++) {
			SPM_CRS_IDX_TYPE x_start = bcol_ind[j_];
			yr0 += bvalues[j]*x[x_start] + bvalues[j+1]*x[x_start+1] + bvalues[j+2]*x[x_start+2] + bvalues[j+3]*x[x_start+3] + bvalues[j+4]*x[x_start+4];
		}

		y[i] = yr0;
	}

	return;
}
