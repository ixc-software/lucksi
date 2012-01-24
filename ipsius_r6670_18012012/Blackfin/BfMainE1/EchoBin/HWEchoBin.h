#ifndef _HW_ECHO_BIN_H_
#define _HW_ECHO_BIN_H_

#include "Platform/Platform.h"

namespace EchoBin
{
	struct EchoBinData
	{
		EchoBinData(Platform::dword size,
			const Platform::byte *data) :
			Size(size),
			Data(data)
		{}
		const Platform::dword Size;
		const Platform::byte *Data;
	};
	
	EchoBinData GetEchoBinData();
};

#endif
