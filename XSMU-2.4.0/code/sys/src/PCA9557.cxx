#include "sys/PCA9557.h"
#include "avr/I2C.h"

PCA9557::PCA9557 (uint8_t deviceAddress) :
    deviceAddress_ (deviceAddress)
{}

const uint8_t &PCA9557::deviceAddress (void) const
{
	return deviceAddress_;
}

uint8_t PCA9557::completeDeviceAddress (void) const
{
	return 0x18 | (deviceAddress() & 0x07);
}

/*************************************************************/

uint8_t PCA9557::readInputPortRegister (void) const
{
	uint8_t data;
	const uint8_t controlValue = 0x00;

	i2c.write_n_read (completeDeviceAddress(),
					  &controlValue, sizeof (controlValue),
					  &data, sizeof (data));

	return data;
}

void PCA9557::writeOutputPortRegister (uint8_t portval)
{
	const uint8_t data[] = {0x01, portval};
	i2c.write (completeDeviceAddress(), data, sizeof (data));
}

uint8_t PCA9557::readOutputPortRegister (void) const
{
	uint8_t data;
	const uint8_t controlValue = 0x01;

	i2c.write_n_read (completeDeviceAddress(),
					  &controlValue, sizeof (controlValue),
					  &data, sizeof (data));

	return data;
}

/*************************************************************/

uint8_t PCA9557::getPinPolarity (void) const
{
	uint8_t polarity;
	const uint8_t controlValue = 0x02;

	i2c.write_n_read (completeDeviceAddress(),
					  &controlValue, sizeof (controlValue),
					  &polarity, sizeof (polarity));

	return polarity;
}

void PCA9557::setPinPolarity (uint8_t polarity)
{
	const uint8_t data[] = {0x02, polarity};
	i2c.write (completeDeviceAddress(), data, sizeof (data));
}

/*************************************************************/

uint8_t PCA9557::getPinDirection (void) const
{
	uint8_t direction;
	const uint8_t controlValue = 0x03;

	i2c.write_n_read (completeDeviceAddress(),
					  &controlValue, sizeof (controlValue),
					  &direction, sizeof (direction));

	return direction;
}

void PCA9557::setPinDirection (uint8_t direction)
{
	const uint8_t data[] = {0x03, direction};
	i2c.write (completeDeviceAddress(), data, sizeof (data));
}

/*************************************************************/
