#ifndef BOOTFROMSPI_H
#define BOOTFROMSPI_H

#include "Platform/PlatformTypes.h"

namespace DevIpTdm
{
	// ≈сли использовалс€ Lw стек должен быть остановлен!
	void BootFromSpi(Platform::dword spiOffs, Platform::word devider);
}


#endif
