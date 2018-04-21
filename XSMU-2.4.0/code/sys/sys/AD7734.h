#ifndef __H__
#define __H__

#include <stdint.h>
#include "avr/SPI.h"

class AD7734
{
public:
	AD7734 (void);
	void initialize (void);
	int32_t read (uint8_t chn);

public:
	void    start (uint8_t chn);
	void    stop (void);

private:
	void activate   (void);
	void deactivate (void);

private:
	void select   (void);
	void deselect (void);

private:
	void    reset (void);
	void    writeReg8 (uint8_t reg_addr, uint8_t value);

//The readData function has been made public for access by Acquisition Class
public:
    int32_t readData (uint8_t reg_addr);

private:
	bool waitForData (void);

private:
	SPI_Configuration spiConfiguration_;
	SPI* spi;

protected:
	virtual void selectLow (void) {}
	virtual void selectHigh (void) {}

	virtual void resetLow (void) {}
	virtual void resetHigh (void) {}

	virtual bool data_ready (void) { return false; }
	virtual void initializeInterface (void) {}

	virtual void synchronize (void) {}
};

#endif
