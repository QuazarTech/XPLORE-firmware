//	1) Set interrupt from rdy pin of ADC : TODO
//	2) Select the ADC channel to read : Done in ADC_CHN
//	3) Set the ADC in streaming mode : Done in AD7734::start()

#include "app/Acquisition.h"
#include <algorithm>
#include <util/atomic.h>
#include <avr/io.h>

/************************************************************************/

// PD4 -> DIO0 -> AI1 -> CS
#define ADC_SELECT_PORT                    PORTD
#define ADC_SELECT_DDR                     DDRD
#define ADC_SELECT_BIT                     4

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

Acquisition::Queue::Queue (uint16_t capacity) :
	_capacity (capacity),
	_size (0)
{
	_values = (int32_t*) malloc (_capacity * sizeof (int32_t));
	_begin = _end = _values;
}

/************************************************************************/

void Acquisition::Queue::push_back (int32_t data)
{
	if (full()) return;
	*_end = data;
	if (++_end == _values + _capacity) _end = _values;
	++_size;
}

/************************************************************************/

void Acquisition::Queue::pop_front (void)
{
	if (empty()) return;
	int32_t ret = *_begin;
	*_begin = 0;
	if (++_begin == _values + _capacity) _begin = _values;
	--_size;
}

/************************************************************************/

void Acquisition::Queue::clear (void)
{
	while (!empty()) pop_front();
	_begin = _end = _values;
}

/************************************************************************/

Acquisition::Queue*
Acquisition::swap_queue (void)
{
	ATOMIC_BLOCK (ATOMIC_RESTORESTATE)
	{
		auto x = _active_queue.release ();
        auto y = _standby_queue.release ();
        std::swap (x, y);
        _active_queue.reset (x);
        _standby_queue.reset (y);
		_active_queue->clear();
	}

	return _standby_queue.get();
}

/************************************************************************/
/************************************************************************/

Acquisition::
Acquisition (void) :
	_active_queue (new Queue(queue_size)),
	_standby_queue (new Queue(queue_size))
{}

/************************************************************************/

void Acquisition::start (void)
{
	_active = true;
}

/************************************************************************/

void Acquisition::stop (void)
{
	_active = false;
}

/************************************************************************/

const int32_t* Acquisition::recData (void)
{
	return _standby_queue->start ();
}

/************************************************************************/

uint16_t Acquisition::recSize (void)
{
	if (_standby_queue->empty ()) swap_queue ();
	return _standby_queue->size ();
}

/************************************************************************/

void Acquisition::clearRecData (void)
{
	_standby_queue->clear();
}

/************************************************************************/

void Acquisition::clearRecData (uint16_t size)
{
	while (size-- and  !_standby_queue->empty())
		_standby_queue->pop_front();
}

/************************************************************************/
/************************************************************************/

Acquisition::AD7734_Streamer::
AD7734_Streamer (void)
{
	initialize(); //initializeInterface(), resetLow(), selectHigh()
}

void Acquisition::AD7734_Streamer::
initializeInterface (void)
{
    // Configure PORTD4 (connected to ADC CS) as output(1)
    ADC_SELECT_DDR = (ADC_SELECT_DDR & ~ADC_SELECT_MASK) | ADC_SELECT_DDR_OUT;

    // Configure PORTD5 (connected to ADC RDY) as input(0)
    ADC_DRDY_DDR = (ADC_DRDY_DDR & ~ADC_DRDY_MASK) | ADC_DRDY_DDR_IN;
}

void Acquisition::AD7734_Streamer::
selectLow (void)
{
    //Enable Chip Select for AD7734 ADC
	ADC_SELECT_PORT = (ADC_SELECT_PORT & ~ADC_SELECT_MASK) | ADC_SELECT_LOW;
}

void Acquisition::AD7734_Streamer::
selectHigh (void)
{
    //Disable Chip Select for AD7734 ADC
	ADC_SELECT_PORT = (ADC_SELECT_PORT & ~ADC_SELECT_MASK) | ADC_SELECT_HIGH;
}

void Acquisition::AD7734_Streamer::
resetLow (void)
{}

void Acquisition::AD7734_Streamer::
resetHigh (void)
{}