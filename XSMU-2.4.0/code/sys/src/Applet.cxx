#include "sys/Applet.h"

using namespace std;
Applet* Applet::applets_[APPLET_UPPER_BOUND] = { 0 };

/************************************************************************/

void Applet::poll (void)
{
	for (auto applet : applets_)
		if (applet) applet->check();
}

void Applet::run (void)
{
	while (1) poll();
}

/************************************************************************/

Applet::Applet (void) :
    user_data_ (0),
    cb_ (default_callback)
{
	for (auto& applet : applets_)
		if (applet == nullptr) {
			applet = this;
			break;
		}
}

Applet::~Applet (void)
{
	for (auto& applet : applets_)
		if (applet == this) {
			applet = nullptr;
			break;
		}
}

void Applet::callback (AppletCallback cb, void* user_data)
{
	cb_ = cb;
	user_data_ = user_data;
}

void Applet::do_callback (const void* oCB)
{
	(*cb_)(user_data_, oCB);
}

/************************************************************************/
