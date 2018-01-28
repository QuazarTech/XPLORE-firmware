#ifndef __AD5542__
#define __AD5542__

#include "avr/SPI.h"

class AD5542
{
public:
	AD5542 (void);

public:
	void initialize (void);

public:
	void write (uint16_t value);
	uint16_t read (void) const { return value_; }

protected:
	virtual void selectLow (void) {}
	virtual void selectHigh (void) {}
	virtual void initializeInterface (void) {}

private:
	uint16_t value_;

private:
	SPI_Configuration spiConfig_;
	SPI *spi;
};

#endif
