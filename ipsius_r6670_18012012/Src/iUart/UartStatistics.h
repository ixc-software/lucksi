#ifndef _UART_STATISTIC_H_
#define _UART_STATISTIC_H_

#include "stdafx.h"
#include "Platform/PlatformTypes.h"

namespace iUart
{
	class UartStatistics 
	{
	public:
		UartStatistics()
		{
			Clear();
		} 

		void Clear()
		{
			m_packReceived = 0;
			m_byteReceived = 0;
		
			m_packSend = 0;
			m_byteSend = 0;
		}
			
		Platform::ddword m_packReceived;
		Platform::ddword m_byteReceived;
		
		Platform::ddword m_packSend;
		Platform::ddword m_byteSend;
	};
}

#endif

