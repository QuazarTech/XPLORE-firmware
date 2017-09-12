#include "app/Comm.h"
#include "avr/UART.h"

#include <cstdlib>
#include <cstring>
#include "pgmspace"

using namespace std;

Comm_SourceMode toComm_SourceMode (uint16_t i)
{
	static const Comm_SourceMode values[] PROGMEM =
	{
		COMM_SOURCE_MODE_CURRENT,
		COMM_SOURCE_MODE_VOLTAGE
	};

	return pgm_read ((i < sizeof (values) / sizeof (values[0])) ?
		values[i] : values[0]);
}

Comm_CS_Range toComm_CS_Range (uint16_t i)
{
	static const Comm_CS_Range values[] PROGMEM =
	{
		COMM_CS_RANGE_10uA,
		COMM_CS_RANGE_100uA,
		COMM_CS_RANGE_1mA,
		COMM_CS_RANGE_10mA,
		COMM_CS_RANGE_100mA,
	};

	return pgm_read ((i < sizeof (values) / sizeof (values[0])) ?
		values[i] : values[0]);
}

Comm_VS_Range toComm_VS_Range (uint16_t i)
{
	static const Comm_VS_Range values[] PROGMEM =
	{
		COMM_VS_RANGE_10V,
		COMM_VS_RANGE_100V
	};

	return pgm_read ((i < sizeof (values) / sizeof (values[0])) ?
		values[i] : values[0]);
}

Comm_CM_Range toComm_CM_Range (uint16_t i)
{
	static const Comm_CM_Range values[] PROGMEM =
	{
		COMM_CM_RANGE_10uA,
		COMM_CM_RANGE_100uA,
		COMM_CM_RANGE_1mA,
		COMM_CM_RANGE_10mA,
		COMM_CM_RANGE_100mA
	};

	return pgm_read ((i < sizeof (values) / sizeof (values[0])) ?
		values[i] : values[0]);
}

Comm_VM_Range toComm_VM_Range (uint16_t i)
{
	static const Comm_VM_Range values[] PROGMEM =
	{
		COMM_VM_RANGE_1mV,
		COMM_VM_RANGE_10mV,
		COMM_VM_RANGE_100mV,
		COMM_VM_RANGE_1V,
		COMM_VM_RANGE_10V,
		COMM_VM_RANGE_100V
	};

	return pgm_read ((i < sizeof (values) / sizeof (values[0])) ?
		values[i] : values[0]);
}

Comm_VM2_Range toComm_VM2_Range (uint16_t i)
{
	static const Comm_VM2_Range values[] PROGMEM =
	{
		COMM_VM2_RANGE_10V
	};

	return pgm_read ((i < sizeof (values) / sizeof (values[0])) ?
		values[i] : values[0]);
}

Comm_VM_Terminal toComm_VM_Terminal (uint16_t i)
{
	static const Comm_VM_Terminal values[] PROGMEM =
	{
		Comm_VM_Terminal_Measurement,
		Comm_VM_Terminal_Source
	};

	return pgm_read ((i < sizeof (values) / sizeof (values[0])) ?
		values[i] : values[0]);
}

/******************************************************************/

CommResponse_Identity::CommResponse_Identity (const char* identity,
											  uint32_t hardware_version,
											  uint32_t firmware_version) :
	hardware_version_ (std::hton (hardware_version)),
	firmware_version_ (std::hton (firmware_version))
{
	memset (identity_, 0, sizeof (identity_));
	strcpy (identity_, identity);
}

/******************************************************************/
/******************************************************************/

Comm& Comm::_ (void)
{
	static Comm o;
	return o;
}

Comm::Comm (void)
{
	uart.setBaudrate (9600);
}

void Comm::check (void)
{
	checkReceiveQueue();
}

void Comm::transmit (const QP4_Packet* packet)
{
	uart.write (packet, packet->size());
}

/******************************************************************/
/******************************************************************/

void Comm::checkReceiveQueue (void)
{
	const UART_RxBuffer* rx = uart.read();
	processReceivedData (rx->data(), rx->size());
}

void Comm::processReceivedData  (const void* data, uint16_t size)
{
	const uint8_t* src = reinterpret_cast<const uint8_t*> (data);

	if (size) do {

		qp4_.receiver().push_back (*src++);

		if (qp4_.receiver().ready()) {

			pair<const void*, uint16_t> packet =
				qp4_.receiver().data();

			interpret (packet.first, packet.second);
			qp4_.receiver().clear();
		}

	} while (--size);
}

void Comm::interpret (const void* data, uint16_t size)
{
	typedef void (Comm::*cb_t)(const void*, uint16_t);

	static const cb_t cbs[] PROGMEM =
	{
		&Comm::nopCB,                         //00                      
		&Comm::identityCB,                    //01                            
		&Comm::syncCB,                        //02                           
		&Comm::setSourceModeCB,               //03                         
		&Comm::CS_setRangeCB,                 //04                   
		&Comm::CS_getCalibrationCB,           //05                      
		&Comm::CS_verifyCalibrationCB,        //06                        
		&Comm::CS_setCalibrationCB,           //07                       
		&Comm::CS_saveCalibrationCB,          //08                      
		&Comm::CS_setCurrentCB,               //09

		&Comm::VS_setRangeCB,                 //10                    
		&Comm::VS_getCalibrationCB,           //11                       
		&Comm::VS_verifyCalibrationCB,        //12                             
		&Comm::VS_setCalibrationCB,           //13                         
		&Comm::VS_saveCalibrationCB,          //14                      
		&Comm::VS_setVoltageCB,               //15                    
		&Comm::CM_setRangeCB,                 //16                    
		&Comm::CM_getCalibrationCB,           //17                       
		&Comm::CM_setCalibrationCB,           //18                      
		&Comm::CM_saveCalibrationCB,          //19                       
		
		&Comm::CM_readCB,                     //20                
		&Comm::VM_setRangeCB,                 //21                   
		&Comm::VM_getCalibrationCB,           //22                      
		&Comm::VM_setCalibrationCB,           //23                      
		&Comm::VM_saveCalibrationCB,          //24                     
		&Comm::VM_readCB,                     //25               
		&Comm::CS_loadDefaultCalibrationCB,   //26                  
		&Comm::VS_loadDefaultCalibrationCB,   //27                  
		&Comm::CM_loadDefaultCalibrationCB,   //28                        
		&Comm::VM_loadDefaultCalibrationCB,   //29                        
		
		&Comm::RM_readAutoscaleCB,            //30               
		&Comm::SystemConfig_GetCB,            //31               
		&Comm::SystemConfig_SetCB,            //32               
		&Comm::SystemConfig_SaveCB,           //33               
		&Comm::SystemConfig_LoadDefaultCB,    //34                  
		&Comm::VM2_setRangeCB,                //35           
		&Comm::VM2_getCalibrationCB,          //36                
		&Comm::VM2_setCalibrationCB,          //37               
		&Comm::VM2_saveCalibrationCB,         //38            
		&Comm::VM2_readCB,                    //39            
		
		&Comm::VM2_loadDefaultCalibrationCB,  //40
		&Comm::VM_setTerminalCB,              //41                   
		&Comm::VM_getTerminalCB,              //42
                
                &Comm::changeBaudCB,                  //43
	};

	if (size < sizeof (CommPacket))
		return;

	const CommPacket *packet =
	    reinterpret_cast<const CommPacket *> (data);

	if (packet->opcode() < sizeof (cbs) / sizeof (cbs[0]))
		(this->*pgm_read (cbs[packet->opcode()]))(data, size);
}

/******************************************************************/

void Comm::nopCB (const void*, uint16_t)
{}

void Comm::identityCB (const void* , uint16_t)
{
	do_callback (new (&callbackObject_) CommCB_Identity);
}

void Comm::syncCB (const void* , uint16_t)
{
	do_callback (new (&callbackObject_) CommCB_Sync);

	/*************************/

	QP4_Packet* response =
	    qp4_.transmitter().alloc_packet (sizeof (CommResponse_Synchronize));

	new (response->body()) CommResponse_Synchronize;
	response->seal();

	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::setSourceModeCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_SetSourceMode))
		return;

	const CommRequest_SetSourceMode* req =
		reinterpret_cast<const CommRequest_SetSourceMode*> (data);

	do_callback (new (&callbackObject_) CommCB_SetSourceMode (req->mode()));
}

/******************************************************************/

void Comm::CS_setRangeCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_CS_SetRange))
		return;

	const CommRequest_CS_SetRange* req =
		reinterpret_cast<const CommRequest_CS_SetRange*> (data);

	do_callback (new (&callbackObject_) CommCB_CS_SetRange (req->range()));
}

void Comm::CS_getCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_CS_GetCalibration))
		return;

	const CommRequest_CS_GetCalibration* req =
		reinterpret_cast <const CommRequest_CS_GetCalibration*> (data);

	do_callback (new (&callbackObject_)
		CommCB_CS_GetCalibration (req->index()));
}

void Comm::CS_verifyCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_CS_VerifyCalibration))
		return;

	const CommRequest_CS_VerifyCalibration* req =
		reinterpret_cast <const CommRequest_CS_VerifyCalibration*> (data);

	do_callback (new (&callbackObject_)
		CommCB_CS_VerifyCalibration (req->index()));
}

void Comm::CS_setCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_CS_SetCalibration))
		return;

	const CommRequest_CS_SetCalibration* req =
		reinterpret_cast <const CommRequest_CS_SetCalibration*> (data);

	do_callback (new (&callbackObject_)
		CommCB_CS_SetCalibration (req->index(), req->current()));
}

void Comm::CS_saveCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_CS_SaveCalibration))
		return;

	do_callback (new (&callbackObject_) CommCB_CS_SaveCalibration);
}

void Comm::CS_setCurrentCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_CS_SetCurrent))
		return;

	const CommRequest_CS_SetCurrent* req =
		reinterpret_cast <const CommRequest_CS_SetCurrent*> (data);

	do_callback (new (&callbackObject_)
		CommCB_CS_SetCurrent (req->current()));
}

/******************************************************************/

void Comm::VS_setRangeCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_VS_SetRange))
		return;

	const CommRequest_VS_SetRange* req =
		reinterpret_cast<const CommRequest_VS_SetRange*> (data);

	do_callback (new (&callbackObject_) CommCB_VS_SetRange (req->range()));
}

void Comm::VS_getCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_VS_GetCalibration))
		return;

	const CommRequest_VS_GetCalibration* req =
		reinterpret_cast <const CommRequest_VS_GetCalibration*> (data);

	do_callback (new (&callbackObject_)
		CommCB_VS_GetCalibration (req->index()));
}

void Comm::VS_verifyCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_VS_VerifyCalibration))
		return;

	const CommRequest_VS_VerifyCalibration* req =
		reinterpret_cast <const CommRequest_VS_VerifyCalibration*> (data);

	do_callback (new (&callbackObject_)
		CommCB_VS_VerifyCalibration (req->index()));
}

void Comm::VS_setCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_VS_SetCalibration))
		return;

	const CommRequest_VS_SetCalibration* req =
		reinterpret_cast <const CommRequest_VS_SetCalibration*> (data);

	do_callback (new (&callbackObject_)
		CommCB_VS_SetCalibration (req->index(), req->voltage()));
}

void Comm::VS_saveCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_VS_SaveCalibration))
		return;

	do_callback (new (&callbackObject_) CommCB_VS_SaveCalibration);
}

void Comm::VS_setVoltageCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_VS_SetVoltage))
		return;

	const CommRequest_VS_SetVoltage* req =
		reinterpret_cast <const CommRequest_VS_SetVoltage*> (data);

	do_callback (new (&callbackObject_)
		CommCB_VS_SetVoltage (req->voltage()));
}

/******************************************************************/

void Comm::CM_setRangeCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_CM_SetRange))
		return;

	const CommRequest_CM_SetRange* req =
		reinterpret_cast<const CommRequest_CM_SetRange*> (data);

	do_callback (new (&callbackObject_) CommCB_CM_SetRange (req->range()));
}

void Comm::CM_getCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_CM_GetCalibration))
		return;

	const CommRequest_CM_GetCalibration* req =
		reinterpret_cast <const CommRequest_CM_GetCalibration*> (data);

	do_callback (new (&callbackObject_)
		CommCB_CM_GetCalibration (req->index()));
}

void Comm::CM_setCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_CM_SetCalibration))
		return;

	const CommRequest_CM_SetCalibration* req =
		reinterpret_cast <const CommRequest_CM_SetCalibration*> (data);

	do_callback (new (&callbackObject_)
		CommCB_CM_SetCalibration (req->index(), req->current()));
}

void Comm::CM_saveCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_CM_SaveCalibration))
		return;

	do_callback (new (&callbackObject_) CommCB_CM_SaveCalibration);
}

void Comm::CM_readCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_CM_Read))
		return;

	const CommRequest_CM_Read* req =
		reinterpret_cast <const CommRequest_CM_Read*> (data);

	do_callback (new (&callbackObject_) CommCB_CM_Read (req->filterLength()));
}

/******************************************************************/

void Comm::VM_setRangeCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_VM_SetRange))
		return;

	const CommRequest_VM_SetRange* req =
		reinterpret_cast<const CommRequest_VM_SetRange*> (data);

	do_callback (new (&callbackObject_) CommCB_VM_SetRange (req->range()));
}

void Comm::VM_getCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_VM_GetCalibration))
		return;

	const CommRequest_VM_GetCalibration* req =
		reinterpret_cast <const CommRequest_VM_GetCalibration*> (data);

	do_callback (new (&callbackObject_)
		CommCB_VM_GetCalibration (req->index()));
}

void Comm::VM_setCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_VM_SetCalibration))
		return;

	const CommRequest_VM_SetCalibration* req =
		reinterpret_cast <const CommRequest_VM_SetCalibration*> (data);

	do_callback (new (&callbackObject_)
		CommCB_VM_SetCalibration (req->index(), req->voltage()));
}

void Comm::VM_saveCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_VM_SaveCalibration))
		return;

	do_callback (new (&callbackObject_) CommCB_VM_SaveCalibration);
}

void Comm::VM_readCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_VM_Read))
		return;

	const CommRequest_VM_Read* req =
		reinterpret_cast <const CommRequest_VM_Read*> (data);

	do_callback (new (&callbackObject_) CommCB_VM_Read (req->filterLength()));
}

/******************************************************************/

void Comm::CS_loadDefaultCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_CS_LoadDefaultCalibration))
		return;

	do_callback (new (&callbackObject_) CommCB_CS_LoadDefaultCalibration);
}

void Comm::VS_loadDefaultCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_VS_LoadDefaultCalibration))
		return;

	do_callback (new (&callbackObject_) CommCB_VS_LoadDefaultCalibration);
}

void Comm::CM_loadDefaultCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_CM_LoadDefaultCalibration))
		return;

	do_callback (new (&callbackObject_) CommCB_CM_LoadDefaultCalibration);
}

void Comm::VM_loadDefaultCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_VM_LoadDefaultCalibration))
		return;

	do_callback (new (&callbackObject_) CommCB_VM_LoadDefaultCalibration);
}

/******************************************************************/
/******************************************************************/

void Comm::RM_readAutoscaleCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_RM_ReadAutoscale))
		return;

	const CommRequest_RM_ReadAutoscale* req =
		reinterpret_cast <const CommRequest_RM_ReadAutoscale*> (data);

	do_callback (new (&callbackObject_)
		CommCB_RM_ReadAutoscale (req->filterLength()));
}

/******************************************************************/
/******************************************************************/

void Comm::SystemConfig_GetCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_SystemConfig_Get))
		return;

	const CommRequest_SystemConfig_Get* req =
		reinterpret_cast <const CommRequest_SystemConfig_Get*> (data);

	do_callback (new (&callbackObject_)
		CommCB_SystemConfig_Get (req->paramID()));
}

void Comm::SystemConfig_SetCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_SystemConfig_Set))
		return;

	const CommRequest_SystemConfig_Set* req =
		reinterpret_cast <const CommRequest_SystemConfig_Set*> (data);

	do_callback (new (&callbackObject_)
		CommCB_SystemConfig_Set (req->paramID(), req->value()));
}

void Comm::SystemConfig_SaveCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_SystemConfig_Save))
		return;

	do_callback (new (&callbackObject_) CommCB_SystemConfig_Save);
}

void Comm::SystemConfig_LoadDefaultCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_SystemConfig_LoadDefault))
		return;

	do_callback (new (&callbackObject_) CommCB_SystemConfig_LoadDefault);
}

/******************************************************************/

void Comm::VM2_setRangeCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_VM2_SetRange))
		return;

	const CommRequest_VM2_SetRange* req =
		reinterpret_cast<const CommRequest_VM2_SetRange*> (data);

	do_callback (new (&callbackObject_) CommCB_VM2_SetRange (req->range()));
}

void Comm::VM2_getCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_VM2_GetCalibration))
		return;

	const CommRequest_VM2_GetCalibration* req =
		reinterpret_cast <const CommRequest_VM2_GetCalibration*> (data);

	do_callback (new (&callbackObject_)
		CommCB_VM2_GetCalibration (req->index()));
}

void Comm::VM2_setCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_VM2_SetCalibration))
		return;

	const CommRequest_VM2_SetCalibration* req =
		reinterpret_cast <const CommRequest_VM2_SetCalibration*> (data);

	do_callback (new (&callbackObject_)
		CommCB_VM2_SetCalibration (req->index(), req->voltage()));
}

void Comm::VM2_saveCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_VM2_SaveCalibration))
		return;

	do_callback (new (&callbackObject_) CommCB_VM2_SaveCalibration);
}

void Comm::VM2_readCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_VM2_Read))
		return;

	const CommRequest_VM2_Read* req =
		reinterpret_cast <const CommRequest_VM2_Read*> (data);

	do_callback (new (&callbackObject_) CommCB_VM2_Read (req->filterLength()));
}

void Comm::VM2_loadDefaultCalibrationCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_VM2_LoadDefaultCalibration))
		return;

	do_callback (new (&callbackObject_) CommCB_VM2_LoadDefaultCalibration);
}

/******************************************************************/

void Comm::VM_setTerminalCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_VM_SetTerminal))
		return;

	const CommRequest_VM_SetTerminal* req =
		reinterpret_cast<const CommRequest_VM_SetTerminal*> (data);

	do_callback (new (&callbackObject_) CommCB_VM_SetTerminal (req->terminal()));
}

void Comm::VM_getTerminalCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_VM_GetTerminal))
		return;

	// const CommRequest_VM_GetTerminal* req =
	// 	reinterpret_cast<const CommRequest_VM_GetTerminal*> (data);

	do_callback (new (&callbackObject_) CommCB_VM_GetTerminal ());
}

/******************************************************************/

void Comm::changeBaudCB (const void* data, uint16_t size)
{
	if (size < sizeof (CommRequest_changeBaud))
		return;

	const CommRequest_changeBaud* req =
		reinterpret_cast <const CommRequest_changeBaud*> (data);

	do_callback (new (&callbackObject_)
		CommCB_changeBaud (req->baudRate()));
}

/******************************************************************/
/******************************************************************/

void Comm::transmitIdentity (const char* identity,
							 uint32_t hardware_version,
							 uint32_t firmware_version)
{
	QP4_Packet* response =
	    qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_Identity));

	new (response->body())
		CommResponse_Identity (identity,
			hardware_version, firmware_version);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmitSourceMode (Comm_SourceMode mode)
{
	QP4_Packet* response =
	    qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_SetSourceMode));

	new (response->body())
		CommResponse_SetSourceMode (mode);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

/******************************************************************/

void Comm::transmit_CS_setRange (Comm_CS_Range range)
{
	QP4_Packet* response =
	    qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_CS_SetRange));

	new (response->body())
		CommResponse_CS_SetRange (range);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_CS_getCalibration (uint16_t index,
									   int16_t dac,
									   float current)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_CS_GetCalibration));

	new (response->body())
		CommResponse_CS_GetCalibration (index, dac, current);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_CS_verifyCalibration (uint16_t index,
										  int16_t dac,
										  float current)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_CS_VerifyCalibration));

	new (response->body())
		CommResponse_CS_VerifyCalibration (index, dac, current);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_CS_setCalibration (uint16_t index,
									   int16_t dac,
									   float current)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_CS_SetCalibration));

	new (response->body())
		CommResponse_CS_SetCalibration (index, dac, current);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_CS_saveCalibration (void)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_CS_SaveCalibration));

	new (response->body())
		CommResponse_CS_SaveCalibration;

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_CS_setCurrent (float current)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_CS_SetCurrent));

	new (response->body())
		CommResponse_CS_SetCurrent (current);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

/******************************************************************/

void Comm::transmit_VS_setRange (Comm_VS_Range range)
{
	QP4_Packet* response =
	    qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_VS_SetRange));

	new (response->body())
		CommResponse_VS_SetRange (range);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_VS_getCalibration (uint16_t index,
									   int16_t dac,
									   float voltage)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_VS_GetCalibration));

	new (response->body())
		CommResponse_VS_GetCalibration (index, dac, voltage);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_VS_verifyCalibration (uint16_t index,
										  int16_t dac,
										  float voltage)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_VS_VerifyCalibration));

	new (response->body())
		CommResponse_VS_VerifyCalibration (index, dac, voltage);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_VS_setCalibration (uint16_t index,
									   int16_t dac,
									   float voltage)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_VS_SetCalibration));

	new (response->body())
		CommResponse_VS_SetCalibration (index, dac, voltage);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_VS_saveCalibration (void)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_VS_SaveCalibration));

	new (response->body())
		CommResponse_VS_SaveCalibration;

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_VS_setVoltage (float voltage)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_VS_SetVoltage));

	new (response->body())
		CommResponse_VS_SetVoltage (voltage);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

/******************************************************************/

void Comm::transmit_CM_setRange (Comm_CM_Range range)
{
	QP4_Packet* response =
	    qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_CM_SetRange));

	new (response->body())
		CommResponse_CM_SetRange (range);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_CM_getCalibration (uint16_t index,
									   int32_t adc,
									   float current)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_CM_GetCalibration));

	new (response->body())
		CommResponse_CM_GetCalibration (index, adc, current);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_CM_setCalibration (uint16_t index,
									   int32_t adc,
									   float current)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_CM_SetCalibration));

	new (response->body())
		CommResponse_CM_SetCalibration (index, adc, current);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_CM_saveCalibration (void)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_CM_SaveCalibration));

	new (response->body())
		CommResponse_CM_SaveCalibration;

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_CM_read (float reading)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_CM_Read));

	new (response->body())
		CommResponse_CM_Read (reading);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

/******************************************************************/

void Comm::transmit_VM_setRange (Comm_VM_Range range)
{
	QP4_Packet* response =
	    qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_VM_SetRange));

	new (response->body())
		CommResponse_VM_SetRange (range);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_VM_getCalibration (uint16_t index,
									   int32_t adc,
									   float voltage)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_VM_GetCalibration));

	new (response->body())
		CommResponse_VM_GetCalibration (index, adc, voltage);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_VM_setCalibration (uint16_t index,
									   int32_t adc,
									   float voltage)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_VM_SetCalibration));

	new (response->body())
		CommResponse_VM_SetCalibration (index, adc, voltage);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_VM_saveCalibration (void)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_VM_SaveCalibration));

	new (response->body())
		CommResponse_VM_SaveCalibration;

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_VM_read (float reading)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_VM_Read));

	new (response->body())
		CommResponse_VM_Read (reading);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

/******************************************************************/

void Comm::transmit_CS_loadDefaultCalibration (void)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_CS_LoadDefaultCalibration));

	new (response->body())
		CommResponse_CS_LoadDefaultCalibration;

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_VS_loadDefaultCalibration (void)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_VS_LoadDefaultCalibration));

	new (response->body())
		CommResponse_VS_LoadDefaultCalibration;

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_CM_loadDefaultCalibration (void)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_CM_LoadDefaultCalibration));

	new (response->body())
		CommResponse_CM_LoadDefaultCalibration;

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_VM_loadDefaultCalibration (void)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_VM_LoadDefaultCalibration));

	new (response->body())
		CommResponse_VM_LoadDefaultCalibration;

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

/******************************************************************/
/******************************************************************/

void Comm::transmit_RM_readAutoscale (float reading)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_RM_ReadAutoscale));

	new (response->body())
		CommResponse_RM_ReadAutoscale (reading);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

/******************************************************************/
/******************************************************************/

void Comm::transmit_SystemConfig_Get (uint16_t paramID, int16_t value)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_SystemConfig_Get));

	new (response->body()) CommResponse_SystemConfig_Get (paramID, value);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_SystemConfig_Set (uint16_t paramID, int16_t value)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_SystemConfig_Set));

	new (response->body()) CommResponse_SystemConfig_Set (paramID, value);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_SystemConfig_Save (void)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_SystemConfig_Save));

	new (response->body()) CommResponse_SystemConfig_Save;

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_SystemConfig_LoadDefault (void)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_SystemConfig_LoadDefault));

	new (response->body()) CommResponse_SystemConfig_LoadDefault;

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

/******************************************************************/
/******************************************************************/

void Comm::transmit_VM2_setRange (Comm_VM2_Range range)
{
	QP4_Packet* response =
	    qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_VM2_SetRange));

	new (response->body())
		CommResponse_VM2_SetRange (range);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_VM2_getCalibration (uint16_t index,
										int32_t adc,
										float voltage)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_VM2_GetCalibration));

	new (response->body())
		CommResponse_VM2_GetCalibration (index, adc, voltage);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_VM2_setCalibration (uint16_t index,
										int32_t adc,
										float voltage)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_VM2_SetCalibration));

	new (response->body())
		CommResponse_VM2_SetCalibration (index, adc, voltage);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_VM2_saveCalibration (void)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_VM2_SaveCalibration));

	new (response->body())
		CommResponse_VM2_SaveCalibration;

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_VM2_read (float reading)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_VM2_Read));

	new (response->body())
		CommResponse_VM2_Read (reading);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

void Comm::transmit_VM2_loadDefaultCalibration (void)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_VM2_LoadDefaultCalibration));

	new (response->body())
		CommResponse_VM2_LoadDefaultCalibration;

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

/******************************************************************/

void Comm::transmit_VM_setTerminal (Comm_VM_Terminal terminal)
{
	QP4_Packet* response =
	    qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_VM_SetTerminal));

	new (response->body())
		CommResponse_VM_SetTerminal (terminal);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}


void Comm::transmit_VM_getTerminal (Comm_VM_Terminal terminal)
{
	QP4_Packet* response =
	    qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_VM_GetTerminal));

	new (response->body())
		CommResponse_VM_GetTerminal (terminal);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}

/******************************************************************/

void Comm::transmit_changeBaud (uint32_t baudRate)
{
	QP4_Packet* response =
		qp4_.transmitter().alloc_packet (
			sizeof (CommResponse_changeBaud));

	new (response->body())
		CommResponse_changeBaud (baudRate);

	response->seal();
	transmit (response);
	qp4_.transmitter().free_packet (response);
}
/******************************************************************/
/******************************************************************/
