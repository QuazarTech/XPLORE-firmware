#include <algorithm>
#include <util/atomic.h>
#include <avr/io.h>

#include "app/Acquisition.h"

/************************************************************************/

#define AD7734_DATA_REGISTER               0x08

#define ADC_CHN                            1

// PD4 -> DIO0 -> AI1 -> CS
#define ADC_SELECT_BIT                     4
#define ADC_SELECT_DDR                     DDRD
#define ADC_SELECT_PORT                    PORTD
#define ADC_SELECT_MASK                   (1 << ADC_SELECT_BIT)
#define ADC_SELECT_DDR_OUT                (1 << ADC_SELECT_BIT)
#define ADC_SELECT_LOW                    (0 << ADC_SELECT_BIT)
#define ADC_SELECT_HIGH                   (1 << ADC_SELECT_BIT)

// PD5 <- DIO1 <- AO1 <- RDY
#define ADC_DRDY_BIT                       5
#define ADC_DRDY_DDR                       DDRD
#define ADC_DRDY_PORT                      PIND
#define ADC_DRDY_MASK                     (1 << ADC_DRDY_BIT)
#define ADC_DRDY_DDR_IN                   (0 << ADC_DRDY_BIT)

/************************************************************************/
/************************************************************************/

Acquisition::Queue::Queue (void) :
_overrun (false)
{
	//	1) Set interrupt from rdy pin of ADC : TODO
	//	2) Select the ADC channel to read : Done in ADC_CHN
	//	3) Set the ADC in streaming mode : Done in AD7734::start()
}

/************************************************************************/

void
Acquisition::Queue::push_back (int32_t data)
{
	if (full()) _overrun = true;
	else base_type::push_back (data);
}

/************************************************************************/

void
Acquisition::Queue::clear (void)
{
	base_type::clear();
	_overrun = false;
}

/************************************************************************/
/************************************************************************/

Acquisition::
Acquisition (void) :
		_active_queue (new Queue),
		_standby_queue (new Queue)
	{}

/************************************************************************/

void Acquisition::data_ready_handler (void)
{
	_active_queue->push_back (_adc->readData (AD7734_DATA_REGISTER | ADC_CHN));
}

/************************************************************************/

void Acquisition::start (void)
{
	_adc->start(ADC_CHN);
}

/************************************************************************/

void Acquisition::stop (void)
{
	_adc->stop();
}

/************************************************************************/

Acquisition::Queue*
Acquisition::swap_queue (void)
{
	ATOMIC_BLOCK (ATOMIC_RESTORESTATE)
	{
		std::swap (_active_queue, _standby_queue);
		_active_queue->clear();
	}

	return _standby_queue;
}

/************************************************************************/
/************************************************************************/

Acquisition::AD7734_Streamer::
AD7734_Streamer (void)
{
	initialize();
}

void Acquisition::AD7734_Streamer::
initializeInterface (void)
{
	ADC_SELECT_DDR = (ADC_SELECT_DDR & ~ADC_SELECT_MASK) | ADC_SELECT_DDR_OUT;
	ADC_DRDY_DDR   = (ADC_DRDY_DDR   & ~ADC_DRDY_MASK  ) | ADC_DRDY_DDR_IN;
// 	ADC_SYNC_DDR   = (ADC_SYNC_DDR   & ~ADC_SYNC_MASK  ) | ADC_SYNC_DDR_IN;
}

void Acquisition::AD7734_Streamer::
selectLow (void)
{
	ADC_SELECT_PORT = (ADC_SELECT_PORT & ~ADC_SELECT_MASK) | ADC_SELECT_LOW;
}

void Acquisition::AD7734_Streamer::
selectHigh (void)
{
	ADC_SELECT_PORT = (ADC_SELECT_PORT & ~ADC_SELECT_MASK) | ADC_SELECT_HIGH;
}

void Acquisition::AD7734_Streamer::
resetLow (void)
{}

void Acquisition::AD7734_Streamer::
resetHigh (void)
{}

/************************************************************************/
/************************************************************************/