#ifndef __UART_LOADER__
#define __UART_LOADER__

#include "stdafx.h"
#include "Platform/platformtypes.h"
#include "Utils/TimerTicks.h"
#include "iUart/IUartIO.h"

namespace BfDev
{
    using namespace Platform;    
    
    //------------------------------------------------------------------------------

    class IBfUartLoaderIO : public Utils::IBasicInterface
    {    	
    	public: 
	        virtual iUart::IUartIO &Uart() = 0;  	  // ссылка на интерфейс UART
	        virtual const iUart::IUartIO &Uart() const = 0;  	  // ссылка на интерфейс UART
	        virtual bool isSlaveReady() const = 0;	  // готовность прошиваемого к приему данных    	
	        
    };
        
    //------------------------------------------------------------------------------            
    
    /*
	 Класс необходим для загрузки исполняемого кода в процессор BF537
	 через UART.
	 Перед загрузкой кода BF537 должен быть аппаратно сброшен кнопкой RESET.
	 После создания экземпляра класса необходимо единожды послать запрос
	 на загрузку LoadRequest(), который должен вернуть true, иначе BF537
	 не готов к началу загрузки кода. 
	 Возможные причины неготовности:
	 	Неправильный режим загрузки BF537 BMODE (аппаратно);
	 	Не было RESET`а;
	 	Отсутствует UART соединение;
	 	Питание;
	 	 
	 После успешного LoadRequest() необходимо приступить к загрузке непосредственно 
	 исполняемого кода с помощью метода LoadBlock(byte *data, dword count);
	 Максимальный размер блока ограничен только типом параметра count.
	 Количество блоков для загрузки неограничено.
	 
	*/	
    
	class BfUartLoader : boost::noncopyable
	{
				
		public:
		    BfUartLoader(IBfUartLoaderIO &loaderIO);
		    
		    bool LoadRequest();
		    void LoadBlock(byte *data, dword count);		    
	
        
	    private:
            void WaitSlaveReady();

	    	IBfUartLoaderIO &m_loaderIO;    	
	    	Utils::TimerTicks	m_timer;
	    	
	    	bool	m_slaveLoadIsReady;	    		    	

	};
	
}; //namespace BfDev

#endif







