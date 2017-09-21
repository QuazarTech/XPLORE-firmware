#include "sys/QP4.h"

#include <new>
#include <stdlib.h>

#include "pgmspace"

using namespace std;

QP4_Packet::QP4_Packet (uint16_t size) :
    startFrameMarker_ (hton (QP4_SOF_MARKER)),
    size_ (hton (size)),
    checksum_ (hton ((uint16_t)((~0) + 1)))
{}

void QP4_Packet::seal (void)
{
	uint16_t checksum = 0;
	const uint8_t* begin = body_;
	const uint8_t* end = begin + datasize();

	for (const uint8_t* it = begin; it != end; ++it)
		checksum += *it;

	checksum = ~checksum + 1;
	checksum_ = hton (checksum);
}

/***********************************************************************/

QP4_Packet* QP4_Transmitter::alloc_packet (uint16_t size)
{
	auto packet = _packet;

	if (size > _capacity)
		packet = static_cast<QP4_Packet*> (realloc (
			_packet, sizeof (QP4_Packet) + (_capacity = size)));

	return packet ? (new (_packet = packet) QP4_Packet (size)) : nullptr;
}

void QP4_Transmitter::free_packet (QP4_Packet *packet)
{}

/***********************************************************/

QP4_Receiver::QP4_Receiver (void) :
	state_            (QP4_RX_STATE_IDLE),
	size_             (0),
	expectedChecksum_ ((~0) + 1),
	receivedChecksum_ (0),
	dataWriter_       (data_),
	ready_            (false),
	window_           (0),
	byteCounter_      (0)
{}

void QP4_Receiver::clear (void)
{
	state_            = QP4_RX_STATE_IDLE;
	size_             = 0;
	expectedChecksum_ = ((~0) + 1);
	receivedChecksum_ = 0;
	dataWriter_       = data_;
	ready_            = false;
	window_           = 0;
	byteCounter_      = 0;
}

/***********************************************************/

void QP4_Receiver::push_back (uint8_t x)
{
	typedef void (QP4_Receiver::*Callback)(uint8_t x);

	Callback callbacks[] =
	{
		&QP4_Receiver::idle_cb,
		&QP4_Receiver::size_cb,
		&QP4_Receiver::checksum_cb,
		&QP4_Receiver::data_cb,
	};

	window_ = (window_ << 8) | (uint32_t)(x);
	if (window_ == QP4_SOF_MARKER) {

		initiateReceptionSequence();
		return;
	}

	(this->*callbacks[state_])(x);
}

void QP4_Receiver::setState (QP4_RxState state)
{
	typedef void (QP4_Receiver::*cb_t)(void);

	static const cb_t cbs[] PROGMEM = {

		&QP4_Receiver::setStateIdle_cb,
		&QP4_Receiver::setStateSize_cb,
		&QP4_Receiver::setStateChecksum_cb,
		&QP4_Receiver::setStateData_cb
	};

	state_ = state;
	(this->*pgm_read(cbs[state_]))();
}

void QP4_Receiver::setStateIdle_cb (void)
{}

void QP4_Receiver::setStateSize_cb (void)
{
	byteCounter_ = sizeof (size_);
}

void QP4_Receiver::setStateChecksum_cb (void)
{
	byteCounter_ = sizeof (receivedChecksum_);
}

void QP4_Receiver::setStateData_cb (void)
{
	byteCounter_ = size_;
}

/***********************************************************/

void QP4_Receiver::initiateReceptionSequence (void)
{
	clear();
	setState (QP4_RX_STATE_SIZE);
}

void QP4_Receiver::abortReceptionSequence (void)
{
	clear();
}

/***********************************************************/

void QP4_Receiver::idle_cb (uint8_t x)
{}

void QP4_Receiver::size_cb (uint8_t x)
{
	size_ = (size_ << 8) | x;

	if (--byteCounter_ == 0) {

		if (size_ > maxAllowedDataSize_)
			abortReceptionSequence();
		else
			setState (QP4_RX_STATE_CHECKSUM);
	}
}

void QP4_Receiver::checksum_cb (uint8_t x)
{
	receivedChecksum_ = (receivedChecksum_ << 8) | x;

	if (--byteCounter_ == 0) {

		if (size_)
			setState (QP4_RX_STATE_DATA);

		else {

			if (receivedChecksum_ == expectedChecksum_)
				ready_ = true;

			setState (QP4_RX_STATE_IDLE);
		}
	}
}

void QP4_Receiver::data_cb (uint8_t x)
{
	*dataWriter_++ = x;
	expectedChecksum_ += x;

	if (--byteCounter_ == 0) {

		expectedChecksum_ = ~expectedChecksum_ + 1;

		if (expectedChecksum_ == receivedChecksum_)
			ready_ = true;

		setState (QP4_RX_STATE_IDLE);
	}
}

/***********************************************************/
