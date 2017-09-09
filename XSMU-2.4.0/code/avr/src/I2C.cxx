#include "avr/I2C.h"
#include "sys/hardware.h"

#include <avr/io.h>
#include <math.h>

#include "pgmspace"

using namespace std;

#define I2C_SCL_FREQ             (100e3)

#define I2C_PRESCALER_4          (0x1 << TWPS0)
#define I2C_INTERFACE_ENABLE     (0x1 << TWEN)
#define I2C_ACK_ENABLE           (0x1 << TWEA)

#define I2C_INTFLAG_MASK         (0x1 << TWINT)
#define I2C_INTFLAG_CLEAR        (0x1 << TWINT)

#define I2C_START_ASSERT         (0x1 << TWSTA)
#define I2C_STOP_ASSERT          (0x1 << TWSTO)
#define I2C_STOP_MASK            (0x1 << TWSTO)

#define I2C_STATUS_MASK          (0x1F << TWS3)

#define I2C_OPERATION_READ       (0x1)
#define I2C_OPERATION_WRITE      (0x0)

enum I2C_Mode
{
	I2C_MODE_WRITE,
	I2C_MODE_READ,
	I2C_MODE_WRITE_THEN_READ
};

I2C& I2C::_ (void)
{
	static I2C o;
	return o;
}

I2C::I2C (void)
{
	stopped_ = true;

	TWSR = I2C_PRESCALER_4;

	TWBR = (uint8_t)(0.5 +
		(cpu_clock() / I2C_SCL_FREQ - 16) / 2 / pow (4, I2C_PRESCALER_4));
}

/***************************************************************/

uint8_t I2C::status (void)
{
	return (TWSR & I2C_STATUS_MASK) >> TWS3;
}

void I2C::start (void)
{
	stopped_ = false;
	TWCR = I2C_INTFLAG_CLEAR | I2C_START_ASSERT | I2C_INTERFACE_ENABLE;
}

void I2C::check (void)
{
	typedef void (I2C::*cb_t)(void);

	static const cb_t cbs[] PROGMEM =
	{
		&I2C::null_cb,

		&I2C::start_cb,
		&I2C::repeatedStart_cb,

		&I2C::slaveAddressWriteOperationAck_cb,
		&I2C::slaveAddressWriteOperationNotAck_cb,
		&I2C::transmitDataAck_cb,
		&I2C::transmitDataNotAck_cb,

		&I2C::arbitrationLost_cb,

		&I2C::slaveAddressReadOperationAck_cb,
		&I2C::slaveAddressReadOperationNotAck_cb,
		&I2C::receiveDataAck_cb,
		&I2C::receiveDataNotAck_cb,
	};

	while (!(TWCR & I2C_INTFLAG_MASK));

	const uint8_t stat = status();

	if (stat < sizeof (cbs) / sizeof (cbs[0]))
		(this->*pgm_read(cbs[stat]))();
	else
		stop();
}

void I2C::stop (void)
{
	stopped_ = true;
	TWCR = I2C_INTFLAG_CLEAR | I2C_STOP_ASSERT | I2C_INTERFACE_ENABLE;
}

void I2C::read8 (void)
{
	*rxdata_++ = TWDR;
}

void I2C::write8 (void)
{
	TWDR = *txdata_++;
	transmitData();
}

void I2C::transmitSLA (void)
{
	TWCR = I2C_INTFLAG_CLEAR | I2C_INTERFACE_ENABLE;
}

void I2C::transmitData (void)
{
	TWCR = I2C_INTFLAG_CLEAR | I2C_INTERFACE_ENABLE;
}

void I2C::fetch8_withAck (void)
{
	TWCR =  I2C_INTFLAG_CLEAR | I2C_ACK_ENABLE | I2C_INTERFACE_ENABLE;
}

void I2C::fetch8_withoutAck (void)
{
	TWCR =  I2C_INTFLAG_CLEAR | I2C_INTERFACE_ENABLE;
}

void I2C::handleError (void)
{
	stop();
}

const bool& I2C::stopped (void) const
{
	return stopped_;
}

/***************************************************************/

void I2C::null_cb (void)
{
	handleError();
}

void I2C::start_cb (void)
{
	const uint8_t addr[] =
	{
		(uint8_t)((slaveAddress_ << 1) | I2C_OPERATION_WRITE),
		(uint8_t)((slaveAddress_ << 1) | I2C_OPERATION_READ),
		(uint8_t)((slaveAddress_ << 1) | I2C_OPERATION_WRITE)
	};

	TWDR = addr[mode_];
	transmitSLA();
}

void I2C::repeatedStart_cb (void)
{
	const uint8_t addr[] =
	{
		(uint8_t)((slaveAddress_ << 1) | I2C_OPERATION_WRITE),
		(uint8_t)((slaveAddress_ << 1) | I2C_OPERATION_READ),
		(uint8_t)((slaveAddress_ << 1) | I2C_OPERATION_READ)
	};

	TWDR = addr[mode_];
	transmitSLA();
}

/***************************************************************/

void I2C::slaveAddressWriteOperationAck_cb (void)
{
	if (txsize_) write8();
	else if (mode_ == I2C_MODE_WRITE_THEN_READ) start();
	else stop();
}

void I2C::slaveAddressWriteOperationNotAck_cb (void)
{
	handleError();
}

void I2C::transmitDataAck_cb (void)
{
	if (--txsize_) write8();
	else if (mode_ == I2C_MODE_WRITE_THEN_READ) start();
	else stop();
}

void I2C::transmitDataNotAck_cb (void)
{
	handleError();
}

void I2C::arbitrationLost_cb (void)
{
	handleError();
}

/***************************************************************/

void I2C::slaveAddressReadOperationAck_cb (void)
{
	if (rxsize_ > 1) fetch8_withAck();
	else if (rxsize_ == 1) fetch8_withoutAck();
	else stop();
}

void I2C::slaveAddressReadOperationNotAck_cb (void)
{
	handleError();
}

void I2C::receiveDataAck_cb (void)
{
	read8();
	if (--rxsize_ > 1) fetch8_withAck();
	else if (rxsize_ == 1) fetch8_withoutAck();
	else stop();
}

void I2C::receiveDataNotAck_cb (void)
{
	read8();
	--rxsize_;
	stop();
}

/***************************************************************/

uint16_t I2C::write (uint8_t slaveAddress,
					 const void* data, uint16_t size)
{
	txsize_ = size;
	slaveAddress_ = slaveAddress;
	txdata_ = reinterpret_cast<const uint8_t*> (data);
	mode_   = I2C_MODE_WRITE;

	start();
	while (!stopped())
		check();

	return txsize_;
}

uint16_t I2C::read (uint8_t slaveAddress, void* data, uint16_t size)
{
	rxsize_ = size;
	slaveAddress_ = slaveAddress;
	rxdata_ = reinterpret_cast<uint8_t*> (data);
	mode_   = I2C_MODE_READ;

	start();
	while (!stopped())
		check();

	return rxsize_;
}

uint16_t I2C::write_n_read (uint8_t slaveAddress,
							const void* txdata, uint16_t txsize,
							void* rxdata, uint16_t rxsize)
{
	rxsize_ = rxsize;
	rxdata_ = reinterpret_cast<uint8_t*> (rxdata);

	txsize_ = txsize;
	txdata_ = reinterpret_cast<const uint8_t*> (txdata);

	slaveAddress_ = slaveAddress;
	mode_   = I2C_MODE_WRITE_THEN_READ;

	start();
	while (!stopped())
		check();

	return txsize_ + rxsize_;
}

/***************************************************************/
