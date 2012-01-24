#ifndef __SPI_LOADER__
#define __SPI_LOADER__

#include "stdafx.h"
#include "Platform/PlatformTypes.h"
#include "Utils/TimerTicks.h"
#include "DevIpTdm/DevIpTdmSpiMng.h"

namespace BfDev
{
    using namespace Platform;    
    
    //------------------------------------------------------------------------------            
    
    /*
	 ����� ��������� ��� �������� ������������ ���� � ��������� BF537
	 ����� SPI.
	 ����� ��������� ���� BF537 ������ ���� ��������� ������� ������� RESET.
	 	 
	 �������� ������������ ���� ������������ � ������� ������ LoadBlock(byte *data, dword count);
	 ������������ ������ ����� ��������� ������ ����� ��������� count.
	 ���������� ������ ��� �������� ������������.
	 
	*/	
    
	class BfSpiLoader : boost::noncopyable
	{
				
		public:
		    BfSpiLoader(BfDev::BfSpi &spi, BfDev::IBfSpiBusLock &echoLoaderBusLock);
			~BfSpiLoader();
		    
		    bool LoadBlock(const byte *data, dword count);		    
	
        
	    private:
            bool WaitSlaveReady();           
            void ResetEcho();            
			bool isSlaveReady() const;

           	BfDev::BfSpi &m_spi;
            BfDev::IBfSpiBusLock &m_busLock;
	    	Utils::TimerTicks	m_timer;	    	

	};
	
}; //namespace BfDev

#endif







