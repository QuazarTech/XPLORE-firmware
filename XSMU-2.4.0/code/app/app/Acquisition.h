#ifndef __ACQUISITION__
#define __ACQUISITION__

#include <deque>

#include "sys/AD7734.h"

/************************************************************************/
/************************************************************************/

class Acquisition
{

public:
	static constexpr uint16_t queue_size = 64;
	class Queue;
	class AD7734_Streamer;
	Acquisition (void);

public:
	void data_ready_handler (void);

public:
	void start (void);
	void stop (void);
	Queue* swap_queue (void);

private:
	Queue* _active_queue;
	Queue* _standby_queue;
	bool   _queue_overrun;

private:
	AD7734_Streamer* _adc;
};

/************************************************************************/

class Acquisition::Queue : public std::deque<int32_t, Acquisition::queue_size>
{
public:
	Queue (void);
public:
	typedef std::deque<int32_t, Acquisition::queue_size> base_type;

public:
	bool overrun (void) const { return _overrun; }

public:
	void push_back (int32_t data);
	void clear (void);

private:
	bool _overrun;
};

/************************************************************************/

class Acquisition::AD7734_Streamer : public AD7734
{
public:
	AD7734_Streamer (void);

protected:
	virtual void selectLow (void);
	virtual void selectHigh (void);

	virtual void resetLow (void);
	virtual void resetHigh (void);

	virtual void initializeInterface (void);

};

/************************************************************************/
/************************************************************************/

#endif