#ifndef __VS__
#define __VS__

#include "app/Calibration.h"
#include "sys/PCA9557.h"
#include "sys/AD5542.h"
#include <stdint.h>

class AD5542_VS : public AD5542
{
public:
	AD5542_VS (void);

protected:
	virtual void selectLow (void);
	virtual void selectHigh (void);
	virtual void initializeInterface (void);
};

enum VS_Range
{
	VS_RANGE_10V,
	VS_RANGE_100V
};

VS_Range toVS_Range (uint16_t i);

class VS
{
public:
	static VS& _ (void);

public:
	const VS_Range& range (void) const {return range_;}
	const float& voltage (void) const {return voltage_;}
	const VS_CalibrationTable& calibration (void) const {
		return calibration_;
	}

public:
	void activate (void);
	void deactivate (void);
	void setRange (VS_Range range);
	void setVoltage (float voltage);
	bool active (void) const { return active_; }
	void fillDefaultCalibration (void);

public:
	void verifyCalibration (uint16_t index);
	void setCalibration (uint16_t index, float voltage);
	void saveCalibration (void);

private:
	float voltage_;
	VS_Range range_;
	VS_CalibrationTable calibration_;
	PCA9557 iox_;
	AD5542_VS dac_;
	bool active_;

private:
	VS (void);

	void fillDefaultCalibration (VS_Range range,
								 VS_CalibrationTable* calibration);

	void fillDefaultCalibration (float V_FS,
								 VS_CalibrationTable* calibration);
	void setDAC (int16_t dac);
};

#define modVS    VS::_()

#endif
