#include "sys/AD5542.h"
#include "inet"

using namespace std;

AD5542::AD5542 (void) :
	value_ (0),
	spiConfig_ (SPI::makeConfiguration (
		SPI_CLOCK_MCLK_BY_128, SPI_LEADING_EDGE_SAMPLING,
		SPI_IDLE_CLOCK_HIGH, SPI_MODE_MASTER, SPI_MSB_FIRST))
{}

void AD5542::initialize (void)
{
	initializeInterface();
	selectHigh();
}

void AD5542::write (uint16_t value)
{
	value_ = value;

	spi.enable (spiConfig_);
	selectLow();

	value = hton (value);
	spi.write (&value, sizeof (value));

	selectHigh();
	spi.reset();
}
