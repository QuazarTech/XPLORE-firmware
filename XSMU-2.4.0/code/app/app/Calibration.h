#ifndef __CALIBRATION__
#define __CALIBRATION__

#include <utility>
#include <stdint.h>

template<typename T, typename U>
class CalibrationTableEntry
{
public:
	CalibrationTableEntry (void) :
		first_ (0), second_ (0)
			{}

public:
	void set (const T& first, const U& second) {
		first_ = first;
		second_ = second;
	}

public:
	const T& first  (void) const {return first_; }
	const U& second (void) const {return second_;}

private:
	T first_;
	U second_;
};

template <typename T, typename U, uint8_t N>
class CalibrationTable
{
public:
	CalibrationTableEntry<T, U>& operator[] (uint_fast8_t i) {
		return entries_[i];
	}

public:
	const CalibrationTableEntry<T, U>& operator[] (uint_fast8_t i) const {
		return entries_[i];
	}

public:
	uint8_t size (void) const {return N;}

public:
	float find_first (const U& u) const;
	float find_second (const T& t) const;

private:
	CalibrationTableEntry<T, U> entries_[N];
};

/***********************************************************************/
/***********************************************************************/

enum CalibrationTableSize
{
	CS_CALIBRATION_TABLE_SIZE = 5,
	VS_CALIBRATION_TABLE_SIZE = 5,
	CM_CALIBRATION_TABLE_SIZE = 5,
	VM_CALIBRATION_TABLE_SIZE = 5
};

/***********************************************************************/

class CS_CalibrationTable :
	public CalibrationTable<int16_t, float, CS_CALIBRATION_TABLE_SIZE>
{
public:
	float find_current (int16_t dac) const;
	int16_t find_dac (float current) const;
};

class VS_CalibrationTable :
	public CalibrationTable<int16_t, float, VS_CALIBRATION_TABLE_SIZE>
{
public:
	float find_voltage (int16_t dac) const;
	int16_t find_dac (float voltage) const;
};

/***********************************************************************/

class CM_CalibrationTable :
	public CalibrationTable<int32_t, float, CM_CALIBRATION_TABLE_SIZE>
{
public:
	float find_current (int32_t adc) const;
	int32_t find_adc (float current) const;
};

class VM_CalibrationTable :
	public CalibrationTable<int32_t, float, VM_CALIBRATION_TABLE_SIZE>
{
public:
	float find_voltage (int32_t adc) const;
	int32_t find_adc (float voltage) const;
};

/***********************************************************************/
/***********************************************************************/

#endif
