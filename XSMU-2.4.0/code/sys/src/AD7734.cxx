#include "avr/SPI.h"
#include "sys/AD7734.h"
#include "inet"
#include "sys/hardware.h"
#include <util/delay.h>
#include "utils/Filter.h"
#include "pgmspace"

using namespace std;

/**** Register addresses ****/
#define AD7734_COMMUNICATION_REGISTER      0x00
#define AD7734_IO_PORT_REGISTER            0x01
#define AD7734_DATA_REGISTER               0x08
#define AD7734_CHANNEL_STATUS_REGISTER     0x20
#define AD7734_CHANNEL_SETUP_REGISTER      0x28
#define AD7734_CONVERSION_TIME_REGISTER    0x30
#define AD7734_MODE_REGISTER               0x38

/**** AD7734_COMMUNICATION_REGISTER ****/
#define AD7734_REGISTER_ADDRESS_MASK       (0x3F << 0)
#define AD7734_OPERATION_READ              (1 << 6)
#define AD7734_OPERATION_WRITE             (0 << 6)

/**** AD7734_IO_PORT_REGISTER ****/
#define AD7734_RDYFN_MASK                  (0x1 << 3)
#define AD7734_RDYFN                       (0x1 << 3)

/**** AD7734_CHANNEL_STATUS_REGISTER ****/
#define AD7734_CHANNEL_MASK                (0x3 << 5)

/**** AD7734_CHANNEL_SETUP_REGISTER ****/
#define AD7734_CHANNEL_ENABLE_MASK         (0x1 << 3)
#define AD7734_CHANNEL_ENABLE              (0x1 << 3)
#define AD7734_CHANNEL_NOT_ENABLE          (0x0 << 3)
#define AD7734_RANGE_MASK                  (0x3 << 0)
#define AD7734_BIPOLAR_10V_RANGE           (0x0 << 3)

/**** AD7734_CONVERSION_TIME_REGISTER ****/
#define AD7734_SPEED_372Hz_WITH_CHOP       (0x80 | 0x7F)
#define AD7734_SPEED_500Hz_WITH_CHOP       (0x80 | 0x60)

/**** AD7734_MODE_REGISTER ****/
#define AD7734_MODE_MASK                   (0x7 << 5)
#define AD7734_MODE_IDLE                   (0x0 << 5)
#define AD7734_CONTINUOUS_CONVERSION_MODE  (0x1 << 5)
#define AD7734_SINGLE_CONVERSION_MODE      (0x2 << 5)
#define AD7734_RESOLUTION_MASK             (0x1 << 1)
#define AD7734_RESOLUTION_24BIT            (0x1 << 1)
#define AD7734_CLAMP_MASK                  (0x1 << 0)
#define AD7734_CLAMP_ENABLE                (0x1 << 0)
#define AD7734_CLAMP_DISABLE               (0x0 << 0)
#define AD7734_CONTINUOUS_READ_MASK        (0x1 << 2)
#define AD7734_CONTINOUS_READ_ENABLE       (0x1 << 2)
#define AD7734_DUMP_MASK                   (0x1 << 3)
#define AD7734_DUMP_MODE                   (0x1 << 3)

/**** AD7734 CHANNEL STATUS REGISTER ****/
#define AD7734_STATUS_SIGN_OVR_MASK        (0x3 << 0)

/**********************************************************************/

AD7734::AD7734 (void) : spi (SPI::get_singleton())
{
	spiConfiguration_ =
	SPI::makeConfiguration (SPI_CLOCK_MCLK_BY_128,
							SPI_TRAILING_EDGE_SAMPLING,
						 SPI_IDLE_CLOCK_HIGH,
						 SPI_MODE_MASTER,
						 SPI_MSB_FIRST);
}

/**********************************************************************/

void AD7734::activate (void)
{
	resetHigh();
	reset();
}

void AD7734::deactivate (void)
{
	resetLow();
}

void AD7734::select (void)
{
	spi->enable (spiConfiguration_);
	selectLow();
}

void AD7734::deselect (void)
{
	selectHigh();
	spi->reset();
}

void AD7734::initialize (void)
{
	initializeInterface();
	resetLow();
	selectHigh();
}

/**********************************************************************/

void AD7734::writeReg8 (uint8_t reg_addr, uint8_t value)
{
	const uint8_t data[] =
	{
		(uint8_t)(reg_addr | AD7734_OPERATION_WRITE), value
	};

	select();
	spi->write (data, sizeof (data));
	deselect();
}

int32_t AD7734::readData (uint8_t reg_addr)
{
	select();

	const uint8_t data = reg_addr | AD7734_OPERATION_READ;
	spi->write (&data, sizeof (data));

	uint8_t status;
	spi->read (&status, sizeof (status), (char)0);

	union
	{
		int32_t i32;
		uint8_t  u8[4];
	} ua32;

	static const uint8_t over_ranges[] PROGMEM =
	{
		0,    // SIGN = 0, OVR = 0
		1,    // SIGN = 0, OVR = 1
		0,    // SIGN = 1, OVR = 0
		0xFF, // SIGN = 1, OVR = 1
	};

	ua32.u8[0] = pgm_read (
		over_ranges[status & AD7734_STATUS_SIGN_OVR_MASK]);

	spi->read (ua32.u8 + 1, 3, (char)0);

	deselect();

	return ntoh (ua32.i32);
}

void AD7734::reset (void)
{
	static const uint8_t resetSequence[] PROGMEM =
	{
		0x00, 0xFF, 0xFF, 0xFF, 0xFF
	};

	select();
	spi->write_P (resetSequence, sizeof (resetSequence));
	deselect();
}

/**********************************************************************/

// bool AD7734::waitForData (void)
// {
// 	for (uint8_t ms = 5; ms; --ms, _delay_ms (1))
// 		if (data_ready())
// 			return true;
//
// 	return false;
// }

bool AD7734::waitForData (void)
{
	for (uint16_t loop = 10000; loop; --loop, _delay_us (1))
		if (data_ready())
			return true;

	return false;
}

// int32_t AD7734::read (uint8_t chn)
// {
// 	synchronize();
// 	activate();
//
// 	// Configures 10V input
// 	writeReg8 (AD7734_CHANNEL_SETUP_REGISTER | chn,
// 			   AD7734_CHANNEL_ENABLE | AD7734_BIPOLAR_10V_RANGE);
//
// 	// Configures digital filters
// 	writeReg8 (AD7734_CONVERSION_TIME_REGISTER | chn,
// 			   AD7734_SPEED_372Hz_WITH_CHOP);
//
// 	// Initiates a single 24-bit conversion.
// 	writeReg8 (AD7734_MODE_REGISTER | chn,
// 			   AD7734_SINGLE_CONVERSION_MODE | AD7734_DUMP_MODE |
// 			   AD7734_RESOLUTION_24BIT | AD7734_CLAMP_DISABLE);
//
// 	// Waits and reads 24-bit data.
// 	const int32_t adc24 = waitForData() ?
// 		readData (AD7734_DATA_REGISTER | chn) : 0x800000;
//
// 	deactivate();
//
// 	return adc24 - 0x800000;
// }

int32_t AD7734::read (uint8_t chn)
{
	synchronize();
	activate();

	// Configures 10V input
	writeReg8 (AD7734_CHANNEL_SETUP_REGISTER | chn,
			   AD7734_CHANNEL_ENABLE | AD7734_BIPOLAR_10V_RANGE);

	// Configures digital filters
	writeReg8 (AD7734_CONVERSION_TIME_REGISTER | chn,
			   AD7734_SPEED_500Hz_WITH_CHOP);

	// Initiates a single 24-bit conversion.
	writeReg8 (AD7734_MODE_REGISTER | chn,
			   AD7734_CONTINUOUS_CONVERSION_MODE | AD7734_DUMP_MODE |
			   AD7734_RESOLUTION_24BIT | AD7734_CLAMP_DISABLE);

	// 2ms (i.e. 500Hz) * 10 = 20ms (i.e. 50Hz)
	SimpleAverageFilter filter (10);

	while (!filter.full()) {

		// Waits for data
		if (!waitForData()) {

			filter.reset();
			break;
		}

		// Reads 24-bit data and pushes it in the filter
		filter.push_back (readData (AD7734_DATA_REGISTER | chn));
	}

	deactivate();

	const int32_t adc24 = filter.empty() ? 0x800000 :
		((int32_t) round (filter.output()));

	return adc24 - 0x800000;
}

/**********************************************************************/

void AD7734::start (uint8_t chn)
{
	synchronize();
	activate();

	// Configures 10V input
	writeReg8 (AD7734_CHANNEL_SETUP_REGISTER | chn,
			   AD7734_CHANNEL_ENABLE | AD7734_BIPOLAR_10V_RANGE);

	// Configures digital filters
	writeReg8 (AD7734_CONVERSION_TIME_REGISTER | chn,
			   AD7734_SPEED_500Hz_WITH_CHOP);

	// Initiates continuous 24-bit conversion.
	writeReg8 (AD7734_MODE_REGISTER | chn,
			   AD7734_CONTINUOUS_CONVERSION_MODE | AD7734_DUMP_MODE |
			   AD7734_RESOLUTION_24BIT | AD7734_CLAMP_DISABLE);
}

/************************************************************************/

void AD7734::stop (void)
{
	deactivate();
}

/************************************************************************/
/************************************************************************/