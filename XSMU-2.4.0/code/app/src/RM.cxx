#include "app/RM.h"

// #include "app/CS.h"
// #include "app/CM.h"
// #include "app/VM.h"

#include "sys/hardware.h"
#include <util/delay.h>

#include "pgmspace"

using namespace std;

RM_Range toRM_Range (uint8_t i)
{
	static const RM_Range ranges[] PROGMEM =
	{
		RM_RANGE_10E,
		RM_RANGE_100E,
		RM_RANGE_1k,
		RM_RANGE_10k,
		RM_RANGE_100k,
		RM_RANGE_1M,
		RM_RANGE_10M
	};

	return pgm_read ((i < sizeof (ranges) / sizeof (ranges[0])) ?
		ranges[i] : ranges[0]);
}

/**************************************************************************/

RM::RM (void) :
	range_ (RM_RANGE_10k)
{}

RM* RM::get_singleton (void)
{
	static auto o = new RM;
	return o;
}

float RM::readResistance (uint16_t filterLength)
{
	int8_t rangeInc;

	do {

		rangeInc = range_ok (range(), 16);

		if (rangeInc == 0)
			return readResistance (range(), filterLength);

		else if ((rangeInc == -1) && (range() == RM_RANGE_10E))
			return readResistance (range(), filterLength);

		else if ((rangeInc == +1) && (range() == RM_RANGE_10M))
			return -1;

		setRange (toRM_Range (range() + rangeInc));

	} while (rangeInc != 0);

	return 0; // The control will never reach this point.
}

/***************************************************************************/

int8_t RM::range_ok (RM_Range range, uint16_t filterLength)
{
	struct RangeSetting
	{
		CS_Range cs_range;
		CM_Range cm_range;
		VM_Range vm_range;
		float   current;
		float   VMax;
	};

	static const RangeSetting rangeSettings[] PROGMEM =
	{
		{ CS_RANGE_10mA,  CM_RANGE_10mA,  VM_RANGE_100mV,  10e-3, 100e-3 },
		{ CS_RANGE_10mA,  CM_RANGE_10mA,  VM_RANGE_1V,     10e-3,  1.0   },
		{ CS_RANGE_10mA,  CM_RANGE_10mA,  VM_RANGE_10V,    10e-3, 10.0   },
		{ CS_RANGE_1mA,   CM_RANGE_1mA,   VM_RANGE_10V,     1e-3, 10.0   },
		{ CS_RANGE_100uA, CM_RANGE_100uA, VM_RANGE_10V,   100e-6, 10.0   },
		{ CS_RANGE_100uA, CM_RANGE_100uA, VM_RANGE_10V,    10e-6, 10.0   },
		{ CS_RANGE_100uA, CM_RANGE_100uA, VM_RANGE_10V,     1e-6, 10.0   },
	};

	modCS->setRange   (pgm_read (rangeSettings[range].cs_range));
	modCM->setRange   (pgm_read (rangeSettings[range].cm_range));
	modVM->setRange   (pgm_read (rangeSettings[range].vm_range));
	modCS->setCurrent (pgm_read (rangeSettings[range].current));
	_delay_ms (100);

	const float V = fabs (modVM->readVoltage (filterLength));

	if (V > pgm_read (rangeSettings[range].VMax))
		return +1;

	else if (V < 0.09 * pgm_read (rangeSettings[range].VMax))
		return -1;

	else
		return 0;
}

/***************************************************************************/

float RM::readResistance (RM_Range range, uint16_t filterLength)
{
	struct RangeSetting
	{
		CS_Range cs_range;
		CM_Range cm_range;
		VM_Range vm_range;
		float    current;
	};

	static const RangeSetting rangeSettings[] PROGMEM =
	{
		{ CS_RANGE_10mA,  CM_RANGE_10mA,  VM_RANGE_100mV,  10e-3 },
		{ CS_RANGE_10mA,  CM_RANGE_10mA,  VM_RANGE_1V,     10e-3 },
		{ CS_RANGE_10mA,  CM_RANGE_10mA,  VM_RANGE_10V,    10e-3 },
		{ CS_RANGE_1mA,   CM_RANGE_1mA,   VM_RANGE_10V,     1e-3 },
		{ CS_RANGE_100uA, CM_RANGE_100uA, VM_RANGE_10V,   100e-6 },
		{ CS_RANGE_100uA, CM_RANGE_100uA, VM_RANGE_10V,    10e-6 },
		{ CS_RANGE_100uA, CM_RANGE_100uA, VM_RANGE_10V,     1e-6 },
	};

	modCS->setRange   (pgm_read (rangeSettings[range].cs_range));
	modCM->setRange   (pgm_read (rangeSettings[range].cm_range));
	modVM->setRange   (pgm_read (rangeSettings[range].vm_range));

	modCS->setCurrent (pgm_read (rangeSettings[range].current));
	_delay_ms (100);

	const float Ipos = modCM->readCurrent (filterLength);
	const float Vpos = modVM->readVoltage (filterLength);

	modCS->setCurrent (-pgm_read (rangeSettings[range].current));
	_delay_ms (100);

	const float Ineg = modCM->readCurrent (filterLength);
	const float Vneg = modVM->readVoltage (filterLength);

	return fabs (Vpos - Vneg) / fabs (Ipos - Ineg);
}

/***************************************************************************/
