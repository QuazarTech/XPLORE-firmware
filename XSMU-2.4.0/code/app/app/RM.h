#ifndef __RM__
#define __RM__

#include <stdint.h>

enum RM_Range
{
	RM_RANGE_10E,
	RM_RANGE_100E,
	RM_RANGE_1k,
	RM_RANGE_10k,
	RM_RANGE_100k,
	RM_RANGE_1M,
	RM_RANGE_10M
};

RM_Range toRM_Range (uint8_t i);

class RM
{
public:
	static RM& _ (void);
	float readResistance (uint16_t filterLength);
	float readResistance (RM_Range range, uint16_t filterLength);

public:
	RM_Range range (void) const { return range_; }

private:
	RM (void);
	void setRange (RM_Range range) { range_ = range; }
	int8_t range_ok (RM_Range range, uint16_t filterLength);

private:
	RM_Range range_;
};

#define modRM    RM::_()

#endif
