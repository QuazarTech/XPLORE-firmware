#ifndef __QP4__
#define __QP4__

#include "inet"
#include <vector>
#include <utility>
#include <stdint.h>

enum QP4_Enumerations
{
	QP4_SOF_MARKER  =
	    (((uint32_t)('Q') << 24) |
	     ((uint32_t)('P') << 16) |
	     ((uint32_t)('4') << 8) |
	     ((uint32_t)('1') << 0)),
};

class QP4_Packet
{
	public:
	static void free (QP4_Packet *);
	static QP4_Packet *alloc (uint16_t size);

	public:
	void* body (void) {return body_;}
	const void* body (void) const {return body_;}

	public:
	uint16_t datasize (void) const {return std::ntoh (size_);}
	uint16_t size (void) const {return std::ntoh (size_) + sizeof (QP4_Packet);}

	public:
	void seal (void);

	private:
	uint32_t startFrameMarker_;
	uint16_t size_, checksum_;
	uint8_t body_[];

	private:
	QP4_Packet (uint16_t size);
};

class QP4_Transmitter
{
	public:
	void free_packet (QP4_Packet *packet);
	QP4_Packet *alloc_packet (uint16_t size);
};

enum QP4_RxState
{
	QP4_RX_STATE_IDLE,
	QP4_RX_STATE_SIZE,
	QP4_RX_STATE_CHECKSUM,
	QP4_RX_STATE_DATA
};

class QP4_Receiver
{
	public:
	QP4_Receiver (void);

	public:
	const bool& ready (void) const {return ready_;}
	std::pair<const void*, uint16_t> data (void) const {
		return std::pair<const void*, uint16_t> (data_, size_);
	}

	public:
	void clear (void);
	void push_back (uint8_t x);

	private:
	QP4_RxState state_;

	private:
	uint16_t size_;
	uint16_t expectedChecksum_, receivedChecksum_;

#if ((defined linux) || (defined WIN32))
	static const uint16_t maxAllowedDataSize_ = 1024;
	uint8_t data_[1024];
#else
	static const uint16_t maxAllowedDataSize_ = 32;
	uint8_t data_[32];
#endif

	uint8_t* dataWriter_;

	private:
	void initiateReceptionSequence (void);
	void abortReceptionSequence (void);

	private:
	void idle_cb (uint8_t x);
	void size_cb (uint8_t x);
	void checksum_cb (uint8_t x);
	void data_cb (uint8_t x);

	private:
	void setState (QP4_RxState state);
	void setStateIdle_cb (void);
	void setStateSize_cb (void);
	void setStateChecksum_cb (void);
	void setStateData_cb (void);

	private:
	bool ready_;
	uint32_t window_;
	uint16_t byteCounter_;
};

class QP4 : public QP4_Receiver, public QP4_Transmitter
{
	public:
	QP4_Receiver &receiver (void)
	{
		return static_cast<QP4_Receiver &> (*this);
	}

	QP4_Transmitter &transmitter (void)
	{
		return static_cast<QP4_Transmitter &> (*this);
	}
};

#endif
