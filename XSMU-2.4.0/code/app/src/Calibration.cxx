#include "app/Calibration.h"
#include <math.h>

#define HEADROOM    1.02

template <typename T, typename U, uint8_t N>
float CalibrationTable<T,U,N>::find_first (const U& u) const
{
	const CalibrationTable<T, U, N>& table = *this;

	uint_fast8_t left = 0, right = size() - 1;

	if (u < table[left].second())
	{
		right = left + 1;

		const float slope =
			(table[right].first() - table[left].first()) /
				(table[right].second() - table[left].second());

		const float rv =
			slope * (u - table[left].second()) + table[left].first();

		return (rv < HEADROOM * table[left].first()) ?
			(HEADROOM * table[left].first()) : rv;
	}

	else if (u > table[right].second())
	{
		left = right - 1;

		const float slope =
			(table[right].first() - table[left].first()) /
				(table[right].second() - table[left].second());

		const float rv =
			slope * (u - table[left].second()) + table[left].first();

		return (rv > HEADROOM * table[right].first()) ?
			(HEADROOM * table[right].first()) : rv;
	}

	else do {

		const uint_fast8_t middle = (left + right) / 2;

		if (u < table[middle].second())
			right = middle;

		else if (u > table[middle].second())
			left = middle;

		else
			return table[middle].first();

	} while (right - left > 1);

	const float slope =
		(table[right].first() - table[left].first()) /
			(table[right].second() - table[left].second());

	return slope * (u - table[left].second()) + table[left].first();
}

template <typename T, typename U, uint8_t N>
float CalibrationTable<T,U,N>::find_second (const T& t) const
{
	const CalibrationTable<T, U, N>& table = *this;

	uint_fast8_t left = 0, right = size() - 1;

	if (t < table[left].first()) {

		right = left + 1;

		const float slope =
			(table[right].second() - table[left].second()) /
				(table[right].first() - table[left].first());

		const float rv =
			slope * (t - table[left].first()) + table[left].second();

		return (rv < HEADROOM * table[left].second()) ?
			(HEADROOM * table[left].second()) : rv;
	}

	else if (t > table[right].first()) {

		left = right - 1;

		const float slope =
			(table[right].second() - table[left].second()) /
				(table[right].first() - table[left].first());

		const float rv =
			slope * (t - table[left].first()) + table[left].second();

		return (rv > HEADROOM * table[right].second()) ?
			(HEADROOM * table[right].second()) : rv;
	}

	else do {

		const uint_fast8_t middle = (left + right) / 2;

		if (t < table[middle].first())
			right = middle;

		else if (t > table[middle].first())
			left = middle;

		else
			return table[middle].second();

	} while (right - left > 1);

	const float slope =
		(table[right].second() - table[left].second()) /
			(table[right].first() - table[left].first());

	return slope * (t - table[left].first()) + table[left].second();
}

/************************************************************************/
/************************************************************************/

float CS_CalibrationTable::find_current (int16_t dac) const
{
	return find_second (dac);
}

int16_t CS_CalibrationTable::find_dac (float current) const
{
	return round (find_first (current));
}

/************************************************************************/

float VS_CalibrationTable::find_voltage (int16_t dac) const
{
	return find_second (dac);
}

int16_t VS_CalibrationTable::find_dac (float voltage) const
{
	return round (find_first (voltage));
}

/***********************************************************************/
/***********************************************************************/

float CM_CalibrationTable::find_current (int32_t adc) const
{
	return find_second (adc);
}

int32_t CM_CalibrationTable::find_adc (float current) const
{
	return round (find_first (current));
}

/************************************************************************/

float VM_CalibrationTable::find_voltage (int32_t adc) const
{
	return find_second (adc);
}

int32_t VM_CalibrationTable::find_adc (float voltage) const
{
	return round (find_first (voltage));
}

/************************************************************************/
/************************************************************************/
