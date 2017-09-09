#ifndef __STORAGE__
#define __STORAGE__

#include "sys/FS.h"
#include "app/CS.h"
#include "app/VS.h"
#include "app/CM.h"
#include "app/VM.h"
#include "app/VM2.h"

enum Storage_FileNo
{
	STORAGE_FILENO_CS_CALIBRATION_10uA,
	STORAGE_FILENO_CS_CALIBRATION_100uA,
	STORAGE_FILENO_CS_CALIBRATION_1mA,
	STORAGE_FILENO_CS_CALIBRATION_10mA,
	STORAGE_FILENO_CS_CALIBRATION_100mA,

	STORAGE_FILENO_VS_CALIBRATION_10V,
	STORAGE_FILENO_VS_CALIBRATION_100V,

	STORAGE_FILENO_CM_CALIBRATION_10uA,
	STORAGE_FILENO_CM_CALIBRATION_100uA,
	STORAGE_FILENO_CM_CALIBRATION_1mA,
	STORAGE_FILENO_CM_CALIBRATION_10mA,
	STORAGE_FILENO_CM_CALIBRATION_100mA,

	STORAGE_FILENO_VM_CALIBRATION_1mV,
	STORAGE_FILENO_VM_CALIBRATION_10mV,
	STORAGE_FILENO_VM_CALIBRATION_100mV,
	STORAGE_FILENO_VM_CALIBRATION_1V,
	STORAGE_FILENO_VM_CALIBRATION_10V,
	STORAGE_FILENO_VM_CALIBRATION_100V,

	STORAGE_FILENO_SYSTEM_CONFIG,

	STORAGE_FILENO_VM2_CALIBRATION_10V,
};

Storage_FileNo toStorage_CS_FileNo (CS_Range range);
Storage_FileNo toStorage_VS_FileNo (VS_Range range);
Storage_FileNo toStorage_CM_FileNo (CM_Range range);
Storage_FileNo toStorage_VM_FileNo (VM_Range range);
Storage_FileNo toStorage_VM2_FileNo (VM2_Range range);

class Storage : public FS
{
public:
	static Storage& _ (void);

private:
	Storage (uint16_t size, const uint16_t* file_sizes);
};

#define storage    Storage::_()

#endif
