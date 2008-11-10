#include <stdlib.h>
#include <inttypes.h>

#include "spm_mt.h"
#include "spm_crs_vi.h"
#include "spm_crs_vi_mt.h"
#include "mt_lib.h"

#include "pthread.h"

#define SPM_CRSVI_CI_TYPE UINT_TYPE(SPM_CRSVI_CI_BITS)
#define SPM_CRSVI_VI_TYPE UINT_TYPE(SPM_CRSVI_VI_BITS)

#define SPM_CRS_VI_NAME(name) \
        CON7(spm_crs, SPM_CRSVI_CI_BITS, _vi, SPM_CRSVI_VI_BITS, _, ELEM_TYPE, name)
#define SPM_CRSVI_TYPE SPM_CRS_VI_NAME(_t)

#define SPM_CRSVI_MT_NAME(name) \
        CON8(spm_crs, SPM_CRSVI_CI_BITS, _vi, SPM_CRSVI_VI_BITS, _, ELEM_TYPE, _mt, name)
#define SPM_CRSVI_MT_TYPE SPM_CRSVI_MT_NAME(_t)

spm_mt_t *SPM_CRSVI_MT_NAME(_init_mmf)(char *mmf_file,
                                       unsigned long *rows_nr, unsigned long *cols_nr,
                                       unsigned long *nz_nr)
{
	unsigned int nr_cpus, *cpus_affinity;
	SPM_CRSVI_TYPE *crsvi;
	SPM_CRSVI_MT_TYPE *crsvi_mt;
	int i;
	unsigned long cur_row, elems_limit, elems_total=0;

	crsvi = SPM_CRS_VI_NAME(_init_mmf)(mmf_file, rows_nr, cols_nr, nz_nr);
	mt_get_options(&nr_cpus, &cpus_affinity);

	spm_mt_t *spm_mt;
	spm_mt_thread_t *spm_thread;

	spm_mt = malloc(sizeof(spm_mt_t));
	if ( !spm_mt ){
		perror("malloc");
		exit(1);
	}

	spm_mt->nr_threads = nr_cpus;
	spm_mt->spm_threads = malloc(sizeof(spm_mt_thread_t)*nr_cpus);
	if ( !spm_mt->spm_threads ){
		perror("malloc");
		exit(1);
	}

	crsvi_mt = malloc(sizeof(SPM_CRSVI_MT_TYPE)*nr_cpus);
	if ( !crsvi_mt ){
		perror("malloc");
		exit(1);
	}


	for (i=0, cur_row=0; i<nr_cpus; i++){
		elems_limit = (*nz_nr - elems_total) / (nr_cpus - i);
		spm_thread = spm_mt->spm_threads + i;
		spm_thread->cpu = cpus_affinity[i];
		spm_thread->spm = crsvi_mt + i;

		unsigned long elems, rows;
		crsvi_mt[i].row_start = cur_row;
		for (elems=0,rows=0; ; ){
			elems += crsvi->row_ptr[cur_row+1] - crsvi->row_ptr[cur_row];
			cur_row++;
			//printf("i:%d nr_cpus:%d cur_row:%lu rows_nr:%lu elems:%lu elems_limit:%lu\n", i, nr_cpus, cur_row, *rows_nr, elems, elems_limit);
			if (elems >= elems_limit){
				break;
			}
		}

		elems_total += elems;
		crsvi_mt[i].row_end = cur_row;
		crsvi_mt[i].nnz = elems;
		crsvi_mt[i].crsvi = crsvi;
	}


	free(cpus_affinity);

	return spm_mt;
}

void SPM_CRSVI_MT_NAME(_multiply)(void *spm, VECTOR_TYPE *in, VECTOR_TYPE *out)
{
	SPM_CRSVI_MT_TYPE *crsvi_mt = (SPM_CRSVI_MT_TYPE *)spm;
	ELEM_TYPE *x = in->elements;
	ELEM_TYPE *y = out->elements;
	ELEM_TYPE *values = crsvi_mt->crsvi->values;
	const register SPM_CRSVI_CI_TYPE *row_ptr = crsvi_mt->crsvi->row_ptr;
	const register SPM_CRSVI_CI_TYPE *col_ind = crsvi_mt->crsvi->col_ind;
	const register SPM_CRSVI_VI_TYPE *val_ind = crsvi_mt->crsvi->val_ind;
	const unsigned long row_start = crsvi_mt->row_start;
	const unsigned long row_end = crsvi_mt->row_end;
	//printf("(%lu) row_start: %lu row_end: %lu\n", pthread_self(), row_start, row_end);
	register ELEM_TYPE yr;

	register unsigned long i, j=0;
	for (i=row_start; i<row_end; i++){
		yr = (ELEM_TYPE)0;
		__asm__ __volatile__ ("# loop start\n\t");
		for(j=row_ptr[i]; j<row_ptr[i+1]; j++) {
			yr += (values[val_ind[j]] * x[col_ind[j]]);
			//printf("(%lu) i:%lu j:%lu val_ind:%ld col_ind:%lu v:%lf x:%lf yr:%lf\n", (unsigned long)pthread_self(),i, j, (long)val_ind[j], (unsigned long)col_ind[j], values[val_ind[j]], x[col_ind[j]], yr);
		}
		y[i] = yr;
		//printf("(%lu): y[%lu] = %lf\n", (unsigned long)pthread_self(), i, (double)yr);
		__asm__ __volatile__ ("# loop end\n\t");
	}
}

unsigned long SPM_CRSVI_MT_NAME(_size)(spm_mt_t *spm_mt)
{
	unsigned long ret;
	SPM_CRSVI_MT_TYPE *crsvi_mt = (SPM_CRSVI_MT_TYPE *)spm_mt->spm_threads->spm;
	SPM_CRSVI_TYPE *crsvi = crsvi_mt->crsvi;

	ret = crsvi->nv*sizeof(ELEM_TYPE);
	ret += crsvi->nz*(sizeof(SPM_CRSVI_VI_TYPE) + sizeof(SPM_CRSVI_CI_TYPE));
	ret += crsvi->nrows*sizeof(SPM_CRSVI_CI_TYPE);

	return ret;
}

#define XSPMV_METH_INIT(x,y,z,w) SPMV_METH_INIT(x,y,z,w)
XSPMV_METH_INIT(
	SPM_CRSVI_MT_NAME(_multiply),
	SPM_CRSVI_MT_NAME(_init_mmf),
	SPM_CRSVI_MT_NAME(_size),
	sizeof(ELEM_TYPE)
)
