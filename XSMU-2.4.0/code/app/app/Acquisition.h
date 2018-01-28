#ifndef __ACQUISITION__
#define __ACQUISITION__

#include <new>
#include <cstdlib>
#include <stdint.h>
#include <stddef.h>
#include <memory>

#include "sys/AD7734.h"
#include "sys/Applet.h"

/************************************************************************/
/************************************************************************/

class Acquisition : public Applet
{
public:
    virtual void check (void);

public:
	static constexpr uint16_t queue_size = 64;
	class Queue;
	class AD7734_Streamer;
	Acquisition (void);

public:
	void start (void);
	void stop (void);
	Queue* swap_queue (void);

public:
	uint16_t recSize (void);
	const int32_t* recData (void);
	void clearRecData (void);
	void clearRecData (uint16_t size);

private:
	std::unique_ptr<Queue> _active_queue;
	std::unique_ptr<Queue> _standby_queue;
	bool _queue_overrun;
	bool _active;

private:
	AD7734_Streamer* _adc;
};

/************************************************************************/

class Acquisition::Queue
{
public:
	Queue (uint16_t capacity);
	~Queue (void) { free (_values); }

public:
	void push_back (int32_t o);
	void pop_front (void);
	bool full  (void) const { return (_size == _capacity); }
	bool empty (void) const { return (_size == 0); }

public:
	int32_t front (void) { return *_begin; }
	const int32_t* start (void) const { return _begin; }

public:
	void clear (void);
	uint16_t size (void) const { return _size; }

private:
	uint16_t _capacity;
	int32_t *_values, *_begin, *_end;
	uint16_t _size;
};

/************************************************************************/
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