#ifndef __VM__
#define __VM__

#include "app/Calibration.h"
#include "utils/Filter.h"
#include "sys/PCA9557.h"
#include "sys/AD7734.h"
#include <stdint.h>

enum VM_Range
{
	VM_RANGE_1mV,
	VM_RANGE_10mV,
	VM_RANGE_100mV,
	VM_RANGE_1V,
	VM_RANGE_10V,
	VM_RANGE_100V
};

enum VM_Terminal
{
	VM_TERMINAL_MEASUREMENT,
	VM_TERMINAL_SOURCE,
};

VM_Range    toVM_Range    (uint16_t i);
VM_Terminal toVM_Terminal (uint16_t i);

class AD7734_VM : public AD7734
{
public:
	AD7734_VM (void);
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

class VM
{
public:
	static VM& _ (void);

public:
	const VM_Range& range (void) const {return range_;}
	const VM_CalibrationTable& calibration (void) const {
		return calibration_;
	}

public:
	void setRange (VM_Range range);

public:
	float readVoltage (void);
	float readVoltage (uint16_t filterLength);

public:
	void setCalibration (uint16_t index, float voltage);
	void saveCalibration (void);
	void fillDefaultCalibration (void);

public:
	void activateTare (void);
	void activateInternalCalibration (void);
	void activateExternalMeasurement (void);

public:
	void check (void);
	bool data_ready (void) const { return filter_.full(); }

public:
	void setTerminal (VM_Terminal terminal);
	VM_Terminal getTerminal () {return terminal_;}

private:
	VM_Range range_;
	VM_CalibrationTable calibration_;
	PCA9557 iox_;
	AD7734_VM adc_;
	SimpleAverageFilter filter_;
	PCA9557 iox2_;
	VM_Terminal terminal_;

private:
	VM (void);
	void fillDefaultCalibration (VM_Range range,
								 VM_CalibrationTable* calibration);

	void fillDefaultCalibration (float V_FS,
								 VM_CalibrationTable* calibration);
};

#define modVM    VM::_()

#endif
