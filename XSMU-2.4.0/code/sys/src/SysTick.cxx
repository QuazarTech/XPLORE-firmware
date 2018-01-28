#include "sys/SysTick.h"
#include "sys/hardware.h"

#include <avr/io.h>
#include <util/atomic.h>
#include <math.h>

static SysTick::tick_t timer_tick = 0;

#define TIMER_CLOCK_PRESCALER         1024

#if defined __AVR_ATmega32__
/*************************************************************************/

#define TCCR_MODE_CTC                 0x08
#define TCCR_NORMAL_PORT_OPERATION    0x00
#define TCCR_PRESCALER_1024           0x05

#define TIMSK_ENABLE_CTC_INTERRUPT   (1 << OCIE0)

static double initialize_timer (double target_interval)
{
	const double  f   = cpu_clock() / TIMER_CLOCK_PRESCALER;
	const uint8_t top = (uint8_t) round (target_interval * f);
	const double  obtained_interval = top / f;

	TCNT0 = 0;
	OCR0  = top;

	TIMSK |= TIMSK_ENABLE_CTC_INTERRUPT;

	TCCR0 = TCCR_MODE_CTC |
			TCCR_NORMAL_PORT_OPERATION |
			TCCR_PRESCALER_1024;

	return obtained_interval;
}

ISR (TIMER0_COMP_vect)
{
	++timer_tick;
}

#elif defined __AVR_ATmega644__
/********************************************************************/

#define TCCR0A_NORMAL_PORT_OPERATION_OCR0A ((0 << COM0A1) | (0 << COM0A0))
#define TCCR0A_NORMAL_PORT_OPERATION_OCR0B ((0 << COM0B1) | (0 << COM0B0))

#define TCCR0A_MODE_CTC         ((1 << WGM01) | (0 << WGM00))
#define TCCR0B_MODE_CTC         (0 << WGM02)

#define TCCR0B_PRESCALER_1024   ((1 << CS02) | (0 << CS01) | (1 << CS00))

#define TIMSK0_ENABLE_CTC_INTERRUPT (1 << OCIE0A)

static double initialize_timer (double target_interval)
{
	const double  f   = cpu_clock() / TIMER_CLOCK_PRESCALER;
	const uint8_t top = (uint8_t) round (target_interval * f);
	const double  obtained_interval = top / f;

	TCNT0 = 0;
	OCR0A  = top;

	TIMSK0 |= TIMSK0_ENABLE_CTC_INTERRUPT;

	TCCR0A = TCCR0A_NORMAL_PORT_OPERATION_OCR0A |
			 TCCR0A_NORMAL_PORT_OPERATION_OCR0B |
			 TCCR0A_MODE_CTC;

	TCCR0B = TCCR0B_MODE_CTC |
			 TCCR0B_PRESCALER_1024;

	return obtained_interval;
}

ISR (TIMER0_COMPA_vect)
{
	++timer_tick;
}

#endif
/********************************************************************/
/********************************************************************/

static SysTick::tick_t get_timer (void)
{
	SysTick::tick_t rv;

	ATOMIC_BLOCK (ATOMIC_RESTORESTATE)
	{
		rv = timer_tick;
	}

	return rv;
}

/********************************************************************/
/********************************************************************/

SysTick* SysTick::get_singleton (void)
{
	static auto o = new SysTick;
	return o;
}

/********************************************************************/

SysTick::SysTick (void)
{
	interval_ = initialize_timer (10e-3);
}

/********************************************************************/

SysTick::tick_t SysTick::get (void) const
{
	return get_timer();
}

/********************************************************************/
/********************************************************************/

SysTick::tick_t SysTick::time_to_tick (double time) const
{
	return (tick_t) round (time / interval());
}

/********************************************************************/

double SysTick::tick_to_time (SysTick::tick_t tick) const
{
	return tick * interval();
}

/********************************************************************/
/********************************************************************/
