#include "spm.h"

#include <vector>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <boost/function.hpp>
#include <boost/foreach.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>

#include <cairomm/context.h>
#include <cairomm/surface.h>

#include "../../debug/debug_user.h"

namespace bll = boost::lambda;

#define FOREACH BOOST_FOREACH

namespace csx {

template <typename T>
T DeltaEncode(T input)
{
	T output;
	typename T::iterator in, out;
	typename T::iterator::value_type prev, curr;

	output.resize(input.size());

	in = input.begin();
	out = output.begin();
	prev = *out++ = *in++;
	while (in < input.end()){
		curr = *in++;
		*out++ = curr - prev;
		prev = curr;
	}

	return output;
}

template <typename T>
struct RLE {
	long freq;
	T val;
};

template <typename T>
std::vector< RLE<typename T::iterator::value_type> >
RLEncode(T input)
{
	typename T::iterator in;
	typename T::iterator::value_type curr;
	std::vector< RLE<typename T::iterator::value_type> > output;
	RLE<typename T::iterator::value_type> rle;

	 in = input.begin();
	 rle.freq = 1;
	 rle.val = *in++;

	while (in < input.end()){
		curr = *in;
		if (rle.val == curr){
			rle.freq++;
		} else {
			output.push_back(rle);
			rle.freq = 1;
			rle.val = curr;
		}
		in++;
	}
	output.push_back(rle);
	return output;
}

class DeltaRLE : public Pattern {
public:
	uint32_t size, drle_len;
	SpmIterOrder type;

	DeltaRLE(uint32_t _size, uint32_t _drle_len, SpmIterOrder _type):
	size(_size), drle_len(_drle_len), type(_type) { ; }
	virtual DeltaRLE *clone() const
	{
		return new DeltaRLE(*this);
	}
	virtual long x_increase(SpmIterOrder order) const
	{
		long ret;
		ret = (order == this->type) ? (this->size*this->drle_len) : 1;
		return ret;
	}

	virtual std::ostream &print_on(std::ostream &out) const
	{
		out << "drle: size=" << this->size << " len=" << this->drle_len << " type=" << this->type;
		return out;
	}
};

typedef std::vector<CooElem> SpmPoints;
typedef std::iterator<std::forward_iterator_tag, CooElem> SpmPointIter;
typedef std::vector<SpmCooElem> SpmCooElems;
typedef std::vector<SpmRowElem> SpmRowElems;
typedef std::vector<SpmRowElems> SpmRows;

typedef boost::function<void (CooElem &p)> TransformFn;

std::ostream &operator<<(std::ostream &out, CooElem p)
{
	out << "(" << std::setw(2) << p.y << "," << std::setw(2) << p.x << ")";
	return out;
}

// mappings for vertical transformation
static inline void pnt_map_V(CooElem &src, CooElem &dst)
{
	uint64_t src_x = src.x;
	uint64_t src_y = src.y;
	dst.x = src_y;
	dst.y = src_x;
}
static inline void pnt_rmap_V(CooElem &src, CooElem &dst)
{
	pnt_map_V(src, dst);
}

static inline void pnt_map_D(const CooElem &src, CooElem &dst, uint64_t nrows)
{
	uint64_t src_x = src.x;
	uint64_t src_y = src.y;
	assert(nrows + src_x - src_y > 0);
	dst.y = nrows + src_x - src_y;
	dst.x = (src_x < src_y) ? src_x : src_y;
}
static inline void pnt_rmap_D(const CooElem &src, CooElem &dst, uint64_t nrows)
{
	uint64_t src_x = src.x;
	uint64_t src_y = src.y;
	if (src_y < nrows) {
		dst.x = src_x;
		dst.y = nrows + src_x - src_y;
	} else {
		dst.y = src_x;
		dst.x = src_y + src_x - nrows;
	}
}

static inline void pnt_map_rD(const CooElem &src, CooElem &dst, uint64_t nrows)
{
	uint64_t src_x = src.x;
	uint64_t src_y = src.y;
	uint64_t dst_y;
	dst.y = dst_y = src_x + src_y - 1;
	dst.x = (dst_y <= nrows) ? src_x : src_x + nrows - dst_y;
}

static inline void pnt_rmap_rD(const CooElem &src, CooElem &dst, uint64_t nrows)
{
	uint64_t src_x = src.x;
	uint64_t src_y = src.y;
	if (src_y < nrows){
		dst.x = src_x;
	} else {
		dst.x = src_x + src_y - nrows;
	}
	dst.y = src_y - dst.x + 1;
}


class SpmIdx {
public:
	uint64_t nrows, ncols, nnz;
	SpmIterOrder type;
	SpmRows rows;

	SpmIdx() {type = NONE;};
	~SpmIdx() {};

	// load matrix from an MMF file
	void loadMMF(std::string mmf_file);
	void loadMMF(std::istream &in=std::cin);

	// Print Functions
	void Print(std::ostream &out=std::cout);
	void PrintRows(std::ostream &out=std::cout);

	template <typename IterT>
	void SetRows(IterT pnts_start, IterT pnts_end);

	// iterators that return a SpmCooElem
	class PointIter;
	PointIter points_begin();
	PointIter points_end();

	// same with PointIter, but removes elements
	class PointPoper;
	PointPoper points_pop_begin();
	PointPoper points_pop_end();

	// functions for data transformations into different coordinates
	inline TransformFn getTransformFn(SpmIterOrder type);
	inline TransformFn getRTransformFn(SpmIterOrder type);
	void Transform(SpmIterOrder type);

	//
	static const long min_limit = 4;
	void DRLEncode();
	void DRLEncodeRow(SpmRowElems &oldrow, SpmRowElems &newrow);
	void doDRLEncode(uint64_t &col, std::vector<uint64_t> &xs, SpmRowElems &newrow);

	//
	void Draw(const char *filename, const int width=600, const int height=600);
};

std::ostream &operator<<(std::ostream &out, SpmCooElem e)
{
	out << static_cast<CooElem>(e);
	if (e.pattern != NULL)
			out << "->[" << *(e.pattern) << "]";
	return out;
}



std::ostream &operator<<(std::ostream  &out, SpmRowElem &elem)
{
	out << elem.x;
	if (elem.pattern){
		out << " (" << *(elem.pattern)  << " " << elem.pattern << ")";
	}
	return out;
}

std::ostream &operator<<(std::ostream &out, SpmRowElems &elems)
{
	out << "row( ";
	FOREACH(SpmRowElem &elem, elems){
		out << elem << " [@" << &elem << "]  ";
	}
	out << ") @" << &elems ;
	return out;
}

std::ostream &operator<<(std::ostream &out, SpmRows &rows)
{
	FOREACH(SpmRowElems &row, rows){
		out << row << std::endl;
	}
	return out;
}


class SpmIdx::PointIter
: public std::iterator<std::forward_iterator_tag, CooElem>
{
public:
	uint64_t row_idx;
	uint64_t elm_idx;
	SpmIdx   *spm;
	PointIter(): row_idx(0), elm_idx(0), spm(NULL) {;}
	PointIter(uint64_t ridx, uint64_t eidx, SpmIdx *s)
	: row_idx(ridx), elm_idx(eidx), spm(s) {
		// find the first non zero row
		while (row_idx < spm->nrows && spm->rows[row_idx].size() == 0) {
			row_idx++;
		}
	}

	bool operator==(const PointIter &x){
		return (spm == x.spm)
			   && (row_idx == x.row_idx)
			   && (elm_idx == x.elm_idx);
	}

	bool operator!=(const PointIter &x){
		return !(*this == x);
	}

	void operator++() {
		uint64_t rows_nr, row_elems_nr;

		rows_nr = spm->rows.size();
		row_elems_nr = spm->rows[row_idx].size();

		// equality means somebody did a ++ on an ended iterator
		assert(row_idx < rows_nr);

		elm_idx++;
		assert(elm_idx <= row_elems_nr);
		if (elm_idx < row_elems_nr){
			return ;
		}

		// change row
		do {
			elm_idx = 0;
			row_idx++;
		} while (row_idx < spm->rows.size() && spm->rows[row_idx].size() == 0);
	}

	SpmCooElem operator*(){
		SpmCooElem ret;
		ret.y = row_idx + 1;
		ret.x = spm->rows[row_idx][elm_idx].x;
		Pattern *p = spm->rows[row_idx][elm_idx].pattern;
		ret.pattern = (p == NULL) ? NULL : p->clone();
		return ret;
	}
};

class SpmIdx::PointPoper : public SpmIdx::PointIter
{
public:
	PointPoper() : PointIter() { ; }
	PointPoper(uint64_t ridx, uint64_t eidx, SpmIdx *s)
	: PointIter(ridx, eidx, s) { ; }

	void operator++() {
		uint64_t rows_nr, row_elems_nr;

		rows_nr = spm->rows.size();
		row_elems_nr = spm->rows[row_idx].size();

		// equality means somebody did a ++ on an ended iterator
		assert(row_idx < rows_nr);

		elm_idx++;
		assert(elm_idx <= row_elems_nr);
		if (elm_idx < row_elems_nr){
			return;
		}

		// remove all elements from previous row
		spm->rows[row_idx].resize(0);
		// change row
		do {
			elm_idx = 0;
			row_idx++;
		} while (row_idx < rows_nr && spm->rows[row_idx].size() == 0);
	}
};


std::ostream &operator<<(std::ostream &out, SpmIdx::PointIter pi)
{
	out << "<" << std::setw(2) << pi.row_idx << "," << std::setw(2) << pi.elm_idx << ">";
	return out;
}


} // end of csx namespace

using namespace csx;

std::istream &operator>>(std::istream &in, SpmIdx &obj)
{
	obj.loadMMF(in);
	return in;
}


SpmIdx::PointIter SpmIdx::points_begin()
{
	return PointIter(0, 0, this);
}

SpmIdx::PointIter SpmIdx::points_end()
{
	return PointIter(this->rows.size(), 0, this);
}

SpmIdx::PointPoper SpmIdx::points_pop_begin()
{
	return PointPoper(0, 0, this);
}

SpmIdx::PointPoper SpmIdx::points_pop_end()
{
	return PointPoper(this->rows.size(), 0, this);
}

static inline bool elem_cmp_less(const SpmCooElem &e0,
                                 const SpmCooElem &e1)
{
	int ret;
	ret = CooCmp(static_cast<CooElem>(e0), static_cast<CooElem>(e1));
	return (ret < 0);
}


static inline void mk_row_elem(const CooElem &p, SpmRowElem &ret)
{
	ret.x = p.x;
	ret.pattern = NULL;
}

static inline void mk_row_elem(const SpmCooElem &p, SpmRowElem &ret)
{
	ret.x = p.x;
	ret.pattern = (p.pattern == NULL) ? NULL : (p.pattern)->clone();
}

template <typename IterT>
void SpmIdx::SetRows(IterT pnts_start, IterT pnts_end)
{
	SpmRowElems *row_elems;
	uint64_t row_prev;
	//std::cout << "SetRows\n";

	//this->rows.resize(this->nrows);
	this->rows.resize(1);
	row_elems = &this->rows.at(0);
	row_prev = 1;

	// XXX
	IterT point;
	for (point = pnts_start; point < pnts_end; ++point){
		uint64_t row = point->y;
		long size;

		if (row != row_prev){
			assert(row > this->rows.size());
			this->rows.resize(row);
			row_elems = &this->rows.at(row - 1);
			row_prev = row;
		}
		size = row_elems->size();
		row_elems->resize(size+1);
		mk_row_elem(*point, (*row_elems)[size]);
	}
	//std::cout << "SetRows [end]\n";
}


void SpmIdx::loadMMF(std::istream &in)
{
	char buff[1024];
	double val;
	uint64_t cnt;
	int ret;
	CooElem *point;
	SpmPoints points;

	// header
	do {
		in.getline(buff, sizeof(buff));
	} while (buff[0] == '#');
	ret = sscanf(buff, "%lu %lu %lu", &this->nrows, &this->ncols, &this->nnz);
	if (ret != 3){
		std::cerr << "mmf header error: sscanf" << std::endl;
		exit(1);
	}

	// body
	type = HORIZONTAL;
	points.resize(this->nnz);
	point = &points[0];
	for (cnt=0; cnt<this->nnz; cnt++){
		in.getline(buff, sizeof(buff));
		ret = sscanf(buff, "%lu %lu %lf", &point->y, &point->x, &val);
		assert(ret == 3);
		point++;
	}
	assert((uint64_t)(point - &points[0]) == this->nnz);
	assert(in.getline(buff, sizeof(buff)).eof());

	SetRows(points.begin(), points.end());
	points.clear();
}

inline TransformFn SpmIdx::getRTransformFn(SpmIterOrder type)
{
	boost::function<void (CooElem &p)> ret;
	ret = NULL;
	switch(type) {
		case HORIZONTAL:
		break;

		case VERTICAL:
		ret = bll::bind(pnt_rmap_V, bll::_1, bll::_1);
		break;

		case DIAGONAL:
		ret = bll::bind(pnt_rmap_D, bll::_1, bll::_1, this->nrows);
		break;

		case REV_DIAGONAL:
		ret = bll::bind(pnt_rmap_rD, bll::_1, bll::_1, this->nrows);
		break;

		default:
		assert(false);
	}
	return ret;
}

inline TransformFn SpmIdx::getTransformFn(SpmIterOrder type)
{
	boost::function<void (CooElem &p)> ret;
	ret = NULL;
	switch(type) {
		case VERTICAL:
		ret = bll::bind(pnt_map_V, bll::_1, bll::_1);
		break;

		case DIAGONAL:
		ret = bll::bind(pnt_map_D, bll::_1, bll::_1, this->nrows);
		break;

		case REV_DIAGONAL:
		ret = bll::bind(pnt_map_rD, bll::_1, bll::_1, this->nrows);
		break;

		default:
		assert(false);
	}
	return ret;
}

void SpmIdx::Transform(SpmIterOrder t)
{
	PointPoper p0, pe, p;
	SpmCooElems elems;
	boost::function<void (CooElem &p)> xform_fn, rxform_fn;
	uint64_t cnt;

	if (this->type == t)
		return;

	// Get the transformation function
	rxform_fn = getRTransformFn(this->type);
	if (t == HORIZONTAL) {
		// just do the reverse transformation
		xform_fn = rxform_fn;
	} else {
		xform_fn = getTransformFn(t);
		if (rxform_fn != NULL){
			// do a double xform: this->type -> HORIZONTAL -> t
			xform_fn = bll::bind(xform_fn, (bll::bind(rxform_fn, bll::_1), bll::_1));
		}
	}

	p0 = points_pop_begin();
	pe = points_pop_end();
	for(p=p0, cnt=0; p != pe; ++p){
		SpmCooElem p_new = SpmCooElem(*p);
		xform_fn(p_new);
		elems.push_back(p_new);
	}
	// This isn't necessary true, since one may meet patterns
	//assert(cnt == this->nnz);

	sort(elems.begin(), elems.end(), elem_cmp_less);
	SetRows(elems.begin(), elems.end());
	elems.clear();
	this->type = t;
}

void SpmIdx::doDRLEncode(uint64_t &col, std::vector<uint64_t> &xs, SpmRowElems &newrow)
{
	std::vector< RLE<uint64_t> > rles;
	SpmRowElem elem;
	//std::cout << "doDRLEncode() start\n";

	rles = RLEncode(DeltaEncode(xs));
	elem.pattern = NULL; // Default inserter (for push_back copies)
	FOREACH(RLE<uint64_t> &rle, rles){
		//std::cout << "newrow size " << newrow.size() << "\n";
		//std::cout << rle.freq << " " << rle.val << "\n";
		if (rle.freq >= min_limit){
			SpmRowElem *last_elem;

			col += rle.val; // go to the first
			elem.x = col;
			newrow.push_back(elem);
			last_elem = &newrow.back();
			last_elem->pattern = new DeltaRLE(rle.freq, rle.val, this->type);
			last_elem = NULL;
			col += rle.val*(rle.freq - 1);
		} else {
			for (int i=0; i < rle.freq; i++){
				col += rle.val;
				elem.x = col;
				newrow.push_back(elem);
			}
		}
	}

	//std::cout << "doDRLEncode() end\n";
}

void SpmIdx::DRLEncodeRow(SpmRowElems &oldrow, SpmRowElems &newrow)
{
	std::vector<uint64_t> xs;
	uint64_t col;

	//std::cout << "DRLEncodeRow() start\n";
	// start indices at 1
	col = 0;
	FOREACH(SpmRowElem &e, oldrow){
		if (e.pattern == NULL){
			xs.push_back(e.x);
			continue;
		}
		if (xs.size() != 0){
			doDRLEncode(col, xs, newrow);
			xs.clear();
		}
		col += e.pattern->x_increase(this->type);
		newrow.push_back(e);
	}
	if (xs.size() != 0){
		doDRLEncode(col, xs, newrow);
		xs.clear();
	}
	//std::cout << "DRLEncodeRow() end\n";
}

void SpmIdx::Draw(const char *filename, const int width, const int height)
{
	//Cairo::RefPtr<Cairo::PdfSurface> surface;
	Cairo::RefPtr<Cairo::ImageSurface> surface;
	Cairo::RefPtr<Cairo::Context> cr;
	SpmIdx::PointIter p, p_start, p_end;
	double max_cols, max_rows, x_pos, y_pos;
	boost::function<double (double v, double max_coord, double max_v)> coord_fn;
	boost::function<double (double a)> x_coord, y_coord;

	//surface = Cairo::PdfSurface::create(filename, width, height);
	surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, width, height);
	cr = Cairo::Context::create(surface);

	// background
	cr->save();
	cr->set_source_rgb(1, 1, 1);
	cr->paint();
	cr->restore();

	// TODO:
	//  If not HORIZNTAL, transform and transofrm back after end
	max_rows = (double)this->rows.size();
	max_cols = (double)this->ncols;

	// Lambdas for calculationg coordinates
	coord_fn = (((bll::_1 - .5)*(bll::_2))/(bll::_3));
	x_coord = bll::bind(coord_fn, bll::_1, (double)width, max_cols);
	y_coord = bll::bind(coord_fn, bll::_1, (double)height, max_rows);

	p_start = this->points_begin();
	p_end = this->points_end();
	for (p = p_start; p != p_end; ++p){
		SpmCooElem elem = *p;
		if (elem.pattern == NULL){
			x_pos = x_coord((*p).x);
			y_pos = y_coord((*p).y);
			cr->move_to(x_pos, y_pos);
			cr->show_text("x");
			//cr->restore();
		} else {
			// FIXME
			DeltaRLE *pattern = (DeltaRLE *)(elem.pattern);
			cr->save();
			switch (pattern->type){
				double x, y;
				case HORIZONTAL:
				cr->set_source_rgb(0, .5, .5);
				x = (*p).x;
				y = (*p).y;
				for (long i=0; i < pattern->size; i++){
					x_pos = x_coord(x);
					y_pos = y_coord(y);
					cr->move_to(x_pos, y_pos);
					cr->show_text("x");
					x += pattern->drle_len;
				}
				break;

				case VERTICAL:
				break;

				case DIAGONAL:
				break;

				case REV_DIAGONAL:
				break;

				case NONE:
				assert(false);
			}
			cr->restore();
		}
		cr->stroke();
	}
	//cr->show_page();
	surface->write_to_png(filename);
}

void SpmIdx::DRLEncode()
{
	FOREACH(SpmRowElems &oldrow, this->rows){
		SpmRowElems newrow;
		long newrow_size;
		// create new row
		DRLEncodeRow(oldrow, newrow);
		// copy data
		newrow_size = newrow.size();
		oldrow.clear();
		oldrow.reserve(newrow_size);
		for (long i=0; i < newrow_size; i++){
			oldrow.push_back(newrow[i]);
		}
	}
}

#if 0
void SpmIdx::PrintRows(std::ostream &out)
{
	uint64_t prev_y=1;
	out << prev_y << ": ";
	FOREACH(SpmCooElem &e, this->elems){
		uint64_t y = e.y;
		uint64_t x = e.x;
		if (y != prev_y){
			out << std::endl << y << ": ";
			prev_y = y;
		}
		out << x << " ";
	}
	out << std::endl;
}



void SpmIdx::genStats()
{
}

void SpmIdx::DeltaRLEncode(int limit, SpmIdxElems dstelems)
{
}

void PrintTransform(uint64_t y, uint64_t x, TransformFn xform_fn, std::ostream &out)
{
	uint64_t i,j;
	point_t p;
	for (i=1; i <= y; i++){
		for (j=1; j <= x;  j++){
			p.y = i;
			p.x = j;
			xform_fn(p);
			out << p << " ";
		}
		out << std::endl;
	}
}

void PrintDiagTransform(uint64_t y, uint64_t x, std::ostream &out)
{
	boost::function<void (point_t &p)> xform_fn;
	xform_fn = boost::bind(pnt_map_D, _1, _1, y);
	PrintTransform(y, x, xform_fn, out);
}

void PrintRDiagTransform(uint64_t y, uint64_t x, std::ostream &out)
{
	boost::function<void (point_t &p)> xform_fn;
	xform_fn = boost::bind(pnt_map_rD, _1, _1, y);
	PrintTransform(y, x, xform_fn, out);
}

void printXforms()
{
	std::cout << "Diagonal" << std::endl;
	PrintDiagTransform(10, 10, std::cout);
	std::cout << std::endl;
	PrintDiagTransform(5, 10, std::cout);
	std::cout << std::endl;
	PrintDiagTransform(10, 5, std::cout);
	std::cout << std::endl;
	std::cout << "Reverse Diagonal" << std::endl;
	PrintRDiagTransform(10, 10, std::cout);
	std::cout << std::endl;
	PrintRDiagTransform(5, 10, std::cout);
	std::cout << std::endl;
	PrintRDiagTransform(10, 5, std::cout);
	std::cout << std::endl;
}

void TestTransform(uint64_t y, uint64_t x, TransformFn xform_fn, TransformFn rxform_fn)
{
	uint64_t i,j;
	point_t p0, p1;
	for (i=1; i <= y; i++){
		for (j=1; j <= x;  j++){
			p0.y = i;
			p0.x = j;
			xform_fn(p0);
			p1.y = p0.y;
			p1.x = p0.x;
			rxform_fn(p1);
			if ( (p1.y != i) || (p1.x != j) ){
				std::cout << "Error for " << i << "," << j << std::endl;
				std::cout << "Transformed: " << p0.y << "," << p0.x << std::endl;
				std::cout << "rTransformed:" << p1.y << "," << p1.x << std::endl;
				exit(1);
			}
		}
	}
}

void TestDiagTransform(uint64_t y, uint64_t x)
{
	boost::function<void (point_t &p)> xform_fn, rxform_fn;
	xform_fn = boost::bind(pnt_map_D, _1, _1, y);
	rxform_fn = boost::bind(pnt_rmap_D, _1, _1, y);
	TestTransform(y, x, xform_fn, rxform_fn);
}

void TestRDiagTransform(uint64_t y, uint64_t x)
{
	boost::function<void (point_t &p)> xform_fn, rxform_fn;
	xform_fn = boost::bind(pnt_map_rD, _1, _1, y);
	rxform_fn = boost::bind(pnt_rmap_rD, _1, _1, y);
	TestTransform(y, x, xform_fn, rxform_fn);
}

void TestXforms()
{
	TestDiagTransform(10, 10);
	TestDiagTransform(5, 10);
	TestDiagTransform(10, 5);
	TestRDiagTransform(10, 10);
	TestRDiagTransform(5, 10);
	TestRDiagTransform(10, 5);
}
#endif

void SpmIdx::Print(std::ostream &out)
{
	SpmIdx::PointIter p, p_start, p_end;
	p_start = this->points_begin();
	p_end = this->points_end();
	for (p = p_start; p != p_end; ++p){
		out << " " << (*p);
	}
	out << std::endl;
}

void test_drle()
{
	std::vector<int> v_in, delta;
	std::vector< RLE<int> > drle;

	v_in.resize(16);
	for (int i=0; i<16; i++){
		v_in[i] = i;
	}
	FOREACH(int &v, v_in){
		std::cout << v << " ";
	}
	std::cout << std::endl;

	delta = DeltaEncode(v_in);
	FOREACH(int &v, delta){
		std::cout << v << " ";
	}
	std::cout << std::endl;

	drle = RLEncode(delta);
	FOREACH(RLE<int> &v, drle){
		std::cout << "(" << v.val << "," << v.freq << ") ";
	}
	std::cout << std::endl;
}

int main(int argc, char **argv)
{
	SpmIdx obj;
	obj.loadMMF();
	obj.DRLEncode();
	std::cout << obj.rows;
	std::cout << "\n";
	obj.Transform(VERTICAL);
	std::cout << obj.rows;
	//obj.Print();
	//obj.DRLEncode();
	//std::cout << obj.rows;
	//obj.Print();
}
