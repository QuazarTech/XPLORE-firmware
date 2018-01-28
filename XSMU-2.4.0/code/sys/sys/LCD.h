#ifndef __LCD__
#define __LCD__

#include <stdint.h>

class LCD_FmtFlags
{
	friend class LCD;

	public:
	LCD_FmtFlags (void);

	public:
	void setw (uint8_t w);
	void setprecision (uint8_t precision);

	void showpos (void);
	void zerofill (void);
	void scientific (void);
	void left (void);

	void noshowpos (void);
	void nozerofill (void);
	void noscientific (void);
	void right (void);

	private:
	uint8_t width_, precision_;
	bool showpos_, zerofill_, scientific_, left_;
};

class LCD
{
	public:
	static LCD* get_singleton (void); // Singleton declaration

	public:
	void home (void);
	void clear (void);
	void cursorAt (uint8_t row, uint8_t col);

	public:
	LCD& operator<< (char c);
	LCD& operator<< (const char* s);

	LCD& operator<< (int16_t i16);
	LCD& operator<< (int32_t i32);
	LCD& operator<< (int64_t i64);

	LCD& operator<< (uint16_t u16);
	LCD& operator<< (uint32_t u32);
	LCD& operator<< (uint64_t u64);

	LCD& operator<< (float f4);
	LCD& operator<< (double f8);

	LCD& operator<< (LCD& (*pf)(LCD&));

	public:
	const LCD_FmtFlags& fmtflags (void) const;
	void fmtflags (const LCD_FmtFlags& flags);

	private:
	void writeData (char c);
	void writeCommand (uint8_t cmd);

	private:
	void writeChar (char c);
	void writeString (const char *s);

	private:
	const uint8_t& row (void) const;
	const uint8_t& col (void) const;

	private:
	LCD (void);
	void initialize (void);

	private:
	uint8_t row_, col_;

	private:
	LCD_FmtFlags fmtflags_;
};

/*********** Switch functions ******************/

LCD& showpos (LCD& o);
LCD& zerofill (LCD& o);
LCD& scientific (LCD& o);
LCD& left (LCD& o);

LCD& noshowpos (LCD& o);
LCD& nozerofill (LCD& o);
LCD& noscientific (LCD& o);
LCD& right (LCD& o);

/*********** Manipulators ******************/

struct LCD_SetW
{
	uint8_t w;
};

struct LCD_SetPrecision
{
	uint8_t precision;
};

LCD_SetW setw (uint8_t w);
LCD_SetPrecision setprecision (uint8_t precision);

LCD& operator<< (LCD& str, const LCD_SetW& o);
LCD& operator<< (LCD& str, const LCD_SetPrecision& o);

/********************************************************************/

#endif
