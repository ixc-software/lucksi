#ifndef _BF_ADI_DEVICE_RESULT_H_
#define _BF_ADI_DEVICE_RESULT_H_

#include "stdafx.h"
#include <drivers/adi_dev.h>
#include "Platform/PlatformTypes.h"

namespace BfDev
{

	class AdiDeviceResult 
	{
	public:
		AdiDeviceResult(u32 code) :
			m_code(code){}

		bool isSuccess() const 
		{
			return m_code == ADI_DEV_RESULT_SUCCESS;
		}
		Platform::word Code() const
		{
			return m_code;
		}
		static std::string ToString(Platform::word code);
		
		std::string ToString() const
		{
			return ToString(m_code);
		}
	private:
		Platform::word m_code;
	};
};	

#endif
