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
	 Класс необходим для загрузки исполняемого кода в процессор BF537
	 через SPI.
	 Перед загрузкой кода BF537 должен быть аппаратно сброшен кнопкой RESET.
	 	 
	 Загрузка исполняемого кода производится с помощью метода LoadBlock(byte *data, dword count);
	 Максимальный размер блока ограничен только типом параметра count.
	 Количество блоков для загрузки неограничено.
	 
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







