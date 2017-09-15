#include "avr/UART.h"
#include "sys/hardware.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include <algorithm>

#ifdef __AVR_ATmega644__

#define UCSRA             UCSR0A
#define UCSRB             UCSR0B
#define UCSRC             UCSR0C
#define UDR               UDR0
#define UBRRH             UBRR0H
#define UBRRL             UBRR0L
#define USART_RXC_vect    USART0_RX_vect

#endif

static void defaultCallback (void *);
typedef void (*Callback)(void*);

static void *user_data_ = 0;
static Callback cb_ = defaultCallback;

void defaultCallback (void *)
{}

ISR (USART_RXC_vect)
{
	(*cb_)(user_data_);
}

void setCallback (Callback cb, void* user_data)
{
	cb_ = cb;
	user_data_ = user_data;
}

/***************************************************************/

UART_RxBuffer::UART_RxBuffer (void)
{
	writer_ = space_;
}

void UART_RxBuffer::clear (void)
{
	writer_ = space_;
}

void UART_RxBuffer::push_back (char data)
{
	*writer_++ = data;
}

bool UART_RxBuffer::empty (void) const
{
	return (writer_ == space_);
}

uint16_t UART_RxBuffer::size (void) const
{
	return (uint16_t)(writer_ - space_);
}

/***************************************************************/

// UCSRA
#define UART_TRANSMIT_COMPLETE_MASK                    (0x1 << 6)
#define UART_TRANSMIT_COMPLETE_CLEAR                   (0x1 << 6)
#define UART_TRANSMIT_READY_MASK                       (0x1 << 5)
#define UART_NO_DOUBLE_TRANSMISSION_SPEED              (0x0 << 1)
#define UART_NO_MULTI_PROCESSOR_COMMUNICATION_MODE     (0x0 << 0)

// UCSRB
#define UART_RX_COMPLETE_INTERRUPT_MASK                (0x1 << 7)
#define UART_RX_COMPLETE_INTERRUPT_ENABLE              (0x1 << 7)
#define UART_RX_COMPLETE_INTERRUPT_DISABLE             (0x0 << 7)
#define UART_TX_COMPLETE_INTERRUPT_DISABLE             (0x0 << 6)
#define UART_DATA_REGISTER_EMPTY_INTERRUPT_DISABLE     (0x0 << 5)
#define UART_RECEIVER_ENABLE                           (0x1 << 4)
#define UART_TRANSMITTER_ENABLE                        (0x1 << 3)

// UCSRC
#if defined __AVR_ATmega644__
#define UART_UCSRC_ACCESS                              (0x0 << 7)
#elif defined __AVR_ATmega32__
#define UART_UCSRC_ACCESS                              (0x1 << 7)
#endif

#define UART_ASYNC_OPERATION                           (0x0 << 6)
#define UART_PARITY_DISABLE                            (0x0 << 4)
#define UART_STOP_BIT_1                                (0x0 << 3)
#define UART_CHAR_SIZE_8                               (0x3 << 1)

#define UART_UBRRH_ACCESS                              (0x0 << 7)

/***************************************************************/

UART& UART::_ (void)
{
	static UART o;
	return o;
}

UART::UART (void)
{
	activeRxBuffer_ = &rxBuffers_[0];
	standbyRxBuffer_ = &rxBuffers_[1];
	rxInterruptMaskDepth_ = 0;

	UCSRA = UART_NO_DOUBLE_TRANSMISSION_SPEED |
		UART_NO_MULTI_PROCESSOR_COMMUNICATION_MODE;

	UCSRB =
		UART_RX_COMPLETE_INTERRUPT_ENABLE |
		UART_TX_COMPLETE_INTERRUPT_DISABLE |
		UART_DATA_REGISTER_EMPTY_INTERRUPT_DISABLE |
		UART_RECEIVER_ENABLE |
		UART_TRANSMITTER_ENABLE;

	UCSRC =
		UART_UCSRC_ACCESS |
		UART_ASYNC_OPERATION |
		UART_PARITY_DISABLE |
		UART_STOP_BIT_1 |
		UART_CHAR_SIZE_8;

	setBaudrate (9600);
	setCallback (isr, this);
}

void UART::isr (void* user_data)
{
	reinterpret_cast<UART*> (user_data)->isr();
}

void UART::isr (void)
{
	activeRxBuffer_->push_back (UDR);
}

void UART::maskRxInterrupt (void)
{
	if (rxInterruptMaskDepth_++ == 0)
		UCSRB = (UCSRB & ~UART_RX_COMPLETE_INTERRUPT_MASK) |
			UART_RX_COMPLETE_INTERRUPT_DISABLE;
}

void UART::resumeRxInterrupt (void)
{
	if (rxInterruptMaskDepth_ && (--rxInterruptMaskDepth_ == 0))
		UCSRB = (UCSRB & ~UART_RX_COMPLETE_INTERRUPT_MASK) |
			UART_RX_COMPLETE_INTERRUPT_ENABLE;
}

void UART::swapRxBuffers (void)
{
	standbyRxBuffer_->clear();

	maskRxInterrupt();
	std::swap (activeRxBuffer_, standbyRxBuffer_);
	resumeRxInterrupt();
}

void UART::setBaudrate (uint32_t bd)
{
	switch (bd)
	{
		case 9600: return _setBaudrate (9600);

		case 19200: return _setBaudrate (19200);

		case 38400: return _setBaudrate (38400);

		case 57600: return _setBaudrate (57600);

		case 115200: return _setBaudrate (115200);
	}
}

void UART::_setBaudrate (uint32_t bd)
{
	baudrate_ = bd;

	const uint16_t baudReg12 =
		(uint16_t)(cpu_clock() / 16 / baudrate() - 1) & 0x0FFF;

	const uint8_t baudRegMSB = (uint8_t)(baudReg12 >> 8);
	const uint8_t baudRegLSB = (uint8_t)(baudReg12);

	UBRRH = baudRegMSB | UART_UBRRH_ACCESS;
	UBRRL = baudRegLSB;
}

const UART_RxBuffer* UART::read (void)
{
	swapRxBuffers();
	return standbyRxBuffer_;
}

void UART::write (const void* data, uint16_t size)
{
	const uint8_t* src = reinterpret_cast<const uint8_t*> (data);

	if (size) {

		do {

			waitForTransmissionReady();
			UDR = *src++;

		} while (--size);

		waitForTransmissionComplete();
	}
}

void UART::waitForTransmissionReady (void)
{
	while (!(UCSRA & UART_TRANSMIT_READY_MASK));
}

void UART::waitForTransmissionComplete (void)
{
	while (!(UCSRA & UART_TRANSMIT_COMPLETE_MASK));

	UCSRA =
		(UCSRA & ~UART_TRANSMIT_COMPLETE_MASK) |
		UART_TRANSMIT_COMPLETE_CLEAR;
}

/***************************************************************/

