#ifndef BOOTFROMSPI_H
#define BOOTFROMSPI_H

#include "Platform/PlatformTypes.h"

namespace DevIpTdm
{
	// ���� ������������� Lw ���� ������ ���� ����������!
	void BootFromSpi(Platform::dword spiOffs, Platform::word devider);
}


#endif
