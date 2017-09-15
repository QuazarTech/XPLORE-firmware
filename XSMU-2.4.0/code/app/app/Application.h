#ifndef __APPLICATION__
#define __APPLICATION__

#include "sys/Applet.h"
#include "app/CS.h"
#include "app/VS.h"
#include "app/CM.h"
#include "app/VM.h"
#include "app/Comm.h"
#include "sys/SysTick.h"

class Application : public Applet
{
public:
	static Application& _ (void);
	void run (void);

public:
	virtual void check (void);

public:
	bool
	is_online (void) const { return online_; }

	bool
	is_offline (void) const { return not online_; }

private:
	Application (void);
	void show_banner (void);

private:
	void freezeLocalDisplay (void);
	bool localDisplayFrozen (void);

private:
	static void appCommCB         (void* user_data, const void* vCB);
	void appCommCB                (const CommCB* oCB);

	void nopCB                    (const CommCB* oCB);
	void identityCB               (const CommCB* oCB);
	void keepAliveCB              (const CommCB* oCB);

	void setSourceModeCB          (const CommCB* oCB);

	void CS_setRangeCB            (const CommCB* oCB);
	void CS_getCalibrationCB      (const CommCB* oCB);
	void CS_verifyCalibrationCB   (const CommCB* oCB);
	void CS_setCalibrationCB      (const CommCB* oCB);
	void CS_saveCalibrationCB     (const CommCB* oCB);
	void CS_setCurrentCB          (const CommCB* oCB);

	void VS_setRangeCB            (const CommCB* oCB);
	void VS_getCalibrationCB      (const CommCB* oCB);
	void VS_verifyCalibrationCB   (const CommCB* oCB);
	void VS_setCalibrationCB      (const CommCB* oCB);
	void VS_saveCalibrationCB     (const CommCB* oCB);
	void VS_setVoltageCB          (const CommCB* oCB);

	void CM_setRangeCB            (const CommCB* oCB);
	void CM_getCalibrationCB      (const CommCB* oCB);
	void CM_setCalibrationCB      (const CommCB* oCB);
	void CM_saveCalibrationCB     (const CommCB* oCB);
	void CM_readCB                (const CommCB* oCB);

	void VM_setRangeCB            (const CommCB* oCB);
	void VM_getCalibrationCB      (const CommCB* oCB);
	void VM_setCalibrationCB      (const CommCB* oCB);
	void VM_saveCalibrationCB     (const CommCB* oCB);
	void VM_readCB                (const CommCB* oCB);

	void CS_loadDefaultCalibrationCB (const CommCB* oCB);
	void VS_loadDefaultCalibrationCB (const CommCB* oCB);
	void CM_loadDefaultCalibrationCB (const CommCB* oCB);
	void VM_loadDefaultCalibrationCB (const CommCB* oCB);

	void RM_readAutoscaleCB (const CommCB* oCB);

	void SystemConfig_GetCB (const CommCB* oCB);
	void SystemConfig_SetCB (const CommCB* oCB);
	void SystemConfig_SaveCB (const CommCB* oCB);
	void SystemConfig_LoadDefaultCB (const CommCB* oCB);

	void VM2_setRangeCB            (const CommCB* oCB);
	void VM2_getCalibrationCB      (const CommCB* oCB);
	void VM2_setCalibrationCB      (const CommCB* oCB);
	void VM2_saveCalibrationCB     (const CommCB* oCB);
	void VM2_readCB                (const CommCB* oCB);
	void VM2_loadDefaultCalibrationCB (const CommCB* oCB);

	void VM_setTerminalCB (const CommCB* oCB);
	void VM_getTerminalCB (const CommCB* oCB);

	void changeBaudCB     (const CommCB* oCB);

private:
	void CS_activate (void);
	void CS_deactivate (void);
	void VS_activate (void);
	void VS_deactivate (void);

private:
	void displayCalibrationSaved (void);

	void displaySourceMode (bool CS_Active, bool VS_Active);

	void displaySourceRange (bool CS_Active, CS_Range IRange,
							 bool VS_Active, VS_Range VRange);

	void displaySourceSetpoint (
		bool CS_Active, float I, CS_Range IRange,
		bool VS_Active, float V, VS_Range VRange);

	void displayMeterRange (CM_Range IRange, VM_Range VRange);

	void displayIV (bool CM_Active, float I, CM_Range IRange,
					bool VM_Active, float V, VM_Range VRange);

private:
	void
	go_online (uint32_t lease_time_ms);

	void
	go_offline (void);

	void
	check_alive (void);

private:
	bool displayFrozen_;
	SysTick::tick_t displayResumeAt_;

private:
	bool online_;
	SysTick::tick_t offline_at_;
};

#define app    Application::_()

#endif
