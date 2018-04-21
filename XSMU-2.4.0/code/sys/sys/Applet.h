#ifndef __APPLET__
#define __APPLET__

#include <vector>
#include <stdint.h>

enum Applet_Enumerations
{
	APPLET_UPPER_BOUND = 4
};

typedef void (*AppletCallback)(void* user_data, const void* oCB);

class Applet
{
	public:
	Applet (void);
	virtual ~Applet (void);

	public:
	void do_callback (const void* app_data);
	void callback (AppletCallback cb, void* user_data);

	public:
	static void run (void);
	static void poll (void);

	public:
	virtual void check (void) {};

	private:
	static Applet* applets_[APPLET_UPPER_BOUND];

	private:
	void* user_data_;
	AppletCallback cb_;
	static void default_callback (void*, const void*) {}
};

#endif
