#include "utils/Filter.h"
#include <stdlib.h>

SimpleAverageFilter::SimpleAverageFilter (uint16_t size) :
	sum_ (0),
	capacity_ (size),
	free_ (size)
{}

void SimpleAverageFilter::reset (void)
{
	sum_ = 0;
	free_ = capacity_;
}

void SimpleAverageFilter::push_back (int32_t o)
{
	if (!full()) {

		--free_;
		sum_ += o;
	}
}

double SimpleAverageFilter::output (void) const
{
	return (double)sum_ / (capacity_ - free_);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MedianFilter::MedianFilter (uint16_t size) :
	data_ (0),
	capacity_ (size),
	occupied_ (0),
	sorted_ (true)
{
	data_ = (int32_t*) malloc (size * sizeof (int32_t));
}

MedianFilter::~MedianFilter (void)
{
	free (data_);
}

void MedianFilter::reset (void)
{
	occupied_ = 0;
	sorted_ = true;
}

void MedianFilter::push_back (int32_t o)
{
	if (!full()) {
		data_[occupied_++] = o;
		sorted_ = false;
	}
}

void MedianFilter::swap (int32_t& a, int32_t& b)
{
	const int32_t _a = a, _b = b;
	a = _b; b = _a;
}

void MedianFilter::sort (void)
{
	if (!empty())
		for (uint16_t i = 0; i < occupied_; ++i)
			for (uint16_t j = 0; j < occupied_ - i - 1; ++j)
				if (data_[j] > data_[j + 1])
					swap (data_[j], data_[j + 1]);

	sorted_ = true;
}

double MedianFilter::output (void) const
{
	double output = 0.0;

	if (!empty()) {

		if (!sorted_)
			const_cast<MedianFilter*> (this)->sort();

		output = (occupied_ % 2 == 0) ?
			((double)(data_[occupied_ / 2] + data_[occupied_ / 2 - 1]) / 2) :
			(data_[occupied_ / 2]);
	}

	return output;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
