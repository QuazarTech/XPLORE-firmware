#ifndef __CS__
#define __CS__

#include "app/Calibration.h"
#include "sys/PCA9557.h"
#include "sys/AD5542.h"
#include <stdint.h>

class AD5542_CS : public AD5542
{
public:
	AD5542_CS (void);

protected:
	virtual void selectLow (void);
	virtual void selectHigh (void);
	virtual void initializeInterface (void);
};

enum CS_Range
{
	CS_RANGE_10uA,
	CS_RANGE_100uA,
	CS_RANGE_1mA,
	CS_RANGE_10mA,
	CS_RANGE_100mA,
};

CS_Range toCS_Range (uint16_t i);

class CS
{
public:
	static CS& _ (void);

public:
	const CS_Range& range (void) const {return range_;}
	const float& current (void) const {return current_;}
	const CS_CalibrationTable& calibration (void) const {
		return calibration_;
	}

public:
	void activate (void);
	void deactivate (void);
	void setRange (CS_Range range);
	void setCurrent (float current);
	bool active (void) const {return active_;}
	void fillDefaultCalibration (void);

public:
	void verifyCalibration (uint16_t index);
	void setCalibration (uint16_t index, float current);
	void saveCalibration (void);

private:
	float current_;
	CS_Range range_;
	CS_CalibrationTable calibration_;
	PCA9557 iox_;
	AD5542_CS dac_;
	bool active_;

private:
	CS (void);

	void fillDefaultCalibration (CS_Range range,
								 CS_CalibrationTable* calibration);

	void fillDefaultCalibration (float I_FS,
								 CS_CalibrationTable* calibration);

	void setDAC (int16_t dac);
};

#define modCS    CS::_()

#endif
