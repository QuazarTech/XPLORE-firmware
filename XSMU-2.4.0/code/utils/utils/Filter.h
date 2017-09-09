#ifndef __FILTER__
#define __FILTER__

#include <stdint.h>

class SimpleAverageFilter
{
public:
	SimpleAverageFilter (uint16_t size);

public:
	void reset (void);
	bool full (void) const { return (free_ == 0); }
	bool empty (void) const { return (free_ == capacity_); }

public:
	void push_back (int32_t o);
	double output (void) const;

private:
	int64_t sum_;
	uint16_t capacity_, free_;
};

class MedianFilter
{
public:
	MedianFilter (uint16_t size);
	~MedianFilter (void);

public:
	void reset (void);
	bool full (void) const { return (occupied_ == capacity_); }
	bool empty (void) const { return (occupied_ == 0); }

public:
	void push_back (int32_t o);
	double output (void) const;

private:
	int32_t* data_;
	uint16_t capacity_, occupied_;
	bool sorted_;
	void swap (int32_t& a, int32_t& b);
	void sort (void);
};

#endif
