#include "app/VS.h"
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

AD5542_VS::AD5542_VS (void)
{
	initialize();
}

void AD5542_VS::selectLow (void)
{
	DAC_SELECT_PORT = (DAC_SELECT_PORT & ~DAC_SELECT_MASK) | DAC_SELECT_LOW;
}

void AD5542_VS::selectHigh (void)
{
	DAC_SELECT_PORT = (DAC_SELECT_PORT & ~DAC_SELECT_MASK) | DAC_SELECT_HIGH;
}

void AD5542_VS::initializeInterface (void)
{
	DAC_SELECT_DDR = (DAC_SELECT_DDR & ~DAC_SELECT_MASK) | DAC_SELECT_DDR_OUT;
}

/*********************************************************************/
/*********************************************************************/

#define IOX_ADDR                      0

#define IOX_RANGE_MASK               (1 << 5)
#define IOX_RANGE_DIR_OUTPUT         (0 << 5)
#define IOX_RANGE_POLARITY_NORMAL    (0 << 5)

#define IOX_RANGE_10V                (0 << 5)
#define IOX_RANGE_100V               (1 << 5)

#define IOX_MODE_MASK                (1 << 4)
#define IOX_MODE_DIR_OUTPUT          (0 << 4)
#define IOX_MODE_POLRITY_NORMAL      (0 << 4)

#define IOX_MODE_CS                  (0 << 4)
#define IOX_MODE_VS                  (1 << 4)

#define IOX_MASK     (IOX_RANGE_MASK | IOX_MODE_MASK)
#define IOX_DIR      (IOX_RANGE_DIR_OUTPUT | IOX_MODE_DIR_OUTPUT )
#define IOX_POLARITY (IOX_RANGE_POLARITY_NORMAL | IOX_MODE_POLRITY_NORMAL)

VS_Range toVS_Range (uint16_t i)
{
	static const VS_Range ranges[] PROGMEM =
	{
		VS_RANGE_10V,
		VS_RANGE_100V
	};

	return pgm_read ((i < sizeof (ranges) / sizeof (ranges[0])) ?
		ranges[i] : ranges[0]);
}

/*********************************************************************/

VS* VS::get_singleton (void)
{
	static auto o = new VS;
	return o;
}

VS::VS (void) :
	voltage_ (0.0),
	range_ (VS_RANGE_10V),
	iox_ (IOX_ADDR),
	active_ (false),
	storage (Storage::get_singleton())
{
	iox_.setPinDirection ((iox_.getPinDirection() & ~IOX_MASK) | IOX_DIR);
	iox_.setPinPolarity  ((iox_.getPinPolarity() & ~IOX_MASK)  | IOX_POLARITY);

	setRange (VS_RANGE_10V);
	setVoltage (0);
}

/*********************************************************************/

void VS::activate (void)
{
	active_ = true;

	iox_.writeOutputPortRegister (
		(iox_.readOutputPortRegister() & ~IOX_MODE_MASK) | IOX_MODE_VS);

	setRange (range());
	setVoltage (voltage());
}

void VS::deactivate (void)
{
	active_ = false;
}

/*********************************************************************/

void VS::setRange (VS_Range range)
{
	range_ = range;

	if (storage->read (toStorage_VS_FileNo (range), &calibration_) !=
		sizeof (calibration_)) {

			fillDefaultCalibration (range, &calibration_);
			saveCalibration();
	}

	static const uint8_t iox_data[] PROGMEM =
	{
		IOX_RANGE_10V,
		IOX_RANGE_100V
	};

	iox_.writeOutputPortRegister (
		(iox_.readOutputPortRegister() & ~IOX_RANGE_MASK) |
		pgm_read (iox_data[range]));

	setVoltage (voltage());
}

/*********************************************************************/

void VS::setVoltage (float voltage)
{
	int16_t dac = calibration_.find_dac (voltage);
	voltage_ = calibration_.find_voltage (dac);
	setDAC (dac);
}

void VS::setDAC (int16_t dac)
{
	dac_.write (0x8000 + dac);
}

/*********************************************************************/

#define DAC_FS     2.5
#define DAC_GAIN   (1 + 50e3 / (10e3 + 5.76e3))
#define VSET_FS    10.0

void VS::fillDefaultCalibration (void)
{
	fillDefaultCalibration (range(), &calibration_);
}

void VS::fillDefaultCalibration (float V_FS,
								 VS_CalibrationTable* calibration)
{
	const int16_t dac_max = (int16_t)(0x7FFF * (
		VSET_FS / (DAC_FS * DAC_GAIN)) + 0.5);

	const int16_t dac_min = -dac_max;
	const float frac = 0.9;

	(*calibration)[0].set (dac_min,                          -V_FS       );
	(*calibration)[1].set ((int16_t) round (dac_min * frac), -V_FS * frac);
	(*calibration)[2].set (0,                                 0.0        );
	(*calibration)[3].set ((int16_t) round (dac_max * frac),  V_FS * frac);
	(*calibration)[4].set (dac_max,                           V_FS       );
}

void VS::fillDefaultCalibration (VS_Range range,
								 VS_CalibrationTable* calibration)
{
	static const float FS[] PROGMEM =
	{
		10.0,
		100.0
	};

	fillDefaultCalibration (pgm_read (FS[range]), calibration);
}

/*********************************************************************/

void VS::verifyCalibration (uint16_t index)
{
	const int16_t dac = calibration_[index].first();
	setDAC (dac);
}

void VS::setCalibration (uint16_t index, float voltage)
{
	const int16_t dac = calibration_[index].first();
	calibration_[index].set (dac, voltage);
}

void VS::saveCalibration (void)
{
	storage->write (toStorage_VS_FileNo (range()), &calibration_);
}

/*********************************************************************/
/*********************************************************************/
