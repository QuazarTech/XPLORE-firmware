#include "app/Storage.h"
#include "app/Calibration.h"
#include "app/SystemConfig.h"
#include "pgmspace"

using namespace std;

Storage_FileNo toStorage_CS_FileNo (CS_Range range)
{
	static const Storage_FileNo fileNos[] PROGMEM =
	{
		STORAGE_FILENO_CS_CALIBRATION_10uA,
		STORAGE_FILENO_CS_CALIBRATION_100uA,
		STORAGE_FILENO_CS_CALIBRATION_1mA,
		STORAGE_FILENO_CS_CALIBRATION_10mA,
		STORAGE_FILENO_CS_CALIBRATION_100mA
	};

	return pgm_read ((range < sizeof (fileNos) / sizeof (fileNos[0])) ?
		fileNos[range] : fileNos[0]);
}

Storage_FileNo toStorage_VS_FileNo (VS_Range range)
{
	static const Storage_FileNo fileNos[] PROGMEM =
	{
		STORAGE_FILENO_VS_CALIBRATION_10V,
		STORAGE_FILENO_VS_CALIBRATION_100V
	};

	return pgm_read ((range < sizeof (fileNos) / sizeof (fileNos[0])) ?
		fileNos[range] : fileNos[0]);
}

Storage_FileNo toStorage_CM_FileNo (CM_Range range)
{
	static const Storage_FileNo fileNos[] PROGMEM =
	{
		STORAGE_FILENO_CM_CALIBRATION_10uA,
		STORAGE_FILENO_CM_CALIBRATION_100uA,
		STORAGE_FILENO_CM_CALIBRATION_1mA,
		STORAGE_FILENO_CM_CALIBRATION_10mA,
		STORAGE_FILENO_CM_CALIBRATION_100mA
	};

	return pgm_read ((range < sizeof (fileNos) / sizeof (fileNos[0])) ?
		fileNos[range] : fileNos[0]);
}

Storage_FileNo toStorage_VM_FileNo (VM_Range range)
{
	static const Storage_FileNo fileNos[] PROGMEM =
	{
		STORAGE_FILENO_VM_CALIBRATION_1mV,
		STORAGE_FILENO_VM_CALIBRATION_10mV,
		STORAGE_FILENO_VM_CALIBRATION_100mV,
		STORAGE_FILENO_VM_CALIBRATION_1V,
		STORAGE_FILENO_VM_CALIBRATION_10V,
		STORAGE_FILENO_VM_CALIBRATION_100V,
	};

	return pgm_read ((range < sizeof (fileNos) / sizeof (fileNos[0])) ?
		fileNos[range] : fileNos[0]);
}

Storage_FileNo toStorage_VM2_FileNo (VM2_Range range)
{
	static const Storage_FileNo fileNos[] PROGMEM =
	{
		STORAGE_FILENO_VM2_CALIBRATION_10V,
	};

	return pgm_read ((range < sizeof (fileNos) / sizeof (fileNos[0])) ?
		fileNos[range] : fileNos[0]);
}

/*********************************************************************/

Storage::Storage (uint16_t size, const uint16_t* file_sizes) :
	FS (size, file_sizes)
{}

Storage& Storage::_ (void)
{
	static const uint16_t file_sizes[] PROGMEM =
	{
		sizeof (CS_CalibrationTable), // STORAGE_FILENO_CS_CALIBRATION_10uA,
		sizeof (CS_CalibrationTable), // STORAGE_FILENO_CS_CALIBRATION_100uA,
		sizeof (CS_CalibrationTable), // STORAGE_FILENO_CS_CALIBRATION_1mA,
		sizeof (CS_CalibrationTable), // STORAGE_FILENO_CS_CALIBRATION_10mA,
		sizeof (CS_CalibrationTable), // STORAGE_FILENO_CS_CALIBRATION_100mA,

		sizeof (VS_CalibrationTable), // STORAGE_FILENO_VS_CALIBRATION_10V,
		sizeof (VS_CalibrationTable), // STORAGE_FILENO_VS_CALIBRATION_100V,

		sizeof (CM_CalibrationTable), // STORAGE_FILENO_CM_CALIBRATION_10uA,
		sizeof (CM_CalibrationTable), // STORAGE_FILENO_CM_CALIBRATION_100uA,
		sizeof (CM_CalibrationTable), // STORAGE_FILENO_CM_CALIBRATION_1mA,
		sizeof (CM_CalibrationTable), // STORAGE_FILENO_CM_CALIBRATION_10mA,
		sizeof (CM_CalibrationTable), // STORAGE_FILENO_CM_CALIBRATION_100mA,

		sizeof (VM_CalibrationTable), // STORAGE_FILENO_VM_CALIBRATION_1mV,
		sizeof (VM_CalibrationTable), // STORAGE_FILENO_VM_CALIBRATION_10mV,
		sizeof (VM_CalibrationTable), // STORAGE_FILENO_VM_CALIBRATION_100mV,
		sizeof (VM_CalibrationTable), // STORAGE_FILENO_VM_CALIBRATION_1V,
		sizeof (VM_CalibrationTable), // STORAGE_FILENO_VM_CALIBRATION_10V,
		sizeof (VM_CalibrationTable), // STORAGE_FILENO_VM_CALIBRATION_100V,

		sizeof (SystemConfig),        //

		sizeof (VM_CalibrationTable), // STORAGE_FILENO_VM2_CALIBRATION_10V,
	};

	static Storage o (
		sizeof (file_sizes) / sizeof (file_sizes[0]), file_sizes);

	return o;
}
