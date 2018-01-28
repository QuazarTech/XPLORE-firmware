#ifndef __VM2__
#define __VM2__

#include "app/Calibration.h"
#include "sys/AD7734.h"
#include <stdint.h>

enum VM2_Range
{
	VM2_RANGE_10V
};

VM2_Range toVM2_Range (uint16_t i);

class AD7734_VM2 : public AD7734
{
public:
	AD7734_VM2 (void);
	int32_t read (void);

protected:
	virtual void selectLow (void);
	virtual void selectHigh (void);

	virtual void resetLow (void);
	virtual void resetHigh (void);

	virtual bool data_ready (void);
	virtual void initializeInterface (void);

// 	virtual void synchronize (void);
};

class Storage;
class VM2
{
public:
	static VM2* get_singleton (void) {static auto o = new VM2; return o;}

public:
	const VM2_Range& range (void) const {return range_;}

	const VM_CalibrationTable&
		calibration (void) const {return calibration_;}

public:
	void setRange (VM2_Range range);
	float readVoltage (uint16_t filterLength);

public:
	void saveCalibration (void);
	void fillDefaultCalibration (void);
	void setCalibration (uint16_t index, float voltage);

private:
	void fillDefaultCalibration (float V_FS,
								 VM_CalibrationTable* calibration);

	void fillDefaultCalibration (VM2_Range range,
								 VM_CalibrationTable* calibration);

private:
	AD7734_VM2 adc_;
	VM2_Range range_;
	VM_CalibrationTable calibration_;
	Storage* storage;

private:
	VM2 (void);
};

#endif
