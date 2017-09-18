#ifndef __COMM__
#define __COMM__

#include "sys/QP4.h"
#include "sys/Applet.h"

#include "inet"
#include <vector>
#include <stdint.h>

/******************************************************************/

enum Comm_Opcode
{
	COMM_OPCODE_NOP,                               //00
	COMM_OPCODE_IDN,                               //01
	COMM_OPCODE_KEEP_ALIVE,                        //02
	COMM_OPCODE_SET_SOURCE_MODE,                   //03
	COMM_OPCODE_CS_SET_RANGE,                      //04
	COMM_OPCODE_CS_GET_CALIBRATION,                //05
	COMM_OPCODE_CS_VERIFY_CALIBRATION,             //06
	COMM_OPCODE_CS_SET_CALIBRATION,                //07
	COMM_OPCODE_CS_SAVE_CALIBRATION,               //08
	COMM_OPCODE_CS_SET_CURRENT,                    //09

	COMM_OPCODE_VS_SET_RANGE,                      //10
	COMM_OPCODE_VS_GET_CALIBRATION,                //11
	COMM_OPCODE_VS_VERIFY_CALIBRATION,             //12
	COMM_OPCODE_VS_SET_CALIBRATION,                //13
	COMM_OPCODE_VS_SAVE_CALIBRATION,               //14
	COMM_OPCODE_VS_SET_VOLTAGE,                    //15
    COMM_OPCODE_CM_SET_RANGE,                      //16
	COMM_OPCODE_CM_GET_CALIBRATION,                //17
	COMM_OPCODE_CM_SET_CALIBRATION,                //18
	COMM_OPCODE_CM_SAVE_CALIBRATION,               //19

	COMM_OPCODE_CM_READ,                           //20
	COMM_OPCODE_VM_SET_RANGE,                      //21
	COMM_OPCODE_VM_GET_CALIBRATION,                //22
	COMM_OPCODE_VM_SET_CALIBRATION,                //23
	COMM_OPCODE_VM_SAVE_CALIBRATION,               //24
	COMM_OPCODE_VM_READ,                           //25
	COMM_OPCODE_CS_LOAD_DEFAULT_CALIBRATION,       //26
	COMM_OPCODE_VS_LOAD_DEFAULT_CALIBRATION,       //27
	COMM_OPCODE_CM_LOAD_DEFAULT_CALIBRATION,       //28
	COMM_OPCODE_VM_LOAD_DEFAULT_CALIBRATION,       //29

	COMM_OPCODE_RM_READ_AUTOSCALE,                 //30
	COMM_OPCODE_SYSTEM_CONFIG_GET,                 //31
	COMM_OPCODE_SYSTEM_CONFIG_SET,                 //32
	COMM_OPCODE_SYSTEM_CONFIG_SAVE,                //33
	COMM_OPCODE_SYSTEM_CONFIG_LOAD_DEFAULT,        //34
	COMM_OPCODE_VM2_SET_RANGE,                     //35
	COMM_OPCODE_VM2_GET_CALIBRATION,               //36
	COMM_OPCODE_VM2_SET_CALIBRATION,               //37
	COMM_OPCODE_VM2_SAVE_CALIBRATION,              //38
	COMM_OPCODE_VM2_READ,                          //39

	COMM_OPCODE_VM2_LOAD_DEFAULT_CALIBRATION,      //40
	COMM_OPCODE_VM_SET_TERMINAL,                   //41
	COMM_OPCODE_VM_GET_TERMINAL,                   //42

	COMM_OPCODE_CHANGE_BAUD,                       //43
	COMM_OPCODE_REC_SIZE,                          //44
	COMM_OPCODE_REC_DATA,                          //45

};

enum Comm_SourceMode
{
	COMM_SOURCE_MODE_CURRENT,
	COMM_SOURCE_MODE_VOLTAGE
};

enum Comm_CS_Range
{
	COMM_CS_RANGE_10uA,
	COMM_CS_RANGE_100uA,
	COMM_CS_RANGE_1mA,
	COMM_CS_RANGE_10mA,
	COMM_CS_RANGE_100mA,
};

enum Comm_VS_Range
{
	COMM_VS_RANGE_10V,
	COMM_VS_RANGE_100V
};

enum Comm_CM_Range
{
	COMM_CM_RANGE_10uA,
	COMM_CM_RANGE_100uA,
	COMM_CM_RANGE_1mA,
	COMM_CM_RANGE_10mA,
	COMM_CM_RANGE_100mA
};

enum Comm_VM_Range
{
	COMM_VM_RANGE_1mV,
	COMM_VM_RANGE_10mV,
	COMM_VM_RANGE_100mV,
	COMM_VM_RANGE_1V,
	COMM_VM_RANGE_10V,
	COMM_VM_RANGE_100V
};

enum Comm_VM2_Range
{
	COMM_VM2_RANGE_10V,
};

enum Comm_VM_Terminal
{
	Comm_VM_Terminal_Measurement,
	Comm_VM_Terminal_Source,
};

Comm_SourceMode   toComm_SourceMode  (uint16_t i);
Comm_CS_Range     toComm_CS_Range    (uint16_t i);
Comm_VS_Range     toComm_VS_Range    (uint16_t i);
Comm_CM_Range     toComm_CM_Range    (uint16_t i);
Comm_VM_Range     toComm_VM_Range    (uint16_t i);
Comm_VM2_Range    toComm_VM2_Range   (uint16_t i);
Comm_VM_Terminal  toComm_VM_Terminal (uint16_t i);

/******************************************************************/

class CommPacket
{
protected:
	CommPacket (uint16_t opcode) :
		opcode_ (std::hton (opcode)),
		reserve_ (0)
	{}

public:
	uint16_t opcode (void) const {return std::ntoh (opcode_);}

private:
	uint16_t opcode_;
	uint16_t reserve_;
};

/******************************************************************/

class CommPacket_Identity : public CommPacket
{
protected:
	CommPacket_Identity (void) :
		CommPacket (COMM_OPCODE_IDN)
	{}
};

class CommRequest_Identity : public CommPacket_Identity
{
private:
	CommRequest_Identity (void);
};

class CommResponse_Identity : public CommPacket_Identity
{
public:
	CommResponse_Identity (const char* identity,
						   uint32_t hardware_version,
						   uint32_t firmware_version);

private:
	char identity_[32];
	uint32_t hardware_version_;
	uint32_t firmware_version_;
};

/******************************************************************/

class CommPacket_nop : public CommPacket
{
protected:
	CommPacket_nop (void) :
		CommPacket (COMM_OPCODE_NOP)
	{}
};

class CommRequest_nop : public CommPacket_nop
{
private:
	CommRequest_nop (void);
};

class CommResponse_nop : public CommPacket_nop
{
public:
	CommResponse_nop (void)
	{}
};

/******************************************************************/

class CommPacket_keepAlive : public CommPacket
{
protected:
	CommPacket_keepAlive (void) :
		CommPacket (COMM_OPCODE_KEEP_ALIVE)
	{}
};

class CommRequest_keepAlive : public CommPacket_keepAlive
{
private:
	CommRequest_keepAlive (void);

public:
	uint32_t leaseTime_ms (void) const {return std::ntoh (leaseTime_ms_);}

private:
	uint32_t leaseTime_ms_;
};

class CommResponse_keepAlive : public CommPacket_keepAlive
{
public:
	CommResponse_keepAlive (uint32_t leaseTime_ms) :
		leaseTime_ms_ (std::hton (leaseTime_ms))
	{}

private:
	uint32_t leaseTime_ms_;
};

/******************************************************************/

class CommPacket_SetSourceMode : public CommPacket
{
protected:
	CommPacket_SetSourceMode (void) :
		CommPacket (COMM_OPCODE_SET_SOURCE_MODE)
	{}
};

class CommRequest_SetSourceMode : public CommPacket_SetSourceMode
{
private:
	CommRequest_SetSourceMode (void);

public:
	Comm_SourceMode mode (void) const {
		return toComm_SourceMode (std::ntoh (mode_));
	}

private:
	uint16_t mode_;
	uint16_t reserve_;
};

class CommResponse_SetSourceMode : public CommPacket_SetSourceMode
{
public:
	CommResponse_SetSourceMode (Comm_SourceMode mode) :
		mode_ (std::hton ((uint16_t)(mode))),
		reserve_ (0)
	{}

private:
	uint16_t mode_;
	uint16_t reserve_;
};

/******************************************************************/
/******************************************************************/

class CommPacket_CS_SetRange : public CommPacket
{
protected:
	CommPacket_CS_SetRange (void) :
		CommPacket (COMM_OPCODE_CS_SET_RANGE)
	{}
};

class CommRequest_CS_SetRange : public CommPacket_CS_SetRange
{
private:
	CommRequest_CS_SetRange (void);

public:
	Comm_CS_Range range (void) const {
		return toComm_CS_Range (std::ntoh (range_));
	}

private:
	uint16_t range_;
	uint16_t reserve_;
};

class CommResponse_CS_SetRange : public CommPacket_CS_SetRange
{
public:
	CommResponse_CS_SetRange (Comm_CS_Range range) :
		range_ (std::hton ((uint16_t)(range))),
		reserve_ (0)
	{}

private:
	uint16_t range_;
	uint16_t reserve_;
};

/******************************************************************/

class CommPacket_CS_GetCalibration : public CommPacket
{
protected:
	CommPacket_CS_GetCalibration (void) :
		CommPacket (COMM_OPCODE_CS_GET_CALIBRATION)
	{}
};

class CommRequest_CS_GetCalibration : public CommPacket_CS_GetCalibration
{
private:
	CommRequest_CS_GetCalibration (void);

public:
	uint16_t index (void) const {return std::ntoh (index_);}

private:
	uint16_t index_;
	uint16_t reserve_;
};

class CommResponse_CS_GetCalibration : public CommPacket_CS_GetCalibration
{
public:
	CommResponse_CS_GetCalibration (uint16_t index,
									int16_t dac, float current) :
		index_ (std::hton (index)),
		dac_ (std::hton (dac)),
		current_ (std::hton (current))
	{}

private:
	uint16_t index_;
	int16_t dac_;
	float current_;
};

/******************************************************************/

class CommPacket_CS_VerifyCalibration : public CommPacket
{
protected:
	CommPacket_CS_VerifyCalibration (void) :
		CommPacket (COMM_OPCODE_CS_VERIFY_CALIBRATION)
	{}
};

class CommRequest_CS_VerifyCalibration :
	public CommPacket_CS_VerifyCalibration
{
private:
	CommRequest_CS_VerifyCalibration (void);

public:
	uint16_t index (void) const {return std::ntoh (index_);}

private:
	uint16_t index_;
	uint16_t reserve_;
};

class CommResponse_CS_VerifyCalibration :
	public CommPacket_CS_VerifyCalibration
{
public:
	CommResponse_CS_VerifyCalibration (uint16_t index,
									   int16_t dac, float current) :
		index_ (std::hton (index)),
		dac_ (std::hton (dac)),
		current_ (std::hton (current))
	{}

private:
	uint16_t index_;
	int16_t dac_;
	float current_;
};

/******************************************************************/

class CommPacket_CS_SetCalibration : public CommPacket
{
protected:
	CommPacket_CS_SetCalibration (void) :
		CommPacket (COMM_OPCODE_CS_SET_CALIBRATION)
	{}
};

class CommRequest_CS_SetCalibration : public CommPacket_CS_SetCalibration
{
private:
	CommRequest_CS_SetCalibration (void);

public:
	uint16_t index (void) const {return std::ntoh (index_);}
	float current (void) const {return std::ntoh (current_);}

private:
	uint16_t index_;
	uint16_t reserve_;
	float current_;
};

class CommResponse_CS_SetCalibration : public CommPacket_CS_SetCalibration
{
public:
	CommResponse_CS_SetCalibration (uint16_t index,
									int16_t dac, float current) :
		index_ (std::hton (index)),
		dac_ (std::hton (dac)),
		current_ (std::hton (current))
	{}

private:
	uint16_t index_;
	int16_t dac_;
	float current_;
};

/******************************************************************/

class CommPacket_CS_SaveCalibration : public CommPacket
{
protected:
	CommPacket_CS_SaveCalibration (void) :
		CommPacket (COMM_OPCODE_CS_SAVE_CALIBRATION)
	{}
};

class CommRequest_CS_SaveCalibration : public CommPacket_CS_SaveCalibration
{
private:
	CommRequest_CS_SaveCalibration (void);
};

class CommResponse_CS_SaveCalibration : public CommPacket_CS_SaveCalibration
{
public:
	CommResponse_CS_SaveCalibration (void) {}
};

/******************************************************************/

class CommPacket_CS_SetCurrent : public CommPacket
{
protected:
	CommPacket_CS_SetCurrent (void) :
		CommPacket (COMM_OPCODE_CS_SET_CURRENT)
	{}
};

class CommRequest_CS_SetCurrent : public CommPacket_CS_SetCurrent
{
private:
	CommRequest_CS_SetCurrent (void);

public:
	float current (void) const {return std::ntoh (current_);}

private:
	float current_;
};

class CommResponse_CS_SetCurrent : public CommPacket_CS_SetCurrent
{
public:
	CommResponse_CS_SetCurrent (float current) :
		current_ (std::hton (current))
	{}

private:
	float current_;
};

/******************************************************************/
/******************************************************************/

class CommPacket_VS_SetRange : public CommPacket
{
protected:
	CommPacket_VS_SetRange (void) :
		CommPacket (COMM_OPCODE_VS_SET_RANGE)
	{}
};

class CommRequest_VS_SetRange : public CommPacket_VS_SetRange
{
private:
	CommRequest_VS_SetRange (void);

public:
	Comm_VS_Range range (void) const {
		return toComm_VS_Range (std::ntoh (range_));
	}

private:
	uint16_t range_;
	uint16_t reserve_;
};

class CommResponse_VS_SetRange : public CommPacket_VS_SetRange
{
public:
	CommResponse_VS_SetRange (Comm_VS_Range range) :
		range_ (std::hton ((uint16_t)(range))),
		reserve_ (0)
	{}

private:
	uint16_t range_;
	uint16_t reserve_;
};

/******************************************************************/

class CommPacket_VS_GetCalibration : public CommPacket
{
protected:
	CommPacket_VS_GetCalibration (void) :
		CommPacket (COMM_OPCODE_VS_GET_CALIBRATION)
	{}
};

class CommRequest_VS_GetCalibration : public CommPacket_VS_GetCalibration
{
private:
	CommRequest_VS_GetCalibration (void);

public:
	uint16_t index (void) const {return std::ntoh (index_);}

private:
	uint16_t index_;
	uint16_t reserve_;
};

class CommResponse_VS_GetCalibration : public CommPacket_VS_GetCalibration
{
public:
	CommResponse_VS_GetCalibration (uint16_t index,
									int16_t dac, float voltage) :
		index_   (std::hton (index)),
		dac      (std::hton (dac)),
		voltage_ (std::hton (voltage))
	{}

private:
	uint16_t index_;
	int16_t  dac;
	float voltage_;
};

/******************************************************************/

class CommPacket_VS_VerifyCalibration : public CommPacket
{
protected:
	CommPacket_VS_VerifyCalibration (void) :
		CommPacket (COMM_OPCODE_VS_VERIFY_CALIBRATION)
	{}
};

class CommRequest_VS_VerifyCalibration :
	public CommPacket_VS_VerifyCalibration
{
private:
	CommRequest_VS_VerifyCalibration (void);

public:
	uint16_t index (void) const {return std::ntoh (index_);}

private:
	uint16_t index_;
	uint16_t reserve_;
};

class CommResponse_VS_VerifyCalibration :
	public CommPacket_VS_VerifyCalibration
{
public:
	CommResponse_VS_VerifyCalibration (uint16_t index,
									   int16_t dac, float voltage) :
		index_   (std::hton (index)),
		dac      (std::hton (dac)),
		voltage_ (std::hton (voltage))
	{}

private:
	uint16_t index_;
	int16_t  dac;
	float voltage_;
};

/******************************************************************/

class CommPacket_VS_SetCalibration : public CommPacket
{
protected:
	CommPacket_VS_SetCalibration (void) :
		CommPacket (COMM_OPCODE_VS_SET_CALIBRATION)
	{}
};

class CommRequest_VS_SetCalibration : public CommPacket_VS_SetCalibration
{
private:
	CommRequest_VS_SetCalibration (void);

public:
	uint16_t index (void) const {return std::ntoh (index_);}
	float voltage (void) const {return std::ntoh (voltage_);}

private:
	uint16_t index_;
	uint16_t reserve_;
	float voltage_;
};

class CommResponse_VS_SetCalibration : public CommPacket_VS_SetCalibration
{
public:
	CommResponse_VS_SetCalibration (uint16_t index,
									int16_t dac, float voltage) :
		index_   (std::hton (index)),
		dac      (std::hton (dac)),
		voltage_ (std::hton (voltage))
	{}

private:
	uint16_t index_;
	int16_t  dac;
	float voltage_;
};

/******************************************************************/

class CommPacket_VS_SaveCalibration : public CommPacket
{
protected:
	CommPacket_VS_SaveCalibration (void) :
		CommPacket (COMM_OPCODE_VS_SAVE_CALIBRATION)
	{}
};

class CommRequest_VS_SaveCalibration : public CommPacket_VS_SaveCalibration
{
private:
	CommRequest_VS_SaveCalibration (void);
};

class CommResponse_VS_SaveCalibration : public CommPacket_VS_SaveCalibration
{
public:
	CommResponse_VS_SaveCalibration (void) {}
};

/******************************************************************/

class CommPacket_VS_SetVoltage : public CommPacket
{
protected:
	CommPacket_VS_SetVoltage (void) :
		CommPacket (COMM_OPCODE_VS_SET_VOLTAGE)
	{}
};

class CommRequest_VS_SetVoltage : public CommPacket_VS_SetVoltage
{
private:
	CommRequest_VS_SetVoltage (void);

public:
	float voltage (void) const {return std::ntoh (voltage_);}

private:
	float voltage_;
};

class CommResponse_VS_SetVoltage : public CommPacket_VS_SetVoltage
{
public:
	CommResponse_VS_SetVoltage (float voltage) :
		voltage_ (std::hton (voltage))
	{}

private:
	float voltage_;
};

/******************************************************************/
/******************************************************************/

class CommPacket_CM_SetRange : public CommPacket
{
protected:
	CommPacket_CM_SetRange (void) :
		CommPacket (COMM_OPCODE_CM_SET_RANGE)
	{}
};

class CommRequest_CM_SetRange : public CommPacket_CM_SetRange
{
private:
	CommRequest_CM_SetRange (void);

public:
	Comm_CM_Range range (void) const {
		return toComm_CM_Range (std::ntoh (range_));
	}

private:
	uint16_t range_;
	uint16_t reserve_;
};

class CommResponse_CM_SetRange : public CommPacket_CM_SetRange
{
public:
	CommResponse_CM_SetRange (Comm_CM_Range range) :
		range_   (std::hton ((uint16_t)(range))),
		reserve_ (0)
	{}

private:
	uint16_t range_;
	uint16_t reserve_;
};

/******************************************************************/

class CommPacket_CM_GetCalibration : public CommPacket
{
protected:
	CommPacket_CM_GetCalibration (void) :
		CommPacket (COMM_OPCODE_CM_GET_CALIBRATION)
	{}
};

class CommRequest_CM_GetCalibration : public CommPacket_CM_GetCalibration
{
private:
	CommRequest_CM_GetCalibration (void);

public:
	uint16_t index (void) const {return std::ntoh (index_);}

private:
	uint16_t index_;
	uint16_t reserve_;
};

class CommResponse_CM_GetCalibration : public CommPacket_CM_GetCalibration
{
public:
	CommResponse_CM_GetCalibration (uint16_t index,
									int32_t adc, float current) :
		index_   (std::hton (index)),
		reserve_ (0),
		adc_     (std::hton (adc)),
		current_ (std::hton (current))
	{}

private:
	uint16_t index_;
	uint16_t reserve_;
	int32_t  adc_;
	float    current_;
};

/******************************************************************/

class CommPacket_CM_SetCalibration : public CommPacket
{
protected:
	CommPacket_CM_SetCalibration (void) :
		CommPacket (COMM_OPCODE_CM_SET_CALIBRATION)
	{}
};

class CommRequest_CM_SetCalibration : public CommPacket_CM_SetCalibration
{
private:
	CommRequest_CM_SetCalibration (void);

public:
	uint16_t index (void) const {return std::ntoh (index_);}
	float current (void) const {return std::ntoh (current_);}

private:
	uint16_t index_;
	uint16_t reserve_;
	float    current_;
};

class CommResponse_CM_SetCalibration : public CommPacket_CM_SetCalibration
{
public:
	CommResponse_CM_SetCalibration (uint16_t index,
									int32_t adc, float current) :
		index_   (std::hton (index)),
		reserve_ (0),
		adc_     (std::hton (adc)),
		current_ (std::hton (current))
	{}

private:
	uint16_t index_;
	uint16_t reserve_;
	int32_t  adc_;
	float    current_;
};

/******************************************************************/

class CommPacket_CM_SaveCalibration : public CommPacket
{
protected:
	CommPacket_CM_SaveCalibration (void) :
		CommPacket (COMM_OPCODE_CM_SAVE_CALIBRATION)
	{}
};

class CommRequest_CM_SaveCalibration : public CommPacket_CM_SaveCalibration
{
private:
	CommRequest_CM_SaveCalibration (void);
};

class CommResponse_CM_SaveCalibration : public CommPacket_CM_SaveCalibration
{
public:
	CommResponse_CM_SaveCalibration (void) {}
};

/******************************************************************/

class CommPacket_CM_Read : public CommPacket
{
protected:
	CommPacket_CM_Read (void) :
		CommPacket (COMM_OPCODE_CM_READ)
	{}
};

class CommRequest_CM_Read : public CommPacket_CM_Read
{
private:
	CommRequest_CM_Read (void);

public:
	uint16_t filterLength (void) const { return std::ntoh (filterLength_); }

private:
	uint16_t filterLength_;
	uint16_t reserve_;
};

class CommResponse_CM_Read : public CommPacket_CM_Read
{
public:
	CommResponse_CM_Read (float current) :
		current_ (std::hton (current))
	{}

private:
	float current_;
};

/******************************************************************/
/******************************************************************/

class CommPacket_VM_SetRange : public CommPacket
{
protected:
	CommPacket_VM_SetRange (void) :
		CommPacket (COMM_OPCODE_VM_SET_RANGE)
	{}
};

class CommRequest_VM_SetRange : public CommPacket_VM_SetRange
{
private:
	CommRequest_VM_SetRange (void);

public:
	Comm_VM_Range range (void) const {
		return toComm_VM_Range (std::ntoh (range_));
	}

private:
	uint16_t range_;
	uint16_t reserve_;
};

class CommResponse_VM_SetRange : public CommPacket_VM_SetRange
{
public:
	CommResponse_VM_SetRange (Comm_VM_Range range) :
		range_   (std::hton ((uint16_t)(range))),
		reserve_ (0)
	{}

private:
	uint16_t range_;
	uint16_t reserve_;
};

/******************************************************************/

class CommPacket_VM_GetCalibration : public CommPacket
{
protected:
	CommPacket_VM_GetCalibration (void) :
		CommPacket (COMM_OPCODE_VM_GET_CALIBRATION)
	{}
};

class CommRequest_VM_GetCalibration : public CommPacket_VM_GetCalibration
{
private:
	CommRequest_VM_GetCalibration (void);

public:
	uint16_t index (void) const { return std::ntoh (index_); }

private:
	uint16_t index_;
	uint16_t reserve_;
};

class CommResponse_VM_GetCalibration : public CommPacket_VM_GetCalibration
{
public:
	CommResponse_VM_GetCalibration (uint16_t index,
									int32_t adc, float voltage) :
		index_   (std::hton (index)),
		reserve_ (0),
		adc_     (std::hton (adc)),
		voltage_ (std::hton (voltage))
	{}

private:
	uint16_t index_;
	uint16_t reserve_;
	int32_t  adc_;
	float    voltage_;
};

/******************************************************************/

class CommPacket_VM_SetCalibration : public CommPacket
{
protected:
	CommPacket_VM_SetCalibration (void) :
		CommPacket (COMM_OPCODE_VM_SET_CALIBRATION)
	{}
};

class CommRequest_VM_SetCalibration : public CommPacket_VM_SetCalibration
{
private:
	CommRequest_VM_SetCalibration (void);

public:
	uint16_t index (void) const { return std::ntoh (index_); }
	float  voltage (void) const { return std::ntoh (voltage_); }

private:
	uint16_t index_;
	uint16_t reserve_;
	float voltage_;
};

class CommResponse_VM_SetCalibration : public CommPacket_VM_SetCalibration
{
public:
	CommResponse_VM_SetCalibration (uint16_t index,
									int32_t adc, float voltage) :
		index_   (std::hton (index)),
		reserve_ (0),
		adc_     (std::hton (adc)),
		voltage_ (std::hton (voltage))
	{}

private:
	uint16_t index_;
	uint16_t reserve_;
	int32_t  adc_;
	float    voltage_;
};

/******************************************************************/

class CommPacket_VM_SaveCalibration : public CommPacket
{
protected:
	CommPacket_VM_SaveCalibration (void) :
		CommPacket (COMM_OPCODE_VM_SAVE_CALIBRATION)
	{}
};

class CommRequest_VM_SaveCalibration : public CommPacket_VM_SaveCalibration
{
private:
	CommRequest_VM_SaveCalibration (void);
};

class CommResponse_VM_SaveCalibration : public CommPacket_VM_SaveCalibration
{
public:
	CommResponse_VM_SaveCalibration (void) {}
};

/******************************************************************/

class CommPacket_VM_Read : public CommPacket
{
protected:
	CommPacket_VM_Read (void) :
		CommPacket (COMM_OPCODE_VM_READ)
	{}
};

class CommRequest_VM_Read : public CommPacket_VM_Read
{
private:
	CommRequest_VM_Read (void);

public:
	uint16_t filterLength (void) const { return std::ntoh (filterLength_); }

private:
	uint16_t filterLength_;
	uint16_t reserve_;
};

class CommResponse_VM_Read : public CommPacket_VM_Read
{
public:
	CommResponse_VM_Read (float voltage) :
		voltage_ (std::hton (voltage))
	{}

private:
	float voltage_;
};

/******************************************************************/
/******************************************************************/

class CommPacket_CS_LoadDefaultCalibration : public CommPacket
{
protected:
	CommPacket_CS_LoadDefaultCalibration (void) :
		CommPacket (COMM_OPCODE_CS_LOAD_DEFAULT_CALIBRATION)
	{}
};

class CommRequest_CS_LoadDefaultCalibration : public CommPacket_CS_LoadDefaultCalibration
{
private:
	CommRequest_CS_LoadDefaultCalibration (void);
};

class CommResponse_CS_LoadDefaultCalibration : public CommPacket_CS_LoadDefaultCalibration
{
public:
	CommResponse_CS_LoadDefaultCalibration (void) {}
};

/******************************************************************/

class CommPacket_VS_LoadDefaultCalibration : public CommPacket
{
protected:
	CommPacket_VS_LoadDefaultCalibration (void) :
		CommPacket (COMM_OPCODE_VS_LOAD_DEFAULT_CALIBRATION)
	{}
};

class CommRequest_VS_LoadDefaultCalibration : public CommPacket_VS_LoadDefaultCalibration
{
private:
	CommRequest_VS_LoadDefaultCalibration (void);
};

class CommResponse_VS_LoadDefaultCalibration : public CommPacket_VS_LoadDefaultCalibration
{
public:
	CommResponse_VS_LoadDefaultCalibration (void) {}
};

/******************************************************************/

class CommPacket_CM_LoadDefaultCalibration : public CommPacket
{
protected:
	CommPacket_CM_LoadDefaultCalibration (void) :
		CommPacket (COMM_OPCODE_CM_LOAD_DEFAULT_CALIBRATION)
	{}
};

class CommRequest_CM_LoadDefaultCalibration : public CommPacket_CM_LoadDefaultCalibration
{
private:
	CommRequest_CM_LoadDefaultCalibration (void);
};

class CommResponse_CM_LoadDefaultCalibration : public CommPacket_CM_LoadDefaultCalibration
{
public:
	CommResponse_CM_LoadDefaultCalibration (void) {}
};

/******************************************************************/

class CommPacket_VM_LoadDefaultCalibration : public CommPacket
{
protected:
	CommPacket_VM_LoadDefaultCalibration (void) :
		CommPacket (COMM_OPCODE_VM_LOAD_DEFAULT_CALIBRATION)
	{}
};

class CommRequest_VM_LoadDefaultCalibration : public CommPacket_VM_LoadDefaultCalibration
{
private:
	CommRequest_VM_LoadDefaultCalibration (void);
};

class CommResponse_VM_LoadDefaultCalibration : public CommPacket_VM_LoadDefaultCalibration
{
public:
	CommResponse_VM_LoadDefaultCalibration (void) {}
};

/******************************************************************/
/******************************************************************/

class CommPacket_RM_ReadAutoscale : public CommPacket
{
protected:
	CommPacket_RM_ReadAutoscale (void) :
		CommPacket (COMM_OPCODE_RM_READ_AUTOSCALE)
	{}
};

class CommRequest_RM_ReadAutoscale : public CommPacket_RM_ReadAutoscale
{
private:
	CommRequest_RM_ReadAutoscale (void);

public:
	uint16_t filterLength (void) const { return std::ntoh (filterLength_); }

private:
	uint16_t filterLength_;
	uint16_t reserve_;
};

class CommResponse_RM_ReadAutoscale : public CommPacket_RM_ReadAutoscale
{
public:
	CommResponse_RM_ReadAutoscale (float resistance) :
		resistance_ (std::hton (resistance))
	{}

private:
	float resistance_;
};

/******************************************************************/
/******************************************************************/

enum SystemConfigParamID
{
	COMM_SYSTEM_CONFIG_PARAM_ID_HW_BOARD_NO = 0,
	COMM_SYSTEM_CONFIG_PARAM_ID_HW_BOM_NO,
	COMM_SYSTEM_CONFIG_PARAM_ID_HW_BUGFIX_NO,
};

class CommPacket_SystemConfig_Get : public CommPacket
{
protected:
	CommPacket_SystemConfig_Get (void) :
		CommPacket (COMM_OPCODE_SYSTEM_CONFIG_GET)
	{}
};

class CommRequest_SystemConfig_Get : public CommPacket_SystemConfig_Get
{
private:
	CommRequest_SystemConfig_Get (void);

public:
	int16_t paramID (void) const { return std::ntoh (paramID_); }

private:
	int16_t paramID_;
	int16_t reserve_;
};

class CommResponse_SystemConfig_Get : public CommPacket_SystemConfig_Get
{
public:
	CommResponse_SystemConfig_Get (int16_t paramID, int16_t value) :
		paramID_ (std::hton (paramID)),
		value_   (std::hton (value))
	{}

private:
	int16_t paramID_;
	int16_t value_;
};

/******************************************************************/

class CommPacket_SystemConfig_Set : public CommPacket
{
protected:
	CommPacket_SystemConfig_Set (void) :
		CommPacket (COMM_OPCODE_SYSTEM_CONFIG_SET)
	{}
};

class CommRequest_SystemConfig_Set : public CommPacket_SystemConfig_Set
{
private:
	CommRequest_SystemConfig_Set (void);

public:
	int16_t paramID (void) const { return std::ntoh (paramID_); }
	int16_t value   (void) const { return std::ntoh (value_); }

private:
	int16_t paramID_;
	int16_t value_;
};

class CommResponse_SystemConfig_Set : public CommPacket_SystemConfig_Set
{
public:
	CommResponse_SystemConfig_Set (int16_t paramID, int16_t value) :
		paramID_ (std::hton (paramID)),
		value_   (std::hton (value))
	{}

private:
	int16_t paramID_;
	int16_t value_;
};

/******************************************************************/

class CommPacket_SystemConfig_Save : public CommPacket
{
protected:
	CommPacket_SystemConfig_Save (void) :
		CommPacket (COMM_OPCODE_SYSTEM_CONFIG_SAVE)
	{}
};

class CommRequest_SystemConfig_Save : public CommPacket_SystemConfig_Save
{
private:
	CommRequest_SystemConfig_Save (void);
};

class CommResponse_SystemConfig_Save : public CommPacket_SystemConfig_Save
{
public:
	CommResponse_SystemConfig_Save (void) {}
};

/******************************************************************/

class CommPacket_SystemConfig_LoadDefault : public CommPacket
{
protected:
	CommPacket_SystemConfig_LoadDefault (void) :
		CommPacket (COMM_OPCODE_SYSTEM_CONFIG_LOAD_DEFAULT)
	{}
};

class CommRequest_SystemConfig_LoadDefault :
	public CommPacket_SystemConfig_LoadDefault
{
private:
	CommRequest_SystemConfig_LoadDefault (void);
};

class CommResponse_SystemConfig_LoadDefault :
	public CommPacket_SystemConfig_LoadDefault
{
public:
	CommResponse_SystemConfig_LoadDefault (void) {}
};

/******************************************************************/
/******************************************************************/

class CommPacket_VM2_SetRange : public CommPacket
{
protected:
	CommPacket_VM2_SetRange (void) :
		CommPacket (COMM_OPCODE_VM2_SET_RANGE)
	{}
};

class CommRequest_VM2_SetRange : public CommPacket_VM2_SetRange
{
private:
	CommRequest_VM2_SetRange (void);

public:
	Comm_VM2_Range range (void) const {
		return toComm_VM2_Range (std::ntoh (range_));
	}

private:
	uint16_t range_;
	uint16_t reserve_;
};

class CommResponse_VM2_SetRange : public CommPacket_VM2_SetRange
{
public:
	CommResponse_VM2_SetRange (Comm_VM2_Range range) :
		range_   (std::hton ((uint16_t)(range))),
		reserve_ (0)
	{}

private:
	uint16_t range_;
	uint16_t reserve_;
};

/******************************************************************/

class CommPacket_VM2_GetCalibration : public CommPacket
{
protected:
	CommPacket_VM2_GetCalibration (void) :
		CommPacket (COMM_OPCODE_VM2_GET_CALIBRATION)
	{}
};

class CommRequest_VM2_GetCalibration : public CommPacket_VM2_GetCalibration
{
private:
	CommRequest_VM2_GetCalibration (void);

public:
	uint16_t index (void) const { return std::ntoh (index_); }

private:
	uint16_t index_;
	uint16_t reserve_;
};

class CommResponse_VM2_GetCalibration : public CommPacket_VM2_GetCalibration
{
public:
	CommResponse_VM2_GetCalibration (uint16_t index,
									int32_t adc, float voltage) :
		index_   (std::hton (index)),
		reserve_ (0),
		adc_     (std::hton (adc)),
		voltage_ (std::hton (voltage))
	{}

private:
	uint16_t index_;
	uint16_t reserve_;
	int32_t  adc_;
	float    voltage_;
};

/******************************************************************/

class CommPacket_VM2_LoadDefaultCalibration : public CommPacket
{
protected:
	CommPacket_VM2_LoadDefaultCalibration (void) :
		CommPacket (COMM_OPCODE_VM2_LOAD_DEFAULT_CALIBRATION)
	{}
};

class CommRequest_VM2_LoadDefaultCalibration : public CommPacket_VM2_LoadDefaultCalibration
{
private:
	CommRequest_VM2_LoadDefaultCalibration (void);
};

class CommResponse_VM2_LoadDefaultCalibration : public CommPacket_VM2_LoadDefaultCalibration
{
public:
	CommResponse_VM2_LoadDefaultCalibration (void) {}
};

/******************************************************************/

class CommPacket_VM2_SetCalibration : public CommPacket
{
protected:
	CommPacket_VM2_SetCalibration (void) :
		CommPacket (COMM_OPCODE_VM2_SET_CALIBRATION)
	{}
};

class CommRequest_VM2_SetCalibration : public CommPacket_VM2_SetCalibration
{
private:
	CommRequest_VM2_SetCalibration (void);

public:
	uint16_t index (void) const { return std::ntoh (index_); }
	float  voltage (void) const { return std::ntoh (voltage_); }

private:
	uint16_t index_;
	uint16_t reserve_;
	float voltage_;
};

class CommResponse_VM2_SetCalibration : public CommPacket_VM2_SetCalibration
{
public:
	CommResponse_VM2_SetCalibration (uint16_t index,
									int32_t adc, float voltage) :
		index_   (std::hton (index)),
		reserve_ (0),
		adc_     (std::hton (adc)),
		voltage_ (std::hton (voltage))
	{}

private:
	uint16_t index_;
	uint16_t reserve_;
	int32_t  adc_;
	float    voltage_;
};

/******************************************************************/

class CommPacket_VM2_SaveCalibration : public CommPacket
{
protected:
	CommPacket_VM2_SaveCalibration (void) :
		CommPacket (COMM_OPCODE_VM2_SAVE_CALIBRATION)
	{}
};

class CommRequest_VM2_SaveCalibration : public CommPacket_VM2_SaveCalibration
{
private:
	CommRequest_VM2_SaveCalibration (void);
};

class CommResponse_VM2_SaveCalibration : public CommPacket_VM2_SaveCalibration
{
public:
	CommResponse_VM2_SaveCalibration (void) {}
};

/******************************************************************/

class CommPacket_VM2_Read : public CommPacket
{
protected:
	CommPacket_VM2_Read (void) :
		CommPacket (COMM_OPCODE_VM2_READ)
	{}
};

class CommRequest_VM2_Read : public CommPacket_VM2_Read
{
private:
	CommRequest_VM2_Read (void);

public:
	uint16_t filterLength (void) const { return std::ntoh (filterLength_); }

private:
	uint16_t filterLength_;
	uint16_t reserve_;
};

class CommResponse_VM2_Read : public CommPacket_VM2_Read
{
public:
	CommResponse_VM2_Read (float voltage) :
		voltage_ (std::hton (voltage))
	{}

private:
	float voltage_;
};

/******************************************************************/
/******************************************************************/

class CommPacket_VM_SetTerminal : public CommPacket
{
protected:
	CommPacket_VM_SetTerminal (void) :
		CommPacket (COMM_OPCODE_VM_SET_TERMINAL)
	{}
};

class CommRequest_VM_SetTerminal : public CommPacket_VM_SetTerminal
{
private:
	CommRequest_VM_SetTerminal (void);

public:
	Comm_VM_Terminal terminal (void) const {
		return toComm_VM_Terminal (std::ntoh (terminal_));
	}

private:
	uint16_t terminal_;
	uint16_t reserve_;
};

class CommResponse_VM_SetTerminal : public CommPacket_VM_SetTerminal
{
public:
	CommResponse_VM_SetTerminal (Comm_VM_Terminal terminal) :
		terminal_   (std::hton ((uint16_t)(terminal))),
		reserve_ (0)
	{}

private:
	uint16_t terminal_;
	uint16_t reserve_;
};

/******************************************************************/
/******************************************************************/

class CommPacket_VM_GetTerminal : public CommPacket
{
protected:
	CommPacket_VM_GetTerminal (void) :
		CommPacket (COMM_OPCODE_VM_SET_TERMINAL)
	{}
};

class CommRequest_VM_GetTerminal : public CommPacket_VM_GetTerminal
{
private:
	CommRequest_VM_GetTerminal (void);
};

class CommResponse_VM_GetTerminal : public CommPacket_VM_GetTerminal
{
public:
	CommResponse_VM_GetTerminal (Comm_VM_Terminal terminal) :
		terminal_   (std::hton ((uint16_t)(terminal))),
		reserve_ (0)
	{}

private:
	uint16_t terminal_;
	uint16_t reserve_;
};

/******************************************************************/
/******************************************************************/

class CommPacket_changeBaud : public CommPacket
{
protected:
	CommPacket_changeBaud (void) :
		CommPacket (COMM_OPCODE_CHANGE_BAUD)
	{}
};

class CommRequest_changeBaud : public CommPacket_changeBaud
{
private:
	CommRequest_changeBaud (void);

public:
	uint32_t baudRate (void) const {return std::ntoh (baudRate_);}

private:
	uint32_t baudRate_;
};

class CommResponse_changeBaud : public CommPacket_changeBaud
{
public:
	CommResponse_changeBaud (uint32_t baudRate) :
		baudRate_ (std::hton (baudRate))
	{}

private:
	uint32_t baudRate_;
};

/******************************************************************/

class CommPacket_recSize : public CommPacket
{
protected:
	CommPacket_recSize (void) :
		CommPacket (COMM_OPCODE_REC_SIZE)
	{}
};

class CommRequest_recSize : public CommPacket_recSize
{
private:
	CommRequest_recSize (void);

public:
	uint32_t recSize (void) const {return std::ntoh (recSize_);}

private:
	uint32_t recSize_;
};

class CommResponse_recSize : public CommPacket_recSize
{
public:
	CommResponse_recSize (uint32_t recSize) :
		recSize_ (std::hton (recSize))
	{}

private:
	uint32_t recSize_;
};

/******************************************************************/

class CommPacket_recData : public CommPacket
{
protected:
	CommPacket_recData (void) :
		CommPacket (COMM_OPCODE_REC_DATA)
	{}
};

class CommRequest_recData : public CommPacket_recData
{
private:
	CommRequest_recData (void);

public:
	int32_t *recData (void) const {return std::ntoh (recData_);}

private:
	int32_t *recData_;
};

class CommResponse_recData : public CommPacket_recData
{
public:
	CommResponse_recData (int32_t *recData) :
		recData_ (std::hton (recData))
	{}

private:
	int32_t *recData_;
};

/******************************************************************/
/******************************************************************/

enum Comm_CallbackCode
{
	COMM_CBCODE_NOP,                             //00
	COMM_CBCODE_IDN,                             //01
	COMM_CBCODE_KEEP_ALIVE,                      //02
	COMM_CBCODE_SET_SOURCE_MODE,                 //03
	COMM_CBCODE_CS_SET_RANGE,                    //04
	COMM_CBCODE_CS_GET_CALIBRATION,              //05
	COMM_CBCODE_CS_VERIFY_CALIBRATION,           //06
	COMM_CBCODE_CS_SET_CALIBRATION,              //07
	COMM_CBCODE_CS_SAVE_CALIBRATION,             //08
	COMM_CBCODE_CS_SET_CURRENT,                  //09

	COMM_CBCODE_VS_SET_RANGE,                    //10
	COMM_CBCODE_VS_GET_CALIBRATION,              //11
	COMM_CBCODE_VS_VERIFY_CALIBRATION,           //12
	COMM_CBCODE_VS_SET_CALIBRATION,              //13
	COMM_CBCODE_VS_SAVE_CALIBRATION,             //14
	COMM_CBCODE_VS_SET_VOLTAGE,                  //15
	COMM_CBCODE_CM_SET_RANGE,                    //16
	COMM_CBCODE_CM_GET_CALIBRATION,              //17
	COMM_CBCODE_CM_SET_CALIBRATION,              //18
	COMM_CBCODE_CM_SAVE_CALIBRATION,             //19

	COMM_CBCODE_CM_READ,                         //20
	COMM_CBCODE_VM_SET_RANGE,                    //21
	COMM_CBCODE_VM_GET_CALIBRATION,              //22
	COMM_CBCODE_VM_SET_CALIBRATION,              //23
	COMM_CBCODE_VM_SAVE_CALIBRATION,             //24
	COMM_CBCODE_VM_READ,                         //25
	COMM_CBCODE_CS_LOAD_DEFAULT_CALIBRATION,     //26
	COMM_CBCODE_VS_LOAD_DEFAULT_CALIBRATION,     //27
	COMM_CBCODE_CM_LOAD_DEFAULT_CALIBRATION,     //28
	COMM_CBCODE_VM_LOAD_DEFAULT_CALIBRATION,     //29

	COMM_CBCODE_RM_READ_AUTOSCALE,               //30
	COMM_CBCODE_SYSTEM_CONFIG_GET,               //31
	COMM_CBCODE_SYSTEM_CONFIG_SET,               //32
	COMM_CBCODE_SYSTEM_CONFIG_SAVE,              //33
	COMM_CBCODE_SYSTEM_CONFIG_LOAD_DEFAULT,      //34
	COMM_CBCODE_VM2_SET_RANGE,                   //35
	COMM_CBCODE_VM2_GET_CALIBRATION,             //36
	COMM_CBCODE_VM2_SET_CALIBRATION,             //37
	COMM_CBCODE_VM2_SAVE_CALIBRATION,            //38
	COMM_CBCODE_VM2_READ,                        //39

	COMM_CBCODE_VM2_LOAD_DEFAULT_CALIBRATION,    //40
	COMM_CBCODE_VM_SET_TERMINAL,                 //41
	COMM_CBCODE_VM_GET_TERMINAL,                 //42

	COMM_CBCODE_CHANGE_BAUD,                     //43
	COMM_CBCODE_REC_SIZE,                        //44
	COMM_CBCODE_REC_DATA,                        //45
};

/******************************************************************/

class CommCB
{
protected:
	CommCB (Comm_CallbackCode code) :
		code_ (code)
	{}

public:
	Comm_CallbackCode code (void) const {return code_;}

private:
	Comm_CallbackCode code_;
};

/******************************************************************/

class CommCB_Identity : public CommCB
{
public:
	CommCB_Identity (void) :
		CommCB (COMM_CBCODE_IDN)
	{}
};

class CommCB_keepAlive : public CommCB
{
public:
	CommCB_keepAlive (uint32_t leaseTime_ms) :
		CommCB (COMM_CBCODE_KEEP_ALIVE),
		leaseTime_ms_ (leaseTime_ms)
	{}

public:
	uint32_t leaseTime_ms (void) const { return leaseTime_ms_; }

private:
	uint32_t leaseTime_ms_;
};

class CommCB_SetSourceMode : public CommCB
{
public:
	CommCB_SetSourceMode (Comm_SourceMode mode) :
		CommCB (COMM_CBCODE_SET_SOURCE_MODE),
		mode_ (mode)
	{}

public:
	Comm_SourceMode mode (void) const { return mode_; }

private:
	Comm_SourceMode mode_;
};

/******************************************************************/

class CommCB_CS_SetRange : public CommCB
{
public:
	CommCB_CS_SetRange (Comm_CS_Range range) :
		CommCB (COMM_CBCODE_CS_SET_RANGE),
		range_ (range)
	{}

public:
	Comm_CS_Range range (void) const { return range_; }

private:
	Comm_CS_Range range_;
};

class CommCB_CS_GetCalibration : public CommCB
{
public:
	CommCB_CS_GetCalibration (uint16_t index) :
		CommCB (COMM_CBCODE_CS_GET_CALIBRATION),
		index_ (index)
	{}

public:
	uint16_t index (void) const {return index_;}

private:
	uint16_t index_;
};

class CommCB_CS_VerifyCalibration : public CommCB
{
public:
	CommCB_CS_VerifyCalibration (uint16_t index) :
		CommCB (COMM_CBCODE_CS_VERIFY_CALIBRATION),
		index_ (index)
	{}

public:
	uint16_t index (void) const {return index_;}

private:
	uint16_t index_;
};

class CommCB_CS_SetCalibration : public CommCB
{
public:
	CommCB_CS_SetCalibration (uint16_t index, float current) :
		CommCB   (COMM_CBCODE_CS_SET_CALIBRATION),
		index_   (index),
		current_ (current)
	{}

public:
	uint16_t index   (void) const { return index_; }
	float    current (void) const { return current_; }

private:
	uint16_t index_;
	float current_;
};

class CommCB_CS_SaveCalibration : public CommCB
{
public:
	CommCB_CS_SaveCalibration (void) :
		CommCB (COMM_CBCODE_CS_SAVE_CALIBRATION)
	{}
};

class CommCB_CS_SetCurrent : public CommCB
{
public:
	CommCB_CS_SetCurrent (float current) :
		CommCB (COMM_CBCODE_CS_SET_CURRENT),
		current_ (current)
	{}

public:
	float current (void) const { return current_; }

private:
	float current_;
};

/******************************************************************/

class CommCB_VS_SetRange : public CommCB
{
public:
	CommCB_VS_SetRange (Comm_VS_Range range) :
		CommCB (COMM_CBCODE_VS_SET_RANGE),
		range_ (range)
	{}

public:
	Comm_VS_Range range (void) const { return range_; }

private:
	Comm_VS_Range range_;
};

class CommCB_VS_GetCalibration : public CommCB
{
public:
	CommCB_VS_GetCalibration (uint16_t index) :
		CommCB (COMM_CBCODE_VS_GET_CALIBRATION),
		index_ (index)
	{}

public:
	uint16_t index (void) const { return index_; }

private:
	uint16_t index_;
};

class CommCB_VS_VerifyCalibration : public CommCB
{
public:
	CommCB_VS_VerifyCalibration (uint16_t index) :
		CommCB (COMM_CBCODE_VS_VERIFY_CALIBRATION),
		index_ (index)
	{}

public:
	uint16_t index (void) const { return index_; }

private:
	uint16_t index_;
};

class CommCB_VS_SetCalibration : public CommCB
{
public:
	CommCB_VS_SetCalibration (uint16_t index, float voltage) :
		CommCB   (COMM_CBCODE_VS_SET_CALIBRATION),
		index_   (index),
		voltage_ (voltage)
	{}

public:
	uint16_t index   (void) const { return index_; }
	float    voltage (void) const { return voltage_; }

private:
	uint16_t index_;
	float voltage_;
};

class CommCB_VS_SaveCalibration : public CommCB
{
public:
	CommCB_VS_SaveCalibration (void) :
		CommCB (COMM_CBCODE_VS_SAVE_CALIBRATION)
	{}
};

class CommCB_VS_SetVoltage : public CommCB
{
public:
	CommCB_VS_SetVoltage (float voltage) :
		CommCB    (COMM_CBCODE_VS_SET_VOLTAGE),
		voltage_ (voltage)
	{}

public:
	float voltage (void) const { return voltage_; }

private:
	float voltage_;
};

/******************************************************************/

class CommCB_CM_SetRange : public CommCB
{
public:
	CommCB_CM_SetRange (Comm_CM_Range range) :
		CommCB (COMM_CBCODE_CM_SET_RANGE),
		range_ (range)
	{}

public:
	Comm_CM_Range range (void) const { return range_; }

private:
	Comm_CM_Range range_;
};

class CommCB_CM_GetCalibration : public CommCB
{
public:
	CommCB_CM_GetCalibration (uint16_t index) :
		CommCB (COMM_CBCODE_CM_GET_CALIBRATION),
		index_ (index)
	{}

public:
	uint16_t index (void) const { return index_; }

private:
	uint16_t index_;
};

class CommCB_CM_SetCalibration : public CommCB
{
public:
	CommCB_CM_SetCalibration (uint16_t index, float current) :
		CommCB   (COMM_CBCODE_CM_SET_CALIBRATION),
		index_   (index),
		current_ (current)
	{}

public:
	uint16_t index   (void) const { return index_;   }
	float    current (void) const { return current_; }

private:
	uint16_t index_;
	float current_;
};

class CommCB_CM_SaveCalibration : public CommCB
{
public:
	CommCB_CM_SaveCalibration (void) :
		CommCB (COMM_CBCODE_CM_SAVE_CALIBRATION)
	{}
};

class CommCB_CM_Read : public CommCB
{
public:
	CommCB_CM_Read (uint16_t filterLength) :
		CommCB (COMM_CBCODE_CM_READ),
		filterLength_ (filterLength)
	{}

public:
	uint16_t filterLength (void) const { return filterLength_; }

private:
	uint16_t filterLength_;
};

/******************************************************************/

class CommCB_VM_SetRange : public CommCB
{
public:
	CommCB_VM_SetRange (Comm_VM_Range range) :
		CommCB (COMM_CBCODE_VM_SET_RANGE),
		range_ (range)
	{}

public:
	Comm_VM_Range range (void) const { return range_; }

private:
	Comm_VM_Range range_;
};

class CommCB_VM_GetCalibration : public CommCB
{
public:
	CommCB_VM_GetCalibration (uint16_t index) :
		CommCB (COMM_CBCODE_VM_GET_CALIBRATION),
		index_ (index)
	{}

public:
	uint16_t index (void) const { return index_; }

private:
	uint16_t index_;
};

class CommCB_VM_SetCalibration : public CommCB
{
public:
	CommCB_VM_SetCalibration (uint16_t index, float voltage) :
		CommCB   (COMM_CBCODE_VM_SET_CALIBRATION),
		index_   (index),
		voltage_ (voltage)
	{}

public:
	uint16_t index   (void) const { return index_;   }
	float    voltage (void) const { return voltage_; }

private:
	uint16_t index_;
	float    voltage_;
};

class CommCB_VM_SaveCalibration : public CommCB
{
public:
	CommCB_VM_SaveCalibration (void) :
		CommCB (COMM_CBCODE_VM_SAVE_CALIBRATION)
	{}
};

class CommCB_VM_Read : public CommCB
{
public:
	CommCB_VM_Read (uint16_t filterLength) :
		CommCB (COMM_CBCODE_VM_READ),
		filterLength_ (filterLength)
	{}

public:
	uint16_t filterLength (void) const { return filterLength_; }

private:
	uint16_t filterLength_;
};

/******************************************************************/

class CommCB_CS_LoadDefaultCalibration : public CommCB
{
public:
	CommCB_CS_LoadDefaultCalibration (void) :
		CommCB (COMM_CBCODE_CS_LOAD_DEFAULT_CALIBRATION)
	{}
};

class CommCB_VS_LoadDefaultCalibration : public CommCB
{
public:
	CommCB_VS_LoadDefaultCalibration (void) :
		CommCB (COMM_CBCODE_VS_LOAD_DEFAULT_CALIBRATION)
	{}
};

class CommCB_CM_LoadDefaultCalibration : public CommCB
{
public:
	CommCB_CM_LoadDefaultCalibration (void) :
		CommCB (COMM_CBCODE_CM_LOAD_DEFAULT_CALIBRATION)
	{}
};

class CommCB_VM_LoadDefaultCalibration : public CommCB
{
public:
	CommCB_VM_LoadDefaultCalibration (void) :
		CommCB (COMM_CBCODE_VM_LOAD_DEFAULT_CALIBRATION)
	{}
};

/******************************************************************/

class CommCB_RM_ReadAutoscale : public CommCB
{
public:
	CommCB_RM_ReadAutoscale (uint16_t filterLength) :
		CommCB (COMM_CBCODE_RM_READ_AUTOSCALE),
		filterLength_ (filterLength)
	{}

public:
	uint16_t filterLength (void) const { return filterLength_; }

private:
	uint16_t filterLength_;
};

/******************************************************************/

class CommCB_SystemConfig_Get : public CommCB
{
public:
	CommCB_SystemConfig_Get (int16_t paramID) :
		CommCB (COMM_CBCODE_SYSTEM_CONFIG_GET),
		paramID_ (paramID)
	{}

public:
	int16_t paramID (void) const { return paramID_; }

private:
	int16_t paramID_;
};

class CommCB_SystemConfig_Set : public CommCB
{
public:
	CommCB_SystemConfig_Set (int16_t paramID, int16_t value) :
		CommCB (COMM_CBCODE_SYSTEM_CONFIG_SET),
		paramID_ (paramID),
		value_ (value)
	{}

public:
	int16_t paramID (void) const { return paramID_; }
	int16_t value   (void) const { return value_;   }

private:
	int16_t paramID_;
	int16_t value_;
};

class CommCB_SystemConfig_Save : public CommCB
{
public:
	CommCB_SystemConfig_Save (void) :
		CommCB (COMM_CBCODE_SYSTEM_CONFIG_SAVE)
	{}
};

class CommCB_SystemConfig_LoadDefault : public CommCB
{
public:
	CommCB_SystemConfig_LoadDefault (void) :
		CommCB (COMM_CBCODE_SYSTEM_CONFIG_LOAD_DEFAULT)
	{}
};

/******************************************************************/
/******************************************************************/

class CommCB_VM2_SetRange : public CommCB
{
public:
	CommCB_VM2_SetRange (Comm_VM2_Range range) :
		CommCB (COMM_CBCODE_VM2_SET_RANGE),
		range_ (range)
	{}

public:
	Comm_VM2_Range range (void) const { return range_; }

private:
	Comm_VM2_Range range_;
};

class CommCB_VM2_GetCalibration : public CommCB
{
public:
	CommCB_VM2_GetCalibration (uint16_t index) :
		CommCB (COMM_CBCODE_VM2_GET_CALIBRATION),
		index_ (index)
	{}

public:
	uint16_t index (void) const { return index_; }

private:
	uint16_t index_;
};

class CommCB_VM2_SetCalibration : public CommCB
{
public:
	CommCB_VM2_SetCalibration (uint16_t index, float voltage) :
		CommCB   (COMM_CBCODE_VM2_SET_CALIBRATION),
		index_   (index),
		voltage_ (voltage)
	{}

public:
	uint16_t index   (void) const { return index_;   }
	float    voltage (void) const { return voltage_; }

private:
	uint16_t index_;
	float    voltage_;
};

class CommCB_VM2_SaveCalibration : public CommCB
{
public:
	CommCB_VM2_SaveCalibration (void) :
		CommCB (COMM_CBCODE_VM2_SAVE_CALIBRATION)
	{}
};

class CommCB_VM2_Read : public CommCB
{
public:
	CommCB_VM2_Read (uint16_t filterLength) :
		CommCB (COMM_CBCODE_VM2_READ),
		filterLength_ (filterLength)
	{}

public:
	uint16_t filterLength (void) const { return filterLength_; }

private:
	uint16_t filterLength_;
};

class CommCB_VM2_LoadDefaultCalibration : public CommCB
{
public:
	CommCB_VM2_LoadDefaultCalibration (void) :
		CommCB (COMM_CBCODE_VM2_LOAD_DEFAULT_CALIBRATION)
	{}
};

/******************************************************************/
/******************************************************************/

class CommCB_VM_SetTerminal : public CommCB
{
public:
	CommCB_VM_SetTerminal (Comm_VM_Terminal terminal) :
		CommCB (COMM_CBCODE_VM_SET_TERMINAL),
		terminal_ (terminal)
	{}

public:
	Comm_VM_Terminal terminal (void) const { return terminal_; }

private:
	Comm_VM_Terminal terminal_;
};

/******************************************************************/

class CommCB_VM_GetTerminal : public CommCB
{
public:
	CommCB_VM_GetTerminal (void) :
		CommCB (COMM_CBCODE_VM_GET_TERMINAL)
	{}
};

/******************************************************************/
/******************************************************************/

class CommCB_changeBaud : public CommCB
{
public:
	CommCB_changeBaud (uint16_t baudRate) :
		CommCB (COMM_CBCODE_CHANGE_BAUD),
		baudRate_ (baudRate)
	{}

public:
	uint32_t baudRate (void) const { return baudRate_; }

private:
	uint32_t baudRate_;
};

/******************************************************************/

class CommCB_recSize : public CommCB
{
public:
	CommCB_recSize (uint32_t recSize) :
		CommCB (COMM_CBCODE_REC_SIZE),
		recSize_ (recSize)
	{}

public:
	uint32_t recSize (void) const {return recSize_;}

private:
	uint32_t recSize_;
};

/******************************************************************/

class CommCB_recData : public CommCB
{
public:
	CommCB_recData (int32_t *recData) :
		CommCB (COMM_CBCODE_REC_DATA),
		recData_ (recData)
	{}

public:
	int32_t *recData (void) const {return recData_;}

private:
	int32_t *recData_;
};
/******************************************************************/
/******************************************************************/

union CommCB_Union
{
	char gen0[sizeof (CommCB)];
	char gen1[sizeof (CommCB_Identity)];
	char gen2[sizeof (CommCB_keepAlive)];
	char gen3[sizeof (CommCB_SetSourceMode)];
	char gen4[sizeof (CommCB_changeBaud)];
	char gen5[sizeof (CommCB_recSize)];
	char gen6[sizeof (CommCB_recData)];

	char cs0[sizeof (CommCB_CS_SetRange)];
	char cs1[sizeof (CommCB_CS_GetCalibration)];
	char cs2[sizeof (CommCB_CS_VerifyCalibration)];
	char cs3[sizeof (CommCB_CS_SetCalibration)];
	char cs4[sizeof (CommCB_CS_SaveCalibration)];
	char cs5[sizeof (CommCB_CS_SetCurrent)];
	char cs6[sizeof (CommCB_CS_LoadDefaultCalibration)];

	char vs0[sizeof (CommCB_VS_SetRange)];
	char vs1[sizeof (CommCB_VS_GetCalibration)];
	char vs2[sizeof (CommCB_VS_VerifyCalibration)];
	char vs3[sizeof (CommCB_VS_SetCalibration)];
	char vs4[sizeof (CommCB_VS_SaveCalibration)];
	char vs5[sizeof (CommCB_VS_SetVoltage)];
	char vs6[sizeof (CommCB_VS_LoadDefaultCalibration)];

	char cm0[sizeof (CommCB_CM_SetRange)];
	char cm1[sizeof (CommCB_CM_GetCalibration)];
	char cm2[sizeof (CommCB_CM_SetCalibration)];
	char cm3[sizeof (CommCB_CM_SaveCalibration)];
	char cm4[sizeof (CommCB_CM_Read)];
	char cm5[sizeof (CommCB_CM_LoadDefaultCalibration)];

	char vm0[sizeof (CommCB_VM_SetRange)];
	char vm1[sizeof (CommCB_VM_GetCalibration)];
	char vm2[sizeof (CommCB_VM_SetCalibration)];
	char vm3[sizeof (CommCB_VM_SaveCalibration)];
	char vm4[sizeof (CommCB_VM_Read)];
	char vm5[sizeof (CommCB_VM_LoadDefaultCalibration)];

	char rm0[sizeof (CommCB_RM_ReadAutoscale)];

	char sysconf0[sizeof (CommCB_SystemConfig_Get)];
	char sysconf1[sizeof (CommCB_SystemConfig_Set)];
	char sysconf2[sizeof (CommCB_SystemConfig_Save)];
	char sysconf3[sizeof (CommCB_SystemConfig_LoadDefault)];

	char vm2_0[sizeof (CommCB_VM2_SetRange)];
	char vm2_1[sizeof (CommCB_VM2_GetCalibration)];
	char vm2_2[sizeof (CommCB_VM2_SetCalibration)];
	char vm2_3[sizeof (CommCB_VM2_SaveCalibration)];
	char vm2_4[sizeof (CommCB_VM2_Read)];
	char vm2_5[sizeof (CommCB_VM2_LoadDefaultCalibration)];

	char vm7[sizeof (CommCB_VM_SetTerminal)];
	char vm8[sizeof (CommCB_VM_GetTerminal)];
};

/******************************************************************/
/******************************************************************/

class Comm : public Applet
{
public:
	static Comm& _ (void);

public:
	virtual void check (void);

	void transmitIdentity (const char* identity,
						   uint32_t hardware_version,
						   uint32_t firmware_version);

	void transmit_nop (void);

	void transmit_keepAlive (uint32_t leaseTime_ms);

	void transmitSourceMode (Comm_SourceMode mode);

	/********************************/

	void transmit_CS_setRange            (Comm_CS_Range range);

	void transmit_CS_getCalibration      (
		uint16_t index, int16_t dac, float current);

	void transmit_CS_verifyCalibration   (
		uint16_t index, int16_t dac, float current);

	void transmit_CS_setCalibration      (
		uint16_t index, int16_t dac, float current);

	void transmit_CS_saveCalibration     (void);
	void transmit_CS_setCurrent          (float current);

	/********************************/

	void transmit_VS_setRange            (Comm_VS_Range range);

	void transmit_VS_getCalibration      (
		uint16_t index, int16_t dac, float voltage);

	void transmit_VS_verifyCalibration   (
		uint16_t index, int16_t dac, float voltage);

	void transmit_VS_setCalibration      (
		uint16_t index, int16_t dac, float voltage);

	void transmit_VS_saveCalibration     (void);
	void transmit_VS_setVoltage          (float voltage);

	/********************************/

	void transmit_CM_setRange            (Comm_CM_Range range);
	void transmit_CM_getCalibration      (
		uint16_t index, int32_t adc, float current);

	void transmit_CM_setCalibration      (
		uint16_t index, int32_t adc, float current);

	void transmit_CM_saveCalibration     (void);
	void transmit_CM_read                (float reading);

	/********************************/

	void transmit_VM_setRange            (Comm_VM_Range range);
	void transmit_VM_getCalibration      (
		uint16_t index, int32_t adc, float voltage);

	void transmit_VM_setCalibration      (
		uint16_t index, int32_t adc, float voltage);

	void transmit_VM_saveCalibration     (void);
	void transmit_VM_read                (float reading);

	/********************************/

	void transmit_CS_loadDefaultCalibration (void);
	void transmit_VS_loadDefaultCalibration (void);
	void transmit_CM_loadDefaultCalibration (void);
	void transmit_VM_loadDefaultCalibration (void);

	/********************************/

	void transmit_RM_readAutoscale (float reading);

	/********************************/

	void transmit_SystemConfig_Get (uint16_t paramID, int16_t value);
	void transmit_SystemConfig_Set (uint16_t paramID, int16_t value);
	void transmit_SystemConfig_Save (void);
	void transmit_SystemConfig_LoadDefault (void);

	/********************************/

	void transmit_VM2_setRange (Comm_VM2_Range range);
	void transmit_VM2_getCalibration (
		uint16_t index, int32_t adc, float voltage);
	void transmit_VM2_setCalibration (
		uint16_t index, int32_t adc, float voltage);
	void transmit_VM2_saveCalibration (void);
	void transmit_VM2_read (float reading);
	void transmit_VM2_loadDefaultCalibration (void);

	void transmit_VM_setTerminal (Comm_VM_Terminal terminal);
	void transmit_VM_getTerminal (Comm_VM_Terminal terminal);

	/********************************/

	void transmit_changeBaud (uint32_t baudRate);

	void transmit_recSize (uint32_t recSize);
	void transmit_recData (int32_t * recData);

	/********************************/

private:
	QP4 qp4_;
	CommCB_Union callbackObject_;

private:
	Comm (void);

private:
	void checkReceiveQueue        (void);

	void processReceivedData      (const void* data, uint16_t size);
	void interpret                (const void* data, uint16_t size);

	void nopCB                    (const void* data, uint16_t size);
	void identityCB               (const void* data, uint16_t size);
	void keepAliveCB              (const void* data, uint16_t size);

	void setSourceModeCB          (const void* data, uint16_t size);

	void CS_setRangeCB            (const void* data, uint16_t size);
	void CS_getCalibrationCB      (const void* data, uint16_t size);
	void CS_verifyCalibrationCB   (const void* data, uint16_t size);
	void CS_setCalibrationCB      (const void* data, uint16_t size);
	void CS_saveCalibrationCB     (const void* data, uint16_t size);
	void CS_setCurrentCB          (const void* data, uint16_t size);

	void VS_setRangeCB            (const void* data, uint16_t size);
	void VS_getCalibrationCB      (const void* data, uint16_t size);
	void VS_verifyCalibrationCB   (const void* data, uint16_t size);
	void VS_setCalibrationCB      (const void* data, uint16_t size);
	void VS_saveCalibrationCB     (const void* data, uint16_t size);
	void VS_setVoltageCB          (const void* data, uint16_t size);

	void CM_setRangeCB            (const void* data, uint16_t size);
	void CM_getCalibrationCB      (const void* data, uint16_t size);
	void CM_setCalibrationCB      (const void* data, uint16_t size);
	void CM_saveCalibrationCB     (const void* data, uint16_t size);
	void CM_readCB                (const void* data, uint16_t size);

	void VM_setRangeCB            (const void* data, uint16_t size);
	void VM_getCalibrationCB      (const void* data, uint16_t size);
	void VM_setCalibrationCB      (const void* data, uint16_t size);
	void VM_saveCalibrationCB     (const void* data, uint16_t size);
	void VM_readCB                (const void* data, uint16_t size);

	void CS_loadDefaultCalibrationCB (const void* data, uint16_t size);
	void VS_loadDefaultCalibrationCB (const void* data, uint16_t size);
	void CM_loadDefaultCalibrationCB (const void* data, uint16_t size);
	void VM_loadDefaultCalibrationCB (const void* data, uint16_t size);

	void RM_readAutoscaleCB (const void* data, uint16_t size);

	void SystemConfig_GetCB         (const void* data, uint16_t size);
	void SystemConfig_SetCB         (const void* data, uint16_t size);
	void SystemConfig_SaveCB        (const void* data, uint16_t size);
	void SystemConfig_LoadDefaultCB (const void* data, uint16_t size);

	void VM2_setRangeCB               (const void* data, uint16_t size);
	void VM2_getCalibrationCB         (const void* data, uint16_t size);
	void VM2_setCalibrationCB         (const void* data, uint16_t size);
	void VM2_saveCalibrationCB        (const void* data, uint16_t size);
	void VM2_readCB                   (const void* data, uint16_t size);
	void VM2_loadDefaultCalibrationCB (const void* data, uint16_t size);

	void VM_setTerminalCB (const void* data, uint16_t size);
	void VM_getTerminalCB (const void* data, uint16_t size);

	void changeBaudCB (const void* data, uint16_t size);

	void recSizeCB (const void* data, uint16_t size);
	void recDataCB (const void* data, uint16_t size);

private:
	void transmit (const QP4_Packet* packet);

public:
	bool isBaudValid (uint32_t baudRate);
	void setBaudRate (uint32_t baudRate);

};

/******************************************************************/

#define appComm    Comm::_()

#endif
