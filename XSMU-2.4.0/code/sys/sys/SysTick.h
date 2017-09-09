#ifndef __SYS_TICK__
#define __SYS_TICK__

#include <stdint.h>

class SysTick
{
public:
	typedef uint32_t tick_t;

public:
	static SysTick& _ (void);

public:
	tick_t get (void) const;
	double interval (void) const { return interval_; }

public:
	tick_t time_to_tick (double time) const;
	double tick_to_time (tick_t tick) const;

private:
	SysTick (void);
	double interval_;
};

#define systick    SysTick::_()

#endif
