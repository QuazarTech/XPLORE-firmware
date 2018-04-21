#include "app/VM2.h"
#include "app/Storage.h"
#include "utils/Filter.h"
// #include "sys/hardware.h"

#include <cmath>
// #include <util/delay.h>

#include "pgmspace"

using namespace std;

#define ADC_CHN                            2

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

AD7734_VM2::AD7734_VM2 (void)
{
	initialize();
}

void AD7734_VM2::initializeInterface (void)
{
	ADC_SELECT_DDR = (ADC_SELECT_DDR & ~ADC_SELECT_MASK) | ADC_SELECT_DDR_OUT;
	ADC_DRDY_DDR   = (ADC_DRDY_DDR   & ~ADC_DRDY_MASK  ) | ADC_DRDY_DDR_IN;
// 	ADC_SYNC_DDR   = (ADC_SYNC_DDR   & ~ADC_SYNC_MASK  ) | ADC_SYNC_DDR_IN;
}

void AD7734_VM2::selectLow (void)
{
	ADC_SELECT_PORT = (ADC_SELECT_PORT & ~ADC_SELECT_MASK) | ADC_SELECT_LOW;
}

void AD7734_VM2::selectHigh (void)
{
	ADC_SELECT_PORT = (ADC_SELECT_PORT & ~ADC_SELECT_MASK) | ADC_SELECT_HIGH;
}

void AD7734_VM2::resetLow (void) {}
void AD7734_VM2::resetHigh (void) {}

bool AD7734_VM2::data_ready (void)
{
	return (ADC_DRDY_PORT & ADC_DRDY_MASK) ? false : true;
}

int32_t AD7734_VM2::read (void)
{
	return AD7734::read (ADC_CHN);
}

// void AD7734_VM2::synchronize (void)
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

/*********************************************************************/

VM2_Range toVM2_Range (uint16_t i)
{
	static const VM2_Range ranges[] PROGMEM =
	{
		VM2_RANGE_10V
	};

	return pgm_read ((i < sizeof (ranges) / sizeof (ranges[0])) ?
		ranges[i] : ranges[0]);
}

VM2::VM2 (void) : storage (Storage::get_singleton())
{
	setRange (VM2_RANGE_10V);
}

/*********************************************************************/

void VM2::setRange (VM2_Range range)
{
	range_ = range;

	if (storage->read ((uint16_t) toStorage_VM2_FileNo (range), &calibration_) !=
		sizeof (calibration_)) {

			fillDefaultCalibration (range, &calibration_);
			saveCalibration();
	}
}

/*********************************************************************/

void VM2::setCalibration (uint16_t index, float voltage)
{
	SimpleAverageFilter filter (128);

	while (!filter.full())
		filter.push_back (adc_.read());

	const int32_t adc = filter.output();
	calibration_[index].set (adc, voltage);
}

void VM2::saveCalibration (void)
{
	storage->write (toStorage_VM2_FileNo (range()), &calibration_);
}

/*********************************************************************/

void VM2::fillDefaultCalibration (void)
{
	fillDefaultCalibration (range(), &calibration_);
}

#define ADC_FS     10.0    // V
#define VM2_FS     10.0    // V

void VM2::fillDefaultCalibration (float V_FS,
								  VM_CalibrationTable* calibration)
{
	const int32_t adc_max = (int32_t) round (0x7FFFFF * (VM2_FS / ADC_FS));
	const int32_t adc_min = -adc_max;
	const float frac = 0.9;

	(*calibration)[0].set (adc_min,                          -V_FS       );
	(*calibration)[1].set ((int32_t) round (adc_min * frac), -V_FS * frac);
	(*calibration)[2].set (0,                                 0.0        );
	(*calibration)[3].set ((int32_t) round (adc_max * frac),  V_FS * frac);
	(*calibration)[4].set (adc_max,                           V_FS       );
}

void VM2::fillDefaultCalibration (VM2_Range range,
								  VM_CalibrationTable* calibration)
{
	static const float V_FS[] PROGMEM = { 10.0 };
	fillDefaultCalibration (pgm_read (V_FS[range]), calibration);
}

/*********************************************************************/

float VM2::readVoltage (uint16_t filterLength)
{
	SimpleAverageFilter filter (filterLength);

	while (!filter.full())
		filter.push_back (adc_.read());

	return calibration_.find_voltage (round (filter.output()));
}

/*********************************************************************/
