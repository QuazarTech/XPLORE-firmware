#include "app/LEDDisplay.h"
#include <avr/io.h>

#define VM_BIT                     0
#define VM_DDR                     DDRB
#define VM_PORT                    PORTB
#define VM_MASK                   (1 << VM_BIT)
#define VM_DDR_OUT                (1 << VM_BIT)
#define VM_LOW                    (0 << VM_BIT)
#define VM_HIGH                   (1 << VM_BIT)

#define CS_BIT                     1
#define CS_DDR                     DDRB
#define CS_PORT                    PORTB
#define CS_MASK                   (1 << CS_BIT)
#define CS_DDR_OUT                (1 << CS_BIT)
#define CS_LOW                    (0 << CS_BIT)
#define CS_HIGH                   (1 << CS_BIT)

#define VS_BIT                     2
#define VS_DDR                     DDRB
#define VS_PORT                    PORTB
#define VS_MASK                   (1 << VS_BIT)
#define VS_DDR_OUT                (1 << VS_BIT)
#define VS_LOW                    (0 << VS_BIT)
#define VS_HIGH                   (1 << VS_BIT)

#define MASK                      (VM_MASK | CS_MASK | VS_MASK)
#define PORT                       PORTB

LED_Display& LED_Display::_ (void)
{
	static LED_Display o;
	return o;
}

LED_Display::LED_Display (void)
{
	VM_DDR = (VM_DDR & ~VM_MASK) | VM_DDR_OUT;
	VM_PORT = (VM_PORT & ~VM_MASK) | VM_HIGH;

	CS_DDR = (CS_DDR & ~CS_MASK) | CS_DDR_OUT;
	CS_PORT = (CS_PORT & ~CS_MASK) | CS_HIGH;

	VS_DDR = (VS_DDR & ~VS_MASK) | VS_DDR_OUT;
	VS_PORT = (VS_PORT & ~VS_MASK) | VS_HIGH;
}

/*****************************************************************/

void LED_Display::CS_activate (void)
{
	CS_PORT = (CS_PORT & ~CS_MASK) | CS_LOW;
}

void LED_Display::CS_deactivate (void)
{
	CS_PORT = (CS_PORT & ~CS_MASK) | CS_HIGH;
}

/*****************************************************************/

void LED_Display::VS_activate (void)
{
	VS_PORT = (VS_PORT & ~VS_MASK) | VS_LOW;
}

void LED_Display::VS_deactivate (void)
{
	VS_PORT = (VS_PORT & ~VS_MASK) | VS_HIGH;
}

/*****************************************************************/

void LED_Display::VM_toggle (void)
{
	VM_PORT = (VM_PORT & ~VM_MASK) |
		((VM_PORT & VM_MASK) ? VM_LOW : VM_HIGH);
}

void LED_Display::VM_activate (void)
{
	VM_PORT = (VM_PORT & ~VM_MASK) | VM_LOW;
}

void LED_Display::VM_deactivate (void)
{
	VM_PORT = (VM_PORT & ~VM_MASK) | VM_HIGH;
}

/*****************************************************************/
