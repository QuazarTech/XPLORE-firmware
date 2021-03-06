#include "avr/SPI.h"
#include <avr/io.h>
#include "pgmspace"

using namespace std;

#define DDR_SPI DDRB
#define DD_SS   DDB4
#define DD_MOSI DDB5
#define DD_MISO DDB6
#define DD_SCK  DDB7

/***************************************************************/

SPI& SPI::_ (void)
{
	static SPI o;
	return o;
}

SPI::SPI (void)
{
	/* Set MOSI and SCK output, all others input */
	DDR_SPI |= (1 << DD_MOSI) | (1 << DD_SCK) | (1 << DD_SS);
	DDR_SPI &= ~(1 << DD_MISO);
	reset();
}

/***************************************************************/

char SPI::read (char fill)
{
	SPDR = fill;
	while (!(SPSR & (1 << SPIF)));
	return SPDR;
}

void SPI::write (char c)
{
	SPDR = c;
	while(!(SPSR & (1 << SPIF)));
}

/***************************************************************/

void SPI::write (const void* data, uint16_t size)
{
	const uint8_t* src = reinterpret_cast<const uint8_t*> (data);

	if (size) do {

		write (*src++);

	} while (--size);
}

void SPI::write_P (const void* data, uint16_t size)
{
	const uint8_t* src = reinterpret_cast<const uint8_t*> (data);

	if (size) do {

		write (pgm_read (*src++));

	} while (--size);
}

void SPI::read (void* data, uint16_t size, char fill)
{
	uint8_t* dst = reinterpret_cast<uint8_t*> (data);

	if (size) do {

		*dst++ = read (fill);

	} while (--size);
}

void SPI::read (void* data, uint16_t size, const void* fill)
{
	uint8_t* dst = reinterpret_cast<uint8_t*> (data);
	const uint8_t* src_fill = reinterpret_cast<const uint8_t*> (fill);

	if (size) do {

		*dst++ = read (*src_fill++);

	} while (--size);
}

void SPI::read_P (void* data, uint16_t size, const void* fill)
{
	uint8_t* dst = reinterpret_cast<uint8_t*> (data);
	const uint8_t* src_fill = reinterpret_cast<const uint8_t*> (fill);

	if (size) do {

		*dst++ = read (pgm_read (*src_fill++));

	} while (--size);
}

/***************************************************************/

void SPI::reset (void)
{
	SPCR = 0;
}

void SPI::enable (SPI_Configuration configuration)
{
	SPSR; // Clears any pending interrupt flag.
	SPCR = configuration | (1 << 6);
}

/***************************************************************/

SPI_Configuration SPI::makeConfiguration (SPI_ClockFrequency freq,
										  SPI_ClockPhase phase,
										  SPI_ClockPolarity polarity,
										  SPI_Mode mode, SPI_DataOrder order)
{
	static const uint8_t freq_[] PROGMEM =
	{
		((uint8_t)0x0 << 0),
		((uint8_t)0x1 << 0),
		((uint8_t)0x2 << 0),
		((uint8_t)0x3 << 0)
	};

	static const uint8_t phase_[] PROGMEM =
	{
		((uint8_t)0x0 << 2),
		((uint8_t)0x1 << 2)
	};

	static const uint8_t polarity_[] PROGMEM =
	{
		((uint8_t)0x0 << 3),
		((uint8_t)0x1 << 3)
	};

	static const uint8_t mode_[] PROGMEM =
	{
		((uint8_t)0x0 << 4),
		((uint8_t)0x1 << 4)
	};

	static const uint8_t order_[] PROGMEM =
	{
		((uint8_t)0x0 << 5),
		((uint8_t)0x1 << 5)
	};

	return SPI_Configuration (
		pgm_read (freq_[freq]) |
		pgm_read (phase_[phase]) |
		pgm_read (polarity_[polarity]) |
		pgm_read (mode_[mode]) |
		pgm_read (order_[order]));
}
