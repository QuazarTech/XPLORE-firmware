#ifndef __CM__
#define __CM__

#include "app/Calibration.h"
#include "sys/PCA9557.h"
#include "sys/AD7734.h"
#include "utils/Filter.h"
#include <stdint.h>

enum CM_Range
{
	CM_RANGE_10uA,
	CM_RANGE_100uA,
	CM_RANGE_1mA,
	CM_RANGE_10mA,
	CM_RANGE_100mA,
};

CM_Range toCM_Range (uint16_t i);

class AD7734_CM : public AD7734
{
public:
	AD7734_CM (void);
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

class CM
{
public:
	static CM* get_singleton (void);

public:
	const CM_Range& range (void) const {return range_;}
	const CM_CalibrationTable& calibration (void) const {
		return calibration_;
	}

public:
	void setRange (CM_Range range);

public:
	float readCurrent (void);
	float readCurrent (uint16_t filterLength);

public:
	void setCalibration  (uint16_t index, float current);
	void saveCalibration (void);
	void fillDefaultCalibration (void);

public:
	void check (void);
	bool data_ready (void) { return filter_.full(); }

private:
	CM_Range range_;
	CM_CalibrationTable calibration_;
	PCA9557 iox_;
	AD7734_CM adc_;
	SimpleAverageFilter filter_;

private:
	CM (void);

	void fillDefaultCalibration (CM_Range range,
								 CM_CalibrationTable* calibration);

	void fillDefaultCalibration (float I_FS,
								 CM_CalibrationTable* calibration);
private:
    Storage* storage;
};

#endif
