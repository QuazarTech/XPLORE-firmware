#ifndef __PCA9557__
#define __PCA9557__

#include <stdint.h>
#include "avr/I2C.h"

class PCA9557
{
	public:
	PCA9557 (uint8_t deviceAddress);

	public:
	uint8_t readInputPortRegister (void) const;

	public:
	void writeOutputPortRegister (uint8_t data);
	uint8_t readOutputPortRegister (void) const;

	public:
	/* 0: output, 1: input */
	uint8_t getPinDirection (void) const;
	void setPinDirection (uint8_t direction);

	public:
	/* 0: normal, 1: inverted */
	uint8_t getPinPolarity (void) const;
	void setPinPolarity (uint8_t polatiry);

	public:
	const uint8_t &deviceAddress (void) const;

	private:
	uint8_t deviceAddress_;

	private:
	uint8_t completeDeviceAddress (void) const;
	I2C* i2c;
};

#endif
