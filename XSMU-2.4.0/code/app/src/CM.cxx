#include "app/CM.h"
#include "app/Storage.h"
// #include "sys/hardware.h"

#include <cmath>
#include <avr/io.h>
// #include <util/delay.h>

#include "pgmspace"
#include "app/SystemConfig.h"
#include "app/firmware_version.h"

using namespace std;

#define IOX_ADDR                           0

#define IOX_RANGE_MASK                    (0xF << 0)
#define IOX_RANGE_DIR                     (0x0 << 0)
#define IOX_RANGE_POLARITY                (0x0 << 0)

#define IOX_RANGE_10uA                    (0x8 << 0)
#define IOX_RANGE_100uA                   (0x4 << 0)
#define IOX_RANGE_1mA                     (0x2 << 0)
#define IOX_RANGE_10mA                    (0x1 << 0)
#define IOX_RANGE_100mA                   (0x0 << 0)

#define ADC_CHN                            0

// PD4 -> DIO0 -> AI1 -> CS
#define ADC_SELECT_BIT                     4
#define ADC_SELECT_DDR                     DDRD
#define ADC_SELECT_PORT                    PORTD
#define ADC_SELECT_MASK                   (1 << ADC_SELECT_BIT)
#define ADC_SELECT_DDR_OUT                (1 << ADC_SELECT_BIT)
#define ADC_SELECT_LOW                    (0 << ADC_SELECT_BIT)
#define ADC_SELECT_HIGH                   (1 << ADC_SELECT_BIT)

// PD5 <- DIO1 <- AO1 <- RDY
#define ADC_DRDY_BIT                       5
#define ADC_DRDY_DDR                       DDRD
#define ADC_DRDY_PORT                      PIND
#define ADC_DRDY_MASK                     (1 << ADC_DRDY_BIT)
#define ADC_DRDY_DDR_IN                   (0 << ADC_DRDY_BIT)

// ADC synchronization
// #define ADC_SYNC_BIT                     3
// #define ADC_SYNC_DDR                     DDRD
// #define ADC_SYNC_PORT                    PIND
// #define ADC_SYNC_MASK                   (1 << ADC_SYNC_BIT)
// #define ADC_SYNC_DDR_IN                 (0 << ADC_SYNC_BIT)

/*********************************************************************/

AD7734_CM::AD7734_CM (void)
{
	initialize();
}

void AD7734_CM::initializeInterface (void)
{
	ADC_SELECT_DDR = (ADC_SELECT_DDR & ~ADC_SELECT_MASK) | ADC_SELECT_DDR_OUT;
	ADC_DRDY_DDR   = (ADC_DRDY_DDR   & ~ADC_DRDY_MASK  ) | ADC_DRDY_DDR_IN;
// 	ADC_SYNC_DDR   = (ADC_SYNC_DDR   & ~ADC_SYNC_MASK  ) | ADC_SYNC_DDR_IN;
}

void AD7734_CM::selectLow (void)
{
	ADC_SELECT_PORT = (ADC_SELECT_PORT & ~ADC_SELECT_MASK) | ADC_SELECT_LOW;
}

void AD7734_CM::selectHigh (void)
{
	ADC_SELECT_PORT = (ADC_SELECT_PORT & ~ADC_SELECT_MASK) | ADC_SELECT_HIGH;
}

void AD7734_CM::resetLow (void)
{}

void AD7734_CM::resetHigh (void)
{}

bool AD7734_CM::data_ready (void)
{
	return (ADC_DRDY_PORT & ADC_DRDY_MASK) ? false : true;
}

// void AD7734_CM::synchronize (void)
// {
// 	/**** Waits for a zero-to-one transmition in sync input ****/
//
// 	for (uint16_t loop = 10000; loop; --loop, _delay_us (1))
// 		if (!(ADC_SYNC_PORT & ADC_SYNC_MASK))
// 			break;
//
// 	for (uint16_t loop = 10000; loop; --loop, _delay_us (1))
// 		if (ADC_SYNC_PORT & ADC_SYNC_MASK)
// 			break;
// }

int32_t AD7734_CM::read (void)
{
	const uint32_t hardware_version =
		MAKE_VERSION (system_config.hwBoardNo(),
					  system_config.hwBomNo(),
					  system_config.hwBugfixNo());

	if (hardware_version < MAKE_VERSION (3,0,0))
			return AD7734::read (ADC_CHN);
	else
			return -AD7734::read (ADC_CHN);
}

/*********************************************************************/

CM_Range toCM_Range (uint16_t i)
{
	static const CM_Range ranges[] PROGMEM =
	{
		CM_RANGE_10uA,
		CM_RANGE_100uA,
		CM_RANGE_1mA,
		CM_RANGE_10mA,
		CM_RANGE_100mA,
	};

	return pgm_read ((i < sizeof (ranges) / sizeof (ranges[0])) ?
		ranges[i] : ranges[0]);
}

/*********************************************************************/

CM& CM::_ (void)
{
	static CM o;
	return o;
}

CM::CM (void) :
	iox_ (0),
	filter_ (16)
{
	iox_.setPinDirection (
		(iox_.getPinDirection() & ~IOX_RANGE_MASK) | IOX_RANGE_DIR);

	iox_.setPinPolarity  (
		(iox_.getPinPolarity() & ~IOX_RANGE_MASK) | IOX_RANGE_POLARITY);

	setRange (CM_RANGE_1mA);
}

/*********************************************************************/

void CM::setRange (CM_Range range)
{
	range_ = range;

	if (storage.read (toStorage_CM_FileNo (range), &calibration_) !=
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
}

/*********************************************************************/

float CM::readCurrent (uint16_t filterLength)
{
	MedianFilter filter ((filterLength > 256) ? 256 : filterLength);

	while (!filter.full())
		filter.push_back (adc_.read());

	return calibration_.find_current (round (filter.output()));
}

/*********************************************************************/

void CM::fillDefaultCalibration (void)
{
	fillDefaultCalibration (range(), &calibration_);
}

#define ADC_FS     10.0 // V
#define CM_FS       1.0 // V

void CM::fillDefaultCalibration (float I_FS,
								 CM_CalibrationTable* calibration)
{
	const int32_t adc_max = (int32_t) round (0x7FFFFF * (CM_FS / ADC_FS));
	const int32_t adc_min = -adc_max;
	const float frac = 0.9;

	(*calibration)[0].set (adc_min,                          -I_FS       );
	(*calibration)[1].set ((int32_t) round (adc_min * frac), -I_FS * frac);
	(*calibration)[2].set (0,                                 0.0        );
	(*calibration)[3].set ((int32_t) round (adc_max * frac),  I_FS * frac);
	(*calibration)[4].set (adc_max,                           I_FS       );
}

void CM::fillDefaultCalibration (CM_Range range,
								 CM_CalibrationTable* calibration)
{
	static const float I_FS[] PROGMEM =
	{
		10e-6, 100e-6, 1e-3, 10e-3, 100e-3
	};

	fillDefaultCalibration (pgm_read (I_FS[range]), calibration);
}

/*********************************************************************/

void CM::setCalibration (uint16_t index, float current)
{
	SimpleAverageFilter filter (128);

	while (!filter.full())
		filter.push_back (adc_.read());

	const int32_t adc = round (filter.output());
	calibration_[index].set (adc, current);
}

void CM::saveCalibration (void)
{
	storage.write (toStorage_CM_FileNo (range()), &calibration_);
}

/*********************************************************************/
/*********************************************************************/

void CM::check (void)
{
	filter_.push_back (adc_.read());
}

float CM::readCurrent (void)
{
	const float current =
		calibration_.find_current (round (filter_.output()));

	filter_.reset();
	return current;
}

/*********************************************************************/
