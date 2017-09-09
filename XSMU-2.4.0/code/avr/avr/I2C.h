#ifndef __I2C__
#define __I2C__

#include <stdint.h>

class I2C
{
	public:
	static I2C& _ (void);

	public:
	uint16_t read (uint8_t slaveAddress, void* data, uint16_t size);
	uint16_t write (uint8_t slaveAddress, const void* data, uint16_t size);
	uint16_t write_n_read (uint8_t slaveAddress,
						   const void* txdata, uint16_t txsize,
						   void* rxdata, uint16_t rxsize);

	private:
	I2C (void);
	uint8_t status (void);
	void start (void);
	void check (void);
	void stop (void);
	void read8 (void);
	void write8 (void);
	void transmitSLA (void);
	void transmitData (void);
	void fetch8_withAck (void);
	void fetch8_withoutAck (void);
	void handleError (void);
	const bool& stopped (void) const;

	private: // Callbacks
	void null_cb (void);

	void start_cb (void);
	void repeatedStart_cb (void);

	void slaveAddressWriteOperationAck_cb (void);
	void slaveAddressWriteOperationNotAck_cb (void);
	void transmitDataAck_cb (void);
	void transmitDataNotAck_cb (void);
	void arbitrationLost_cb (void);

	void slaveAddressReadOperationAck_cb (void);
	void slaveAddressReadOperationNotAck_cb (void);
	void receiveDataAck_cb (void);
	void receiveDataNotAck_cb (void);

	private:
	bool stopped_;
	uint8_t* rxdata_;
	const uint8_t* txdata_;
	uint16_t txsize_, rxsize_;
	uint8_t slaveAddress_, mode_;
};

#define i2c    I2C::_()

#endif
