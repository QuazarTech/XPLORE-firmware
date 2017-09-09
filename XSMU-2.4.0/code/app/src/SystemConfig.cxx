#include "app/SystemConfig.h"
#include "app/Storage.h"

SystemConfig& SystemConfig::_ (void)
{
	static SystemConfig o;
	return o;
}

SystemConfig::SystemConfig (void)
{
	read();
}

void SystemConfig::read (void)
{
	if (storage.read (STORAGE_FILENO_SYSTEM_CONFIG, this) !=
	sizeof (SystemConfig))
		fillDefault();
}

void SystemConfig::write (void)
{
	storage.write (STORAGE_FILENO_SYSTEM_CONFIG, this);
}

void SystemConfig::fillDefault (void)
{
	hwBoardNo_ = 4;
	hwBomNo_   = 0;

	hwBugfixNo_ = 0;
	reserved3_ = 0;

	reserved4_ = 0;
	reserved5_ = 0;

	reserved6_ = 0;
	reserved7_ = 0;

	reserved8_ = 0;
	reserved9_ = 0;

	reserved10_ = 0;
	reserved11_ = 0;

	reserved12_ = 0;
	reserved13_ = 0;

	reserved14_ = 0;
	reserved15_ = 0;
}
