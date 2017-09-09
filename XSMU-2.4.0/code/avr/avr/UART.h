#ifndef __UART__
#define __UART__

#include <utility>
#include <stdint.h>

class UART_RxBuffer
{
	public:
	UART_RxBuffer (void);

	public:
	void clear (void);
	void push_back (char data);

	public:
	bool empty (void) const;
	uint16_t size (void) const;

	public:
	const void* data (void) const {return space_;}
	const char& at (uint16_t i) const {return space_[i];}
	const char& operator[] (uint16_t i) const {return space_[i];}

	private:
	char space_[64], *writer_;
};

class UART
{
	public:
	static UART& _ (void);

	public:
	void setBaudrate (uint32_t baudrate);
	const uint32_t& baudrate (void) const {return baudrate_;}

	public:
	const UART_RxBuffer* read (void);
	void write (const void* data, uint16_t size);

	private:
	UART (void);

	private:
	uint32_t baudrate_;
	UART_RxBuffer rxBuffers_[2];
	UART_RxBuffer *activeRxBuffer_, *standbyRxBuffer_;
	uint8_t rxInterruptMaskDepth_;

	private:
	void swapRxBuffers (void);
	void maskRxInterrupt (void);
	void resumeRxInterrupt (void);
	void waitForTransmissionReady (void);
	void waitForTransmissionComplete (void);

	private:
	void isr (void);
	static void isr (void*);
};

#define uart    UART::_()

#endif
