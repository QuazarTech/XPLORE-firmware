#include "sys/LCD.h"
#include "sys/hardware.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "pgmspace"

using namespace std;

#define EN          _BV(PA4)
#define RW          _BV(PA5)
#define RS          _BV(PA6)
#define LCD_PORT    PORTA

#define LCD_HEIGHT  2
#define LCD_WIDTH   16

/*********************************************/

LCD_FmtFlags::LCD_FmtFlags (void)
{
	width_ = 0;
	precision_ = 0;
	showpos_ = false;
	zerofill_ = false;
	scientific_ = false;
	left_ = false;
}

void LCD_FmtFlags::setw (uint8_t w) {width_ = w;}
void LCD_FmtFlags::setprecision (uint8_t precision) {precision_ = precision;}

void LCD_FmtFlags::showpos (void) {showpos_ = true;}
void LCD_FmtFlags::zerofill (void) {zerofill_ = true;}
void LCD_FmtFlags::scientific (void) {scientific_ = true;}
void LCD_FmtFlags::left (void) {left_ = true;}

void LCD_FmtFlags::noshowpos (void) {showpos_ = false;}
void LCD_FmtFlags::nozerofill (void) {zerofill_ = false;}
void LCD_FmtFlags::noscientific (void) {scientific_ = false;}
void LCD_FmtFlags::right (void) {left_ = false;}

/*********************************************/

LCD& LCD::_ (void)
{
	static LCD o;
	return o;
}

LCD::LCD (void)
{
	row_ = 0;
	col_ = 0;
	initialize();
	clear();
	home();
}

void LCD::initialize (void)
{
	DDRA = 0x7F;            // 7 LSB LCD OutwriteString
	writeCommand (0x03);    // 4 bit mode selection
	writeCommand (0x03);    // 4 bit mode selection
	writeCommand (0x28);    // 4 bit mode selection
	writeCommand (0x28);    // 4 bit mode selection
	writeCommand (0x08);    // cursor off
	writeCommand (0x06);    //
	writeCommand (0x0C);
}

const uint8_t& LCD::row (void) const
{
	return row_;
}

const uint8_t& LCD::col (void) const
{
	return col_;
}

void LCD::writeCommand (uint8_t cmd)
{
	LCD_PORT &= ~RS;
	LCD_PORT = ((LCD_PORT & 0xF0) | (cmd >> 4));
	LCD_PORT |= EN;
	LCD_PORT &= ~EN;
	LCD_PORT = ((LCD_PORT & 0xF0) | (cmd & 0x0F));
	LCD_PORT &= ~RS;
	LCD_PORT |= EN;
	LCD_PORT &= ~EN;
	_delay_ms(1);
}

void LCD::writeData (char c)
{
	LCD_PORT |=RS;
	LCD_PORT = ((LCD_PORT & 0xF0) | (c >> 4));
	LCD_PORT |= EN;
	LCD_PORT &= ~EN;
	LCD_PORT |= RS;
	LCD_PORT = ((LCD_PORT & 0xF0) | (c & 0x0F));
	LCD_PORT |= EN;
	LCD_PORT &= ~EN;
	_delay_ms(1);

	++col_;
}

void LCD::clear (void)
{
	writeCommand (0x01);
	home();
}

void LCD::home (void)
{
	writeCommand (0x02);
	row_ = col_ = 0;
}

void LCD::cursorAt (uint8_t row, uint8_t col)
{
	row_ = row % LCD_HEIGHT;
	col_ = col % LCD_WIDTH;

	static const uint8_t addresses[] PROGMEM = {0x80, 0xC0};
	writeCommand (pgm_read (addresses[row_]) + col_);
}

void LCD::writeChar (char c)
{
	if (c == '\n')
		cursorAt (row() + 1, 0);

	else {

		writeData (c);
		if (col() == LCD_WIDTH)
			cursorAt (row() + 1, 0);
	}
}

void LCD::writeString (const char *s)
{
	char c;
	while ((c = *s++))
		writeChar (c);
}

/***** Format flags *****/

const LCD_FmtFlags& LCD::fmtflags (void) const
{
	return fmtflags_;
}

void LCD::fmtflags (const LCD_FmtFlags& flags)
{
	fmtflags_ = flags;
}

/************** operator << definitions ******************/

LCD& LCD::operator<< (char c)
{
	char fmt[16], *fmt_ = fmt, s[32];

	fmt_ += sprintf (fmt_, "%%");
	if (fmtflags_.left_) fmt_ += sprintf (fmt_, "-");
	if (fmtflags_.width_) fmt_ += sprintf (fmt_, "%hhu", fmtflags_.width_);
	fmt_ += sprintf (fmt_, "c");

	sprintf (s, fmt, c);
	writeString (s);
	return *this;
}

LCD& LCD::operator<< (const char* str)
{
	char fmt[16], *fmt_ = fmt, s[32];

	fmt_ += sprintf (fmt_, "%%");
	if (fmtflags_.left_) fmt_ += sprintf (fmt_, "-");
	if (fmtflags_.width_) fmt_ += sprintf (fmt_, "%hhu", fmtflags_.width_);
	fmt_ += sprintf (fmt_, "s");

	sprintf (s, fmt, str);
	writeString (s);
	return *this;
}

LCD& LCD::operator<< (int16_t i16)
{
	char fmt[16], *fmt_ = fmt, s[32];

	fmt_ += sprintf (fmt_, "%%");
	if (fmtflags_.left_) fmt_ += sprintf (fmt_, "-");
	if (fmtflags_.showpos_) fmt_ += sprintf (fmt_, "+");
	if (fmtflags_.zerofill_) fmt_ += sprintf (fmt_, "0");
	if (fmtflags_.width_) fmt_ += sprintf (fmt_, "%hhu", fmtflags_.width_);
	fmt_ += sprintf (fmt_, "hd");

	sprintf (s, fmt, i16);
	writeString (s);
	return *this;
}

LCD& LCD::operator<< (int32_t i32)
{
	char fmt[16], *fmt_ = fmt, s[32];

	fmt_ += sprintf (fmt_, "%%");
	if (fmtflags_.left_) fmt_ += sprintf (fmt_, "-");
	if (fmtflags_.showpos_) fmt_ += sprintf (fmt_, "+");
	if (fmtflags_.zerofill_) fmt_ += sprintf (fmt_, "0");
	if (fmtflags_.width_) fmt_ += sprintf (fmt_, "%hhu", fmtflags_.width_);
	fmt_ += sprintf (fmt_, "ld");

	sprintf (s, fmt, i32);
	writeString (s);
	return *this;
}

LCD& LCD::operator<< (int64_t i64)
{
	char fmt[16], *fmt_ = fmt, s[32];

	fmt_ += sprintf (fmt_, "%%");
	if (fmtflags_.left_) fmt_ += sprintf (fmt_, "-");
	if (fmtflags_.showpos_) fmt_ += sprintf (fmt_, "+");
	if (fmtflags_.zerofill_) fmt_ += sprintf (fmt_, "0");
	if (fmtflags_.width_) fmt_ += sprintf (fmt_, "%hhu", fmtflags_.width_);
	fmt_ += sprintf (fmt_, "lld");

	sprintf (s, fmt, i64);
	writeString (s);
	return *this;
}

LCD& LCD::operator<< (uint16_t u16)
{
	char fmt[16], *fmt_ = fmt, s[32];

	fmt_ += sprintf (fmt_, "%%");
	if (fmtflags_.left_) fmt_ += sprintf (fmt_, "-");
	if (fmtflags_.showpos_) fmt_ += sprintf (fmt_, "+");
	if (fmtflags_.zerofill_) fmt_ += sprintf (fmt_, "0");
	if (fmtflags_.width_) fmt_ += sprintf (fmt_, "%hhu", fmtflags_.width_);
	fmt_ += sprintf (fmt_, "hu");

	sprintf (s, fmt, u16);
	writeString (s);
	return *this;
}

LCD& LCD::operator<< (uint32_t u32)
{
	char fmt[16], *fmt_ = fmt, s[32];

	fmt_ += sprintf (fmt_, "%%");
	if (fmtflags_.left_) fmt_ += sprintf (fmt_, "-");
	if (fmtflags_.showpos_) fmt_ += sprintf (fmt_, "+");
	if (fmtflags_.zerofill_) fmt_ += sprintf (fmt_, "0");
	if (fmtflags_.width_) fmt_ += sprintf (fmt_, "%hhu", fmtflags_.width_);
	fmt_ += sprintf (fmt_, "lu");

	sprintf (s, fmt, u32);
	writeString (s);
	return *this;
}

LCD& LCD::operator<< (uint64_t u64)
{
	char fmt[16], *fmt_ = fmt, s[32];

	fmt_ += sprintf (fmt_, "%%");
	if (fmtflags_.left_) fmt_ += sprintf (fmt_, "-");
	if (fmtflags_.showpos_) fmt_ += sprintf (fmt_, "+");
	if (fmtflags_.zerofill_) fmt_ += sprintf (fmt_, "0");
	if (fmtflags_.width_) fmt_ += sprintf (fmt_, "%hhu", fmtflags_.width_);
	fmt_ += sprintf (fmt_, "llu");

	sprintf (s, fmt, u64);
	writeString (s);
	return *this;
}

LCD& LCD::operator<< (float f4)
{
	char fmt[16], *fmt_ = fmt, s[32];

	fmt_ += sprintf (fmt_, "%%");
	if (fmtflags_.left_) fmt_ += sprintf (fmt_, "-");
	if (fmtflags_.showpos_) fmt_ += sprintf (fmt_, "+");
	if (fmtflags_.zerofill_) fmt_ += sprintf (fmt_, "0");
	if (fmtflags_.width_) fmt_ += sprintf (fmt_, "%hhu", fmtflags_.width_);
	if (fmtflags_.precision_)
			fmt_ += sprintf (fmt_, ".%hhu", fmtflags_.precision_);
	fmt_ += sprintf (fmt_, "%c", (fmtflags_.scientific_ ? 'e' : 'f'));

	sprintf (s, fmt, (double)f4);
	writeString (s);
	return *this;
}

LCD& LCD::operator<< (double f8)
{
	char fmt[16], *fmt_ = fmt, s[32];

	fmt_ += sprintf (fmt_, "%%");
	if (fmtflags_.left_) fmt_ += sprintf (fmt_, "-");
	if (fmtflags_.showpos_) fmt_ += sprintf (fmt_, "+");
	if (fmtflags_.zerofill_) fmt_ += sprintf (fmt_, "0");
	if (fmtflags_.width_) fmt_ += sprintf (fmt_, "%hhu", fmtflags_.width_);
	if (fmtflags_.precision_)
		fmt_ += sprintf (fmt_, ".%hhu", fmtflags_.precision_);
	fmt_ += sprintf (fmt_, "%c", (fmtflags_.scientific_ ? 'e' : 'f'));

	sprintf (s, fmt, f8);
	writeString (s);
	return *this;
}

LCD& LCD::operator<< (LCD& (*pf)(LCD&))
{
	return (*pf)(*this);
}

/*********** Switch functions ******************/

LCD& showpos (LCD& o)
{
	LCD_FmtFlags flags = o.fmtflags();
	flags.showpos();
	o.fmtflags (flags);
	return o;
}

LCD& zerofill (LCD& o)
{
	LCD_FmtFlags flags = o.fmtflags();
	flags.zerofill();
	o.fmtflags (flags);
	return o;
}

LCD& scientific (LCD& o)
{
	LCD_FmtFlags flags = o.fmtflags();
	flags.scientific();
	o.fmtflags (flags);
	return o;
}

LCD& noshowpos (LCD& o)
{
	LCD_FmtFlags flags = o.fmtflags();
	flags.noshowpos();
	o.fmtflags (flags);
	return o;
}

LCD& nozerofill (LCD& o)
{
	LCD_FmtFlags flags = o.fmtflags();
	flags.nozerofill();
	o.fmtflags (flags);
	return o;
}

LCD& noscientific (LCD& o)
{
	LCD_FmtFlags flags = o.fmtflags();
	flags.noscientific();
	o.fmtflags (flags);
	return o;
}

LCD& left (LCD& o)
{
	LCD_FmtFlags flags = o.fmtflags();
	flags.left();
	o.fmtflags (flags);
	return o;
}

LCD& right (LCD& o)
{
	LCD_FmtFlags flags = o.fmtflags();
	flags.right();
	o.fmtflags (flags);
	return o;
}

/*********** Manipulators ******************/

LCD_SetW setw (uint8_t w)
{
	LCD_SetW o = { w };
	return o;
}

LCD_SetPrecision setprecision (uint8_t precision)
{
	LCD_SetPrecision o  = { precision };
	return o;
}

LCD& operator<< (LCD& str, const LCD_SetW& o)
{
	LCD_FmtFlags flags = str.fmtflags();
	flags.setw (o.w);
	str.fmtflags (flags);
	return str;
}

LCD& operator<< (LCD& str, const LCD_SetPrecision& o)
{
	LCD_FmtFlags flags = str.fmtflags();
	flags.setprecision (o.precision);
	str.fmtflags (flags);
	return str;
}

/*****************************************************************/
