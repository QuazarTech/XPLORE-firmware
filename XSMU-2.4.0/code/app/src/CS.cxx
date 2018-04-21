#include "app/CS.h"
#include "app/Storage.h"

#include <cmath>
#include <avr/io.h>
#include "pgmspace"

using namespace std;

/*********************************************************************/

#define DAC_SELECT_BIT                     7
#define DAC_SELECT_DDR                     DDRD
#define DAC_SELECT_PORT                    PORTD
#define DAC_SELECT_MASK                   (1 << DAC_SELECT_BIT)
#define DAC_SELECT_DDR_OUT                (1 << DAC_SELECT_BIT)
#define DAC_SELECT_LOW                    (0 << DAC_SELECT_BIT)
#define DAC_SELECT_HIGH                   (1 << DAC_SELECT_BIT)

AD5542_CS::AD5542_CS (void)
{
	initialize();
}

void AD5542_CS::selectLow (void)
{
	DAC_SELECT_PORT = (DAC_SELECT_PORT & ~DAC_SELECT_MASK) | DAC_SELECT_LOW;
}

void AD5542_CS::selectHigh (void)
{
	DAC_SELECT_PORT = (DAC_SELECT_PORT & ~DAC_SELECT_MASK) | DAC_SELECT_HIGH;
}

void AD5542_CS::initializeInterface (void)
{
	DAC_SELECT_DDR = (DAC_SELECT_DDR & ~DAC_SELECT_MASK) | DAC_SELECT_DDR_OUT;
}

/*********************************************************************/
/*********************************************************************/

#define IOX_ADDR                           0

#define IOX_RANGE_MASK                    (0xF << 0)
#define IOX_RANGE_DIR_OUTPUT              (0x0 << 0)
#define IOX_RANGE_POLARITY_NORMAL         (0x0 << 0)

#define IOX_RANGE_10uA                    (0x8 << 0)
#define IOX_RANGE_100uA                   (0x4 << 0)
#define IOX_RANGE_1mA                     (0x2 << 0)
#define IOX_RANGE_10mA                    (0x1 << 0)
#define IOX_RANGE_100mA                   (0x0 << 0)

#define IOX_MODE_MASK                     (1 << 4)
#define IOX_MODE_DIR_OUTPUT               (0 << 4)
#define IOX_MODE_POLRITY_NORMAL           (0 << 4)

#define IOX_MODE_CS                       (0 << 4)
#define IOX_MODE_VS                       (1 << 4)

#define IOX_COMPLIANCE_MASK               (1 << 5)
#define IOX_COMPLIANCE_DIR_OUTPUT         (0 << 5)
#define IOX_COMPLIANCE_POLARITY_NORMAL    (0 << 5)

#define IOX_COMPLIANCE_10V                (0 << 5)
#define IOX_COMPLIANCE_100V               (1 << 5)

#define IOX_MASK ( \
	IOX_RANGE_MASK | \
	IOX_MODE_MASK  | \
	IOX_COMPLIANCE_MASK \
)

#define IOX_DIR ( \
	IOX_RANGE_DIR_OUTPUT | \
	IOX_MODE_DIR_OUTPUT  | \
	IOX_COMPLIANCE_DIR_OUTPUT \
)

#define IOX_POLARITY ( \
	IOX_RANGE_POLARITY_NORMAL | \
	IOX_MODE_POLRITY_NORMAL   | \
	IOX_COMPLIANCE_POLARITY_NORMAL \
)

CS_Range toCS_Range (uint16_t i)
{
	static const CS_Range ranges[] PROGMEM =
	{
		CS_RANGE_10uA,
		CS_RANGE_100uA,
		CS_RANGE_1mA,
		CS_RANGE_10mA,
		CS_RANGE_100mA,
	};

	return pgm_read ((i < sizeof (ranges) / sizeof (ranges[0])) ?
		ranges[i] : ranges[0]);
}

/*********************************************************************/

CS* CS::get_singleton (void)
{
	static auto o = new CS;
	return o;
}

CS::CS (void) :
	current_ (0.0),
	range_ (CS_RANGE_1mA),
	iox_ (IOX_ADDR),
	active_ (false),
	storage (Storage::get_singleton())
{
	iox_.setPinDirection ((iox_.getPinDirection() & ~IOX_MASK) | IOX_DIR);
	iox_.setPinPolarity  ((iox_.getPinPolarity() & ~IOX_MASK)  | IOX_POLARITY);

	setRange (CS_RANGE_1mA);
	setCurrent (0);
}

/*********************************************************************/

void CS::activate (void)
{
	active_ = true;

	iox_.writeOutputPortRegister (
		(iox_.readOutputPortRegister() & ~IOX_MODE_MASK) | IOX_MODE_CS);

	setRange (range());
}

void CS::deactivate (void)
{
	active_ = false;
}

/*********************************************************************/

void CS::setRange (CS_Range range)
{
	range_ = range;

	if (storage->read (toStorage_CS_FileNo (range), &calibration_) !=
		sizeof (calibration_)) {

			fillDefaultCalibration (range, &calibration_);
			saveCalibration();
	}

	static const uint8_t iox_data[] PROGMEM =
	{
		IOX_RANGE_10uA,
		IOX_RANGE_100uA,
		IOX_RANGE_1mA,
		IOX_RANGE_10mA,
		IOX_RANGE_100mA
	};

	iox_.writeOutputPortRegister (
		(iox_.readOutputPortRegister() & ~IOX_RANGE_MASK) |
		pgm_read (iox_data[range]));

	if (active())
		setCurrent (current());
}

/*********************************************************************/

void CS::setCurrent (float current)
{
	int16_t dac = calibration_.find_dac (current);
	current_ = calibration_.find_current (dac);
	setDAC (dac);
}

void CS::setDAC (int16_t dac)
{
	dac_.write (0x8000 + dac);
}

/*********************************************************************/

#define DAC_FS     2.5
#define DAC_GAIN   (1 + 50e3 / (10e3 + 5.76e3))
#define VSET_FS    1.0

void CS::fillDefaultCalibration (void)
{
	fillDefaultCalibration (range(), &calibration_);
}

void CS::fillDefaultCalibration (float I_FS,
								 CS_CalibrationTable* calibration)
{
	const int16_t dac_max = (int16_t)(0x7FFF * (
		VSET_FS / (DAC_FS * DAC_GAIN)) + 0.5);

 	const int16_t dac_min = -dac_max;
	const float frac = 0.9;

	(*calibration)[0].set (dac_min,                          -I_FS       );
	(*calibration)[1].set ((int16_t) round (dac_min * frac), -I_FS * frac);
	(*calibration)[2].set (0,                                 0.0        );
	(*calibration)[3].set ((int16_t) round (dac_max * frac),  I_FS * frac);
	(*calibration)[4].set (dac_max,                           I_FS       );
}

void CS::fillDefaultCalibration (CS_Range range,
								 CS_CalibrationTable* calibration)
{
	static const float FS[] PROGMEM =
	{
		10e-6,
		100e-6,
		1e-3,
		10e-3,
		100e-3
	};

	fillDefaultCalibration (pgm_read (FS[range]), calibration);
}

/*********************************************************************/

void CS::verifyCalibration (uint16_t index)
{
	const int16_t dac = calibration_[index].first();
	setDAC (dac);
}

void CS::setCalibration (uint16_t index, float current)
{
	const int16_t dac = calibration_[index].first();
	calibration_[index].set (dac, current);
}

void CS::saveCalibration (void)
{
	storage->write (toStorage_CS_FileNo (range()), &calibration_);
}

/*********************************************************************/
/*********************************************************************/
