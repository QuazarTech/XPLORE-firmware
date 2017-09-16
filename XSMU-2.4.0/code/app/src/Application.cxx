#include "app/Application.h"
#include "app/CS.h"
#include "app/VS.h"
#include "app/CM.h"
#include "app/VM.h"
#include "app/VM2.h"
#include "app/RM.h"
#include "app/Comm.h"
#include "app/LEDDisplay.h"
#include "app/firmware_version.h"
#include "app/SystemConfig.h"

#include "sys/LCD.h"
#include "sys/hardware.h"

#include <util/delay.h>
#include <avr/interrupt.h>

#include "pgmspace"

using namespace std;

Application::Application (void) :
	displayFrozen_   (false),
	displayResumeAt_ (0),
	online_          (false),
	offline_at_      (0)
{
	lcd;   // Creates LCD singleton
	modCS; // Creates current source singleton
	modVS; // Creates voltage source singleton
	modCM; // Creates current meter  singleton
	modVM; // Creates voltage meter  singleton
	modVM2; // Creates second voltmeter singleton
	modRM; // Creates ohm meter singleton
	appComm; // Creates communication applet

	CS_activate();
	modVS.setVoltage(0);
	modVM.setTerminal (VM_TERMINAL_MEASUREMENT);

	appComm.callback (appCommCB, this);

	show_banner();

	sei();
	freezeLocalDisplay();
}

void Application::show_banner (void)
{
	char str[32];
	LCD_FmtFlags flags = lcd.fmtflags();

	lcd.cursorAt (0, 0);
	lcd << strcpy_P (str, PSTR ("I-V Source Meter"));

#ifdef FIRMWARE_RELEASED
#define FIRMWARE_VERSION_PROMPT "F/w ver "
#else
#define FIRMWARE_VERSION_PROMPT "F/w dev "
#endif

	lcd.cursorAt (1, 0);
	lcd << strcpy_P (str, PSTR (FIRMWARE_VERSION_PROMPT))
		<< setw (2) << zerofill
			<< MAJOR_VERSION_NO (FIRMWARE_VERSION) << setw (1) << "."
		<< setw (2) << zerofill
			<< MINOR_VERSION_NO (FIRMWARE_VERSION) << setw (1) << "."
		<< setw (2) << zerofill
			<< BUGFIX_VERSION_NO (FIRMWARE_VERSION);

	lcd.fmtflags (flags);
}

Application& Application::_ (void)
{
	static Application o;
	return o;
}

void Application::run (void)
{
	Applet::run();
}

/*************************************************************************/
/*************************************************************************/

void Application::appCommCB (void* user_data, const void* vCB)
{
	reinterpret_cast<Application*> (user_data)->appCommCB (
		reinterpret_cast<const CommCB*> (vCB));
}

void Application::appCommCB (const CommCB* oCB)
{
	typedef void (Application::*cb_t)(const CommCB* oCB);

	static const cb_t cbs[] PROGMEM =
	{
		&Application::nopCB,
		&Application::identityCB,
		&Application::keepAliveCB,

		&Application::setSourceModeCB,

		&Application::CS_setRangeCB,
		&Application::CS_getCalibrationCB,
		&Application::CS_verifyCalibrationCB,
		&Application::CS_setCalibrationCB,
		&Application::CS_saveCalibrationCB,
		&Application::CS_setCurrentCB,

		&Application::VS_setRangeCB,
		&Application::VS_getCalibrationCB,
		&Application::VS_verifyCalibrationCB,
		&Application::VS_setCalibrationCB,
		&Application::VS_saveCalibrationCB,
		&Application::VS_setVoltageCB,

		&Application::CM_setRangeCB,
		&Application::CM_getCalibrationCB,
		&Application::CM_setCalibrationCB,
		&Application::CM_saveCalibrationCB,
		&Application::CM_readCB,

		&Application::VM_setRangeCB,
		&Application::VM_getCalibrationCB,
		&Application::VM_setCalibrationCB,
		&Application::VM_saveCalibrationCB,
		&Application::VM_readCB,

		&Application::CS_loadDefaultCalibrationCB,
		&Application::VS_loadDefaultCalibrationCB,
		&Application::CM_loadDefaultCalibrationCB,
		&Application::VM_loadDefaultCalibrationCB,

		&Application::RM_readAutoscaleCB,

		&Application::SystemConfig_GetCB,
		&Application::SystemConfig_SetCB,
		&Application::SystemConfig_SaveCB,
		&Application::SystemConfig_LoadDefaultCB,

		&Application::VM2_setRangeCB,
		&Application::VM2_getCalibrationCB,
		&Application::VM2_setCalibrationCB,
		&Application::VM2_saveCalibrationCB,
		&Application::VM2_readCB,
		&Application::VM2_loadDefaultCalibrationCB,

		&Application::VM_setTerminalCB,
		&Application::VM_getTerminalCB,

		&Application::changeBaudCB,
	};

	if (oCB->code() < sizeof (cbs) / sizeof (cbs[0]))
		(this->*pgm_read (cbs[oCB->code()]))(oCB);
}

/*************************************************************************/

void Application::nopCB (const CommCB* oCB)
{}

void Application::identityCB (const CommCB* oCB)
{
	const uint32_t hardware_version =
		MAKE_VERSION (system_config.hwBoardNo(),
					  system_config.hwBomNo(),
					  system_config.hwBugfixNo());

	appComm.transmitIdentity ("XPLORE SMU",
							  hardware_version, FIRMWARE_VERSION);
}

void Application::keepAliveCB (const CommCB* oCB)
{
	const CommCB_keepAlive* o =
		reinterpret_cast<const CommCB_keepAlive*> (oCB);

	uint32_t lease_time_ms = o->leaseTime_ms();
	go_online (lease_time_ms);
	appComm.transmit_keepAlive (lease_time_ms);
}

void Application::setSourceModeCB (const CommCB* oCB)
{
	const CommCB_SetSourceMode* o =
		reinterpret_cast<const CommCB_SetSourceMode*> (oCB);

	switch (o->mode())
	{
		case COMM_SOURCE_MODE_CURRENT:
			VS_deactivate();
			CS_activate();
			appComm.transmitSourceMode (COMM_SOURCE_MODE_CURRENT);
			break;

		case COMM_SOURCE_MODE_VOLTAGE:
			CS_deactivate();
			VS_activate();
			appComm.transmitSourceMode (COMM_SOURCE_MODE_VOLTAGE);
			break;
	}

// 	freezeLocalDisplay();
// 	displaySourceMode (modCS.active(), modVS.active());
}

/*************************************************************************/

void Application::CS_setRangeCB (const CommCB* oCB)
{
	const CommCB_CS_SetRange* o =
		reinterpret_cast<const CommCB_CS_SetRange*> (oCB);

	modCM.setRange (toCM_Range (o->range()));
	appComm.transmit_CM_setRange (toComm_CM_Range (modCM.range()));

	modCS.setRange (toCS_Range (o->range()));
	appComm.transmit_CS_setRange (toComm_CS_Range (modCS.range()));

// 	freezeLocalDisplay();
// 	displaySourceRange (modCS.active(), modCS.range(),
// 						modVS.active(), modVS.range());
}

void Application::CS_getCalibrationCB (const CommCB* oCB)
{
	const CommCB_CS_GetCalibration* o =
		reinterpret_cast<const CommCB_CS_GetCalibration*> (oCB);

	uint8_t index = o->index();
	const CS_CalibrationTable& calibration = modCS.calibration();

	appComm.transmit_CS_getCalibration (index,
										calibration[index].first(),
										calibration[index].second());
}

void Application::CS_verifyCalibrationCB (const CommCB* oCB)
{
	const CommCB_CS_VerifyCalibration* o =
		reinterpret_cast<const CommCB_CS_VerifyCalibration*> (oCB);

	uint8_t index = o->index();
	modCS.verifyCalibration (index);

	const CS_CalibrationTable& calibration = modCS.calibration();
	appComm.transmit_CS_verifyCalibration (index,
										   calibration[index].first(),
										   calibration[index].second());
}

void Application::CS_setCalibrationCB (const CommCB* oCB)
{
	const CommCB_CS_SetCalibration* o =
		reinterpret_cast<const CommCB_CS_SetCalibration*> (oCB);

	uint8_t index = o->index();
	modCS.setCalibration (index, o->current());

	const CS_CalibrationTable& calibration = modCS.calibration();
	appComm.transmit_CS_setCalibration (index,
										calibration[index].first(),
										calibration[index].second());
}

void Application::CS_saveCalibrationCB (const CommCB* oCB)
{
	modCS.saveCalibration();
	appComm.transmit_CS_saveCalibration();

	freezeLocalDisplay();
	displayCalibrationSaved();
}

void Application::CS_setCurrentCB (const CommCB* oCB)
{
	const CommCB_CS_SetCurrent* o =
		reinterpret_cast<const CommCB_CS_SetCurrent*> (oCB);

	modCS.setCurrent (o->current());
	appComm.transmit_CS_setCurrent (modCS.current());

// 	freezeLocalDisplay();
// 	displaySourceSetpoint (modCS.active(), modCS.current(), modCS.range(),
// 						   modVS.active(), modVS.voltage(), modVS.range());
}

/*************************************************************************/

void Application::VS_setRangeCB (const CommCB* oCB)
{
	const CommCB_VS_SetRange* o =
		reinterpret_cast<const CommCB_VS_SetRange*> (oCB);

	modVS.setRange (toVS_Range (o->range()));
	appComm.transmit_VS_setRange (toComm_VS_Range (modVS.range()));

// 	freezeLocalDisplay();
// 	displaySourceRange (modCS.active(), modCS.range(),
// 						modVS.active(), modVS.range());
}

void Application::VS_getCalibrationCB (const CommCB* oCB)
{
	const CommCB_VS_GetCalibration* o =
		reinterpret_cast<const CommCB_VS_GetCalibration*> (oCB);

	uint8_t index = o->index();
	const VS_CalibrationTable& calibration = modVS.calibration();

	appComm.transmit_VS_getCalibration (index,
										calibration[index].first(),
										calibration[index].second());
}

void Application::VS_verifyCalibrationCB (const CommCB* oCB)
{
	const CommCB_VS_VerifyCalibration* o =
		reinterpret_cast<const CommCB_VS_VerifyCalibration*> (oCB);

	uint8_t index = o->index();
	modVS.verifyCalibration (index);

	const VS_CalibrationTable& calibration = modVS.calibration();
	appComm.transmit_VS_verifyCalibration (index,
										   calibration[index].first(),
										   calibration[index].second());
}

void Application::VS_setCalibrationCB (const CommCB* oCB)
{
	const CommCB_VS_SetCalibration* o =
		reinterpret_cast<const CommCB_VS_SetCalibration*> (oCB);

	uint8_t index = o->index();
	modVS.setCalibration (index, o->voltage());

	const VS_CalibrationTable& calibration = modVS.calibration();
	appComm.transmit_VS_setCalibration (index,
										calibration[index].first(),
										calibration[index].second());
}

void Application::VS_saveCalibrationCB (const CommCB* oCB)
{
	modVS.saveCalibration();
	appComm.transmit_VS_saveCalibration();

	freezeLocalDisplay();
	displayCalibrationSaved();
}

void Application::VS_setVoltageCB (const CommCB* oCB)
{

	const CommCB_VS_SetVoltage* o =
		reinterpret_cast<const CommCB_VS_SetVoltage*> (oCB);

	modVS.setVoltage (o->voltage());
	appComm.transmit_VS_setVoltage (modVS.voltage());

// 	freezeLocalDisplay();
// 	displaySourceSetpoint (modCS.active(), modCS.current(), modCS.range(),
// 						   modVS.active(), modVS.voltage(), modVS.range());
}

/*************************************************************************/

void Application::CM_setRangeCB (const CommCB* oCB)
{
	const CommCB_CM_SetRange* o =
		reinterpret_cast<const CommCB_CM_SetRange*> (oCB);

	modCS.setRange (toCS_Range (o->range()));
	appComm.transmit_CS_setRange (toComm_CS_Range (modCS.range()));

	modCM.setRange (toCM_Range (o->range()));
	appComm.transmit_CM_setRange (toComm_CM_Range (modCM.range()));

// 	freezeLocalDisplay();
// 	displayMeterRange (modCM.range(), modVM.range());
}

void Application::CM_getCalibrationCB (const CommCB* oCB)
{
	const CommCB_CM_GetCalibration* o =
		reinterpret_cast<const CommCB_CM_GetCalibration*> (oCB);

	uint8_t index = o->index();
	const CM_CalibrationTable& calibration = modCM.calibration();

	appComm.transmit_CM_getCalibration (index,
										calibration[index].first(),
										calibration[index].second());
}

void Application::CM_setCalibrationCB (const CommCB* oCB)
{
	const CommCB_CM_SetCalibration* o =
		reinterpret_cast<const CommCB_CM_SetCalibration*> (oCB);

	uint8_t index = o->index();
	modCM.setCalibration (index, o->current());

	const CM_CalibrationTable& calibration = modCM.calibration();
	appComm.transmit_CM_setCalibration (index,
										calibration[index].first(),
										calibration[index].second());
}

void Application::CM_saveCalibrationCB (const CommCB* oCB)
{
	modCM.saveCalibration();
	appComm.transmit_CM_saveCalibration();

	freezeLocalDisplay();
	displayCalibrationSaved();
}

void Application::CM_readCB (const CommCB* oCB)
{
	modLED.VM_activate();

	const CommCB_CM_Read* o =
		reinterpret_cast<const CommCB_CM_Read*> (oCB);

	float current = modCM.readCurrent(o->filterLength());
	appComm.transmit_CM_read (current);

	freezeLocalDisplay();
	displayIV (true,  current, modCM.range(),
			   false, 0.0,     modVM.range());

	modLED.VM_deactivate();
}

/*************************************************************************/

void Application::VM_setRangeCB (const CommCB* oCB)
{
	const CommCB_VM_SetRange* o =
		reinterpret_cast<const CommCB_VM_SetRange*> (oCB);

	modVM.setRange (toVM_Range (o->range()));
	appComm.transmit_VM_setRange (toComm_VM_Range (modVM.range()));

// 	freezeLocalDisplay();
// 	displayMeterRange (modCM.range(), modVM.range());
}

void Application::VM_getCalibrationCB (const CommCB* oCB)
{
	const CommCB_VM_GetCalibration* o =
		reinterpret_cast<const CommCB_VM_GetCalibration*> (oCB);

	uint8_t index = o->index();
	const VM_CalibrationTable& calibration = modVM.calibration();

	appComm.transmit_VM_getCalibration (index,
										calibration[index].first(),
										calibration[index].second());
}

void Application::VM_setCalibrationCB (const CommCB* oCB)
{
	const CommCB_VM_SetCalibration* o =
		reinterpret_cast<const CommCB_VM_SetCalibration*> (oCB);

	uint8_t index = o->index();
	modVM.setCalibration (index, o->voltage());

	const VM_CalibrationTable& calibration = modVM.calibration();
	appComm.transmit_VM_setCalibration (index,
										calibration[index].first(),
										calibration[index].second());
}

void Application::VM_saveCalibrationCB (const CommCB* oCB)
{
	modVM.saveCalibration();
	appComm.transmit_VM_saveCalibration();

	freezeLocalDisplay();
	displayCalibrationSaved();
}

void Application::VM_readCB (const CommCB* oCB)
{
	modLED.VM_activate();

	const CommCB_VM_Read* o =
		reinterpret_cast<const CommCB_VM_Read*> (oCB);

	const float voltage = modVM.readVoltage(o->filterLength());
	appComm.transmit_VM_read (voltage);

	freezeLocalDisplay();
	displayIV (false, 0.0,     modCM.range(),
			   true,  voltage, modVM.range());

	modLED.VM_deactivate();
}

/*************************************************************************/

void Application::CS_loadDefaultCalibrationCB (const CommCB* oCB)
{
	modCS.fillDefaultCalibration();
	appComm.transmit_CS_loadDefaultCalibration();
}

void Application::VS_loadDefaultCalibrationCB (const CommCB* oCB)
{
	modVS.fillDefaultCalibration();
	appComm.transmit_VS_loadDefaultCalibration();
}

void Application::CM_loadDefaultCalibrationCB (const CommCB* oCB)
{
	modCM.fillDefaultCalibration();
	appComm.transmit_CM_loadDefaultCalibration();
}

void Application::VM_loadDefaultCalibrationCB (const CommCB* oCB)
{
	modVM.fillDefaultCalibration();
	appComm.transmit_VM_loadDefaultCalibration();
}

/*************************************************************************/
/*************************************************************************/

void Application::RM_readAutoscaleCB (const CommCB* oCB)
{
	VS_deactivate();
	CS_activate();

	const CommCB_RM_ReadAutoscale* o =
		reinterpret_cast<const CommCB_RM_ReadAutoscale*> (oCB);

	const float R = modRM.readResistance (o->filterLength());
	appComm.transmit_RM_readAutoscale (R);
}

/*************************************************************************/
/*************************************************************************/

void Application::SystemConfig_GetCB (const CommCB* oCB)
{
	const CommCB_SystemConfig_Get* o =
		reinterpret_cast <const CommCB_SystemConfig_Get*> (oCB);

	const int16_t paramID = o->paramID();

	switch (paramID)
	{
		case COMM_SYSTEM_CONFIG_PARAM_ID_HW_BOARD_NO:
			appComm.transmit_SystemConfig_Get (
				paramID, system_config.hwBoardNo());
			break;

		case COMM_SYSTEM_CONFIG_PARAM_ID_HW_BOM_NO:
			appComm.transmit_SystemConfig_Get (
				paramID, system_config.hwBomNo());
			break;

		case COMM_SYSTEM_CONFIG_PARAM_ID_HW_BUGFIX_NO:
			appComm.transmit_SystemConfig_Get (
				paramID, system_config.hwBugfixNo());
			break;
	};
}

void Application::SystemConfig_SetCB (const CommCB* oCB)
{
	const CommCB_SystemConfig_Set* o =
		reinterpret_cast <const CommCB_SystemConfig_Set*> (oCB);

	const int16_t paramID = o->paramID();
	const int16_t value   = o->value();

	switch (paramID)
	{
		case COMM_SYSTEM_CONFIG_PARAM_ID_HW_BOARD_NO:
			system_config.hwBoardNo (value);
			break;

		case COMM_SYSTEM_CONFIG_PARAM_ID_HW_BOM_NO:
			system_config.hwBomNo (value);
			break;

		case COMM_SYSTEM_CONFIG_PARAM_ID_HW_BUGFIX_NO:
			system_config.hwBugfixNo (value);
			break;
	};

	switch (paramID)
	{
		case COMM_SYSTEM_CONFIG_PARAM_ID_HW_BOARD_NO:
			appComm.transmit_SystemConfig_Set (
				paramID, system_config.hwBoardNo());
			break;

		case COMM_SYSTEM_CONFIG_PARAM_ID_HW_BOM_NO:
			appComm.transmit_SystemConfig_Set (
				paramID, system_config.hwBomNo());
			break;

		case COMM_SYSTEM_CONFIG_PARAM_ID_HW_BUGFIX_NO:
			appComm.transmit_SystemConfig_Set (
				paramID, system_config.hwBugfixNo());
			break;
	};
}

void Application::SystemConfig_SaveCB (const CommCB* oCB)
{
	system_config.write();
	appComm.transmit_SystemConfig_Save();
}

void Application::SystemConfig_LoadDefaultCB (const CommCB* oCB)
{
	system_config.fillDefault();
	appComm.transmit_SystemConfig_LoadDefault();
}

/*************************************************************************/

void Application::VM2_setRangeCB (const CommCB* oCB)
{
	const CommCB_VM2_SetRange* o =
		reinterpret_cast<const CommCB_VM2_SetRange*> (oCB);

	modVM2.setRange (toVM2_Range (o->range()));
	appComm.transmit_VM2_setRange (toComm_VM2_Range (modVM2.range()));
}

void Application::VM2_getCalibrationCB (const CommCB* oCB)
{
	const CommCB_VM2_GetCalibration* o =
		reinterpret_cast<const CommCB_VM2_GetCalibration*> (oCB);

	uint8_t index = o->index();
	const VM_CalibrationTable& calibration = modVM2.calibration();

	appComm.transmit_VM2_getCalibration (index,
										calibration[index].first(),
										calibration[index].second());
}

void Application::VM2_setCalibrationCB (const CommCB* oCB)
{
	const CommCB_VM2_SetCalibration* o =
		reinterpret_cast<const CommCB_VM2_SetCalibration*> (oCB);

	uint8_t index = o->index();
	modVM2.setCalibration (index, o->voltage());

	const VM_CalibrationTable& calibration = modVM2.calibration();
	appComm.transmit_VM2_setCalibration (index,
										calibration[index].first(),
										calibration[index].second());
}

void Application::VM2_saveCalibrationCB (const CommCB* oCB)
{
	modVM2.saveCalibration();
	appComm.transmit_VM2_saveCalibration();

	freezeLocalDisplay();
	displayCalibrationSaved();
}

void Application::VM2_readCB (const CommCB* oCB)
{
	modLED.VM_activate();

	const CommCB_VM2_Read* o =
		reinterpret_cast<const CommCB_VM2_Read*> (oCB);

	const float voltage = modVM2.readVoltage(o->filterLength());
	appComm.transmit_VM2_read (voltage);

	modLED.VM_deactivate();
}

void Application::VM2_loadDefaultCalibrationCB (const CommCB* oCB)
{
	modVM2.fillDefaultCalibration();
	appComm.transmit_VM2_loadDefaultCalibration();
}

/*************************************************************************/

void Application::VM_setTerminalCB (const CommCB* oCB)
{
	const CommCB_VM_SetTerminal* o =
		reinterpret_cast<const CommCB_VM_SetTerminal*> (oCB);

	modVM.setTerminal (toVM_Terminal (o->terminal()));
	appComm.transmit_VM_setTerminal (toComm_VM_Terminal (modVM.getTerminal()));
}

void Application::VM_getTerminalCB (const CommCB* oCB)
{
	// const CommCB_VM_GetTerminal* o =
	// 	reinterpret_cast<const CommCB_VM_GetTerminal*> (oCB);

	appComm.transmit_VM_getTerminal (
		toComm_VM_Terminal (modVM.getTerminal()));
}

/*************************************************************************/

void Application::changeBaudCB (const CommCB* oCB)
{
	const CommCB_changeBaud* o =
		reinterpret_cast<const CommCB_changeBaud*> (oCB);

	uint32_t baudRate = o->baudRate();

	if (appComm.isBaudValid (baudRate))
		appComm.transmit_changeBaud (baudRate);

	else
		appComm.transmit_nop();

	appComm.setBaudRate (baudRate);
}

/*************************************************************************/
/*************************************************************************/

void Application::freezeLocalDisplay (void)
{
	displayFrozen_ = true;
	displayResumeAt_ = systick.get() + systick.time_to_tick (2);
}

/************************************************************************/

bool Application::localDisplayFrozen (void)
{
	if ((displayFrozen_) && (systick.get() >= displayResumeAt_))
			displayFrozen_ = false;

	return displayFrozen_;
}

/************************************************************************/
/************************************************************************/

void Application::check_alive (void)
{
	if (online_ && (systick.get() >= offline_at_))
		go_offline();
}

/************************************************************************/

void
Application::go_online (uint32_t lease_time_ms)
{
	online_ = true;

	offline_at_ = systick.get() +
		systick.time_to_tick (lease_time_ms * 1e-3);
}

/************************************************************************/

void Application::go_offline (void)
{
	online_ = false;
//	appComm.restore_default_baudrate();
}

/************************************************************************/
/************************************************************************/

void Application::check (void)
{
	check_alive();

	modCM.check();
	modVM.check();

	if (modCM.data_ready() && modVM.data_ready()) {

		const float I = modCM.readCurrent();
		const float V = modVM.readVoltage();

		if (!localDisplayFrozen())
			displayIV (true, I, modCM.range(), true, V, modVM.range());
	}
}

/************************************************************************/
/************************************************************************/

void Application::CS_activate (void)
{
	modCS.activate();
	modLED.CS_activate();
}

/************************************************************************/

void Application::CS_deactivate (void)
{
	modCS.deactivate();
	modLED.CS_deactivate();
}

/************************************************************************/
/************************************************************************/

void Application::VS_activate (void)
{
	modVS.activate();
	modLED.VS_activate();
}

/************************************************************************/

void Application::VS_deactivate (void)
{
	modVS.deactivate();
	modLED.VS_deactivate();
}

/************************************************************************/
/************************************************************************/

void Application::displayCalibrationSaved (void)
{
	lcd.cursorAt (0, 0);
	lcd << "  Calibration   ";

	lcd.cursorAt (1, 0);
	lcd << "     saved      ";
}

/************************************************************************/

void Application::displaySourceMode (bool CS_Active, bool VS_Active)
{
	LCD_FmtFlags flags = lcd.fmtflags();

	lcd.cursorAt (0, 0);

	lcd << left << setw (2) << (CS_Active ? "*" : "")
		<< right << setw (14) << "Current source";

	lcd.cursorAt (1, 0);

	lcd << left << setw (2) << (VS_Active ? "*" : "")
		<< right << setw (14) << "Voltage source";

	lcd.fmtflags (flags);
}

/************************************************************************/
/************************************************************************/

void Application::displaySourceRange (bool CS_Active, CS_Range IRange,
									  bool VS_Active, VS_Range VRange)
{
	static const char* const IRanges[] PROGMEM =
	{
		"10uA", "100uA", "1mA", "10mA", "100mA"
	};

	static const char* const VRanges[] PROGMEM =
	{
		"10V", "100V"
	};

	const char* IRangeStr = CS_Active ?
		pgm_read (IRanges[IRange]) : "OFF";

	const char* VRangeStr = VS_Active ?
		pgm_read (VRanges[VRange]) : "OFF";

	LCD_FmtFlags flags = lcd.fmtflags();

	lcd.cursorAt (0, 0);
	lcd << left << setw (9) << "CS range:"
			<< right << setw (7) << IRangeStr;

	lcd.cursorAt (1, 0);
	lcd << left << setw (9) << "VS range:"
			<< right << setw (7) << VRangeStr;

	lcd.fmtflags (flags);
}

/************************************************************************/

void Application::displaySourceSetpoint (
	bool CS_Active, float I, CS_Range IRange,
	bool VS_Active, float V, VS_Range VRange)
{
	struct Unit
	{
		const char* unit;
		float multiplier;
		uint8_t precision;
	};

	static const Unit currentUnits[] PROGMEM =
	{
		{"uA", 1e6, 4},  // CS_RANGE_10uA
		{"uA", 1e6, 3},  // CS_RANGE_100uA
		{"mA", 1e3, 5},  // CS_RANGE_1mA
		{"mA", 1e3, 4},  // CS_RANGE_10mA
		{"mA", 1e3, 3},  // CS_RANGE_100mA
	};

	static const Unit voltageUnits[] PROGMEM =
	{
		{"V", 1e0, 4}, // VS_RANGE_10V
		{"V", 1e0, 3}, // VS_RANGE_100V
	};

	const Unit& currentUnit = currentUnits[IRange];
	const Unit& voltageUnit = voltageUnits[VRange];

	LCD_FmtFlags flags = lcd.fmtflags();

	lcd.cursorAt (0, 0);

	if (CS_Active)
		lcd << left << setw (2) << "I:"
			<< right << setw (12)
				<< setprecision (pgm_read (currentUnit.precision))
				<< I * pgm_read (currentUnit.multiplier)
			<< left << setw (2) << pgm_read (currentUnit.unit);
	else
		lcd << left << setw (2) << "I:"
			<< right << setw (14) << "OFF";

	lcd.cursorAt (1, 0);

	if (VS_Active)
		lcd << left << setw (2) << "V:"
			<< right << setw (12)
				<< setprecision (pgm_read (voltageUnit.precision))
				<< V * pgm_read (voltageUnit.multiplier)
			<< left << setw (2) << pgm_read (voltageUnit.unit);
	else
		lcd << left << setw (2) << "V:"
			<< right << setw (14) << "OFF";

	lcd.fmtflags (flags);
}

/************************************************************************/

void Application::displayMeterRange (CM_Range IRange, VM_Range VRange)
{
	static const char* const IRanges[] PROGMEM =
	{
		"10uA", "100uA", "1mA", "10mA", "100mA"
	};

	static const char* const VRanges[] PROGMEM =
	{
		"1mV", "10mV", "100mV", "1V", "10V", "100V"
	};

	LCD_FmtFlags flags = lcd.fmtflags();

	lcd.cursorAt (0, 0);
	lcd << left << setw (9) << "CM range:"
			<< right << setw (7) << pgm_read (IRanges[IRange]);

	lcd.cursorAt (1, 0);
	lcd << left << setw (9) << "VM range:"
			<< right << setw (7) << pgm_read (VRanges[VRange]);

	lcd.fmtflags (flags);
}

/************************************************************************/

void Application::displayIV (bool CM_Active, float I, CM_Range IRange,
							 bool VM_Active, float V, VM_Range VRange)
{
	struct Unit
	{
		const char* unit;
		float multiplier;
		uint8_t precision;
	};

	static const Unit currentUnits[] PROGMEM =
	{
		{"uA", 1e6, 5},  // CM_RANGE_10uA
		{"uA", 1e6, 4},  // CM_RANGE_100uA
		{"mA", 1e3, 6},  // CM_RANGE_1mA
		{"mA", 1e3, 5},  // CM_RANGE_10mA
		{"mA", 1e3, 4},  // CM_RANGE_100mA
	};

	static const Unit voltageUnits[] PROGMEM =
	{
		{"mV", 1e3, 6}, // VM_RANGE_1mV
		{"mV", 1e3, 5}, // VM_RANGE_10mV
		{"mV", 1e3, 4}, // VM_RANGE_100mV
		{"V" , 1e0, 6}, // VM_RANGE_1V
		{"V" , 1e0, 5}, // VM_RANGE_10V
		{"V" , 1e0, 4}, // VM_RANGE_100V
	};

	const Unit& currentUnit = currentUnits[IRange];
	const Unit& voltageUnit = voltageUnits[VRange];

	LCD_FmtFlags flags = lcd.fmtflags();

	lcd.cursorAt (0, 0);

	if (CM_Active)
		lcd << left << setw (4) << "I:"
			<< right << showpos << zerofill << setw (9)
				<< setprecision (pgm_read (currentUnit.precision))
				<< I * pgm_read (currentUnit.multiplier)
			<< setw (1) << " "
			<< left << setw (2) << pgm_read (currentUnit.unit);
// 	else
// 		lcd << left << setw (4) << "I:"
// 			<< right << setw (9) << "OFF";

	lcd.cursorAt (1, 0);
	if (VM_Active)
		lcd << left << setw (4) << "V:"
			<< right << showpos << zerofill << setw (9)
				<< setprecision (pgm_read (voltageUnit.precision))
				<< V * pgm_read (voltageUnit.multiplier)
			<< setw (1) << " "
			<< left << setw (2) << pgm_read (voltageUnit.unit);
// 	else
// 		lcd << left << setw (4) << "V:"
// 			<< right << setw (9) << "OFF";

	lcd.fmtflags (flags);
}

// Resistance

/************************************************************************/

int main (void)
{
	_delay_ms (1000);
	app.run();
	return 0;
}

/************************************************************************/
/************************************************************************/
