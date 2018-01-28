#ifndef __SYSTEM_CONFIG__
#define __SYSTEM_CONFIG__

#include <stdint.h>

class Storage;
class SystemConfig
{
public:
	static SystemConfig* get_singleton (void);

public:
	void hwBoardNo  (int16_t no) { hwBoardNo_  = no; }
	void hwBomNo    (int16_t no) { hwBomNo_    = no; }
	void hwBugfixNo (int16_t no) { hwBugfixNo_ = no; }

public:
	int16_t hwBoardNo  (void) const { return hwBoardNo_;  }
	int16_t hwBomNo    (void) const { return hwBomNo_;    }
	int16_t hwBugfixNo (void) const { return hwBugfixNo_; }

public:
	void read (void);
	void write (void);
	void fillDefault (void);

private:
	int16_t hwBoardNo_;
	int16_t hwBomNo_;

	int16_t hwBugfixNo_;
	int16_t reserved3_;

	int16_t reserved4_;
	int16_t reserved5_;

	int16_t reserved6_;
	int16_t reserved7_;

	int16_t reserved8_;
	int16_t reserved9_;

	int16_t reserved10_;
	int16_t reserved11_;

	int16_t reserved12_;
	int16_t reserved13_;

	int16_t reserved14_;
	int16_t reserved15_;

private:
	Storage* storage;
private:
	SystemConfig (void);
};

#endif
