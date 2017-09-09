#include "app/VM.h"
#include "app/Storage.h"
// #include "sys/hardware.h"

#include <cmath>
#include <avr/io.h>
// #include <util/delay.h>

#include "pgmspace"

using namespace std;

#define IOX_ADDR                           4

#define IOX_RANGE_MASK                    (0x3F << 0)
#define IOX_RANGE_DIR                     (0x00 << 0)
#define IOX_RANGE_POLARITY                (0x00 << 0)

#define IOX_RANGE_1mV                     (0x08 << 0)
#define IOX_RANGE_10mV                    (0x04 << 0)
#define IOX_RANGE_100mV                   (0x02 << 0)
#define IOX_RANGE_1V                      (0x01 << 0)
#define IOX_RANGE_10V                     (0x00 << 0)
#define IOX_RANGE_100V                    (0x30 << 0)

#define IOX_INPUT_SRC_MASK                (0x3 << 6)
#define IOX_INPUT_SRC_DIR                 (0x0 << 6)
#define IOX_INPUT_SRC_POLARITY            (0x0 << 6)

#define IOX_INPUT_SRC_GROUND              (0x1 << 6)
#define IOX_INPUT_SRC_VREF                (0x2 << 6)
#define IOX_INPUT_SRC_SIGNAL              (0x0 << 6)

#define IOX_MASK (IOX_RANGE_MASK | IOX_INPUT_SRC_MASK)
#define IOX_DIR  (IOX_RANGE_DIR  | IOX_INPUT_SRC_DIR)
#define IOX_POLARITY (IOX_RANGE_POLARITY | IOX_INPUT_SRC_POLARITY)

#define IOX2_ADDR                           0

#define IOX2_VM_TERMINAL_MASK                (0x1 << 6)
#define IOX2_VM_TERMINAL_DIR                 (0x0 << 6)
#define IOX2_VM_TERMINAL_POLARITY            (0x0 << 6)

#define IOX2_MASK     (IOX2_VM_TERMINAL_MASK)
#define IOX2_DIR      (IOX2_VM_TERMINAL_DIR)
#define IOX2_POLARITY (IOX2_VM_TERMINAL_POLARITY)

#define ADC_CHN                            1

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

AD7734_VM::AD7734_VM (void)
{
	initialize();
}

void AD7734_VM::initializeInterface (void)
{
	ADC_SELECT_DDR = (ADC_SELECT_DDR & ~ADC_SELECT_MASK) | ADC_SELECT_DDR_OUT;
	ADC_DRDY_DDR   = (ADC_DRDY_DDR   & ~ADC_DRDY_MASK  ) | ADC_DRDY_DDR_IN;
// 	ADC_SYNC_DDR   = (ADC_SYNC_DDR   & ~ADC_SYNC_MASK  ) | ADC_SYNC_DDR_IN;
}

void AD7734_VM::selectLow (void)
{
	ADC_SELECT_PORT = (ADC_SELECT_PORT & ~ADC_SELECT_MASK) | ADC_SELECT_LOW;
}

void AD7734_VM::selectHigh (void)
{
	ADC_SELECT_PORT = (ADC_SELECT_PORT & ~ADC_SELECT_MASK) | ADC_SELECT_HIGH;
}

void AD7734_VM::resetLow (void)
{}

void AD7734_VM::resetHigh (void)
{}

bool AD7734_VM::data_ready (void)
{
	return (ADC_DRDY_PORT & ADC_DRDY_MASK) ? false : true;
}

// void AD7734_VM::synchronize (void)
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

int32_t AD7734_VM::read (void)
{
	return AD7734::read (ADC_CHN);
}

/*********************************************************************/

VM_Range toVM_Range (uint16_t i)
{
	static const VM_Range ranges[] PROGMEM =
	{
		VM_RANGE_1mV,
		VM_RANGE_10mV,
		VM_RANGE_100mV,
		VM_RANGE_1V,
		VM_RANGE_10V,
		VM_RANGE_100V
	};

	return pgm_read ((i < sizeof (ranges) / sizeof (ranges[0])) ?
		ranges[i] : ranges[0]);
}

VM_Terminal toVM_Terminal (uint16_t i)
{
	static const VM_Terminal ranges[] PROGMEM =
	{
		VM_TERMINAL_MEASUREMENT,
		VM_TERMINAL_SOURCE,
	};

	return pgm_read ((i < sizeof (ranges) / sizeof (ranges[0])) ?
		ranges[i] : ranges[0]);
}

/*********************************************************************/

VM& VM::_ (void)
{
	static VM o;
	return o;
}

VM::VM (void) :
	iox_ (IOX_ADDR),
	filter_ (16),
	iox2_ (IOX2_ADDR)
{
	iox_.setPinDirection (
		(iox_.getPinDirection() & ~IOX_MASK) | IOX_DIR);

	iox_.setPinPolarity  (
		(iox_.getPinPolarity() & ~IOX_MASK) | IOX_POLARITY);

	iox2_.setPinDirection (
		(iox2_.getPinDirection() & ~IOX_MASK) | IOX_DIR);

	iox_.setPinPolarity  (
		(iox_.getPinPolarity() & ~IOX_MASK) | IOX_POLARITY);

	setRange (VM_RANGE_10V);
	activateExternalMeasurement();
	setTerminal (VM_TERMINAL_SOURCE);
}

/*********************************************************************/

void VM::setRange (VM_Range range)
{
	range_ = range;

	if (storage.read (toStorage_VM_FileNo (range), &calibration_) !=
		sizeof (calibration_)) {

			fillDefaultCalibration (range, &calibration_);
			saveCalibration();
	}

	static const uint8_t iox_data[] PROGMEM =
	{
		IOX_RANGE_1mV, IOX_RANGE_10mV, IOX_RANGE_100mV,
		IOX_RANGE_1V,  IOX_RANGE_10V,  IOX_RANGE_100V
	};

	iox_.writeOutputPortRegister (
		(iox_.readOutputPortRegister() & ~IOX_RANGE_MASK) |
		pgm_read (iox_data[range]));
}

/*********************************************************************/

void VM::activateTare (void)
{
	iox_.writeOutputPortRegister (
		(iox_.readOutputPortRegister() & ~IOX_INPUT_SRC_MASK) |
			IOX_INPUT_SRC_GROUND);
}

void VM::activateInternalCalibration (void)
{
	iox_.writeOutputPortRegister (
		(iox_.readOutputPortRegister() & ~IOX_INPUT_SRC_MASK) |
			IOX_INPUT_SRC_VREF);
}

void VM::activateExternalMeasurement (void)
{
	iox_.writeOutputPortRegister (
		(iox_.readOutputPortRegister() & ~IOX_INPUT_SRC_MASK) |
			IOX_INPUT_SRC_SIGNAL);
}

/*********************************************************************/

float VM::readVoltage (uint16_t filterLength)
{
	MedianFilter filter ((filterLength > 256) ? 256 : filterLength);

	while (!filter.full())
		filter.push_back (adc_.read());

	return calibration_.find_voltage (round (filter.output()));
}

/*********************************************************************/

void VM::fillDefaultCalibration (void)
{
	fillDefaultCalibration (range(), &calibration_);
}

#define ADC_FS     10.0 // V
#define VM_FS      10.0 // V

void VM::fillDefaultCalibration (float V_FS,
								 VM_CalibrationTable* calibration)
{
	const int32_t adc_max = (int32_t) round (0x7FFFFF * (VM_FS / ADC_FS));
	const int32_t adc_min = -adc_max;
	const float frac = 0.9;

	(*calibration)[0].set (adc_min,                          -V_FS       );
	(*calibration)[1].set ((int32_t) round (adc_min * frac), -V_FS * frac);
	(*calibration)[2].set (0,                                 0.0        );
	(*calibration)[3].set ((int32_t) round (adc_max * frac),  V_FS * frac);
	(*calibration)[4].set (adc_max,                           V_FS       );
}

void VM::fillDefaultCalibration (VM_Range range,
								 VM_CalibrationTable* calibration)
{
	static const float V_FS[] PROGMEM =
	{
		1e-3, 10e-3, 100e-3, 1.0, 10.0, 100.0
	};

	fillDefaultCalibration (pgm_read (V_FS[range]), calibration);
}

/*********************************************************************/

void VM::setCalibration (uint16_t index, float voltage)
{
	SimpleAverageFilter filter (128);

	while (!filter.full())
		filter.push_back (adc_.read());

	const int32_t adc = filter.output();
	calibration_[index].set (adc, voltage);
}

void VM::saveCalibration (void)
{
	storage.write (toStorage_VM_FileNo (range()), &calibration_);
}

/*********************************************************************/
/*********************************************************************/

void VM::check (void)
{
	filter_.push_back (adc_.read());
}

float VM::readVoltage (void)
{
	const float voltage =
		calibration_.find_voltage (round (filter_.output()));

	filter_.reset();
	return voltage;
}

/*********************************************************************/
/*********************************************************************/

void VM::setTerminal (VM_Terminal terminal)
{
	iox2_.writeOutputPortRegister (
		(iox2_.readOutputPortRegister() & ~IOX2_VM_TERMINAL_MASK) |
			(uint16_t) terminal << 6);
	terminal_ = terminal;
}

/*********************************************************************

void VM::setTerminal (VM_Terminal terminal)
{
	iox_.writeOutputPortRegister (
		(iox_.readOutputPortRegister() & 0b11101111) |
			(uint16_t) terminal << 4);
}

/*********************************************************************/
