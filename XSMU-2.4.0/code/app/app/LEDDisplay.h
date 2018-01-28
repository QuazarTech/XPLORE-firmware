#ifndef __LED_DISPLAY__
#define __LED_DISPLAY__

#include <stdint.h>

class LED_Display
{
public:
	static LED_Display* get_singleton (void);

public:
	void CS_activate (void);
	void CS_deactivate (void);

public:
	void VS_activate (void);
	void VS_deactivate (void);

public:
	void VM_toggle (void);
	void VM_activate (void);
	void VM_deactivate (void);

private:
	LED_Display (void);
};

#endif
