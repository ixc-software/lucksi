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
	        virtual iUart::IUartIO &Uart() = 0;  	  // ������ �� ��������� UART
	        virtual const iUart::IUartIO &Uart() const = 0;  	  // ������ �� ��������� UART
	        virtual bool isSlaveReady() const = 0;	  // ���������� ������������ � ������ ������    	
	        
    };
        
    //------------------------------------------------------------------------------            
    
    /*
	 ����� ��������� ��� �������� ������������ ���� � ��������� BF537
	 ����� UART.
	 ����� ��������� ���� BF537 ������ ���� ��������� ������� ������� RESET.
	 ����� �������� ���������� ������ ���������� �������� ������� ������
	 �� �������� LoadRequest(), ������� ������ ������� true, ����� BF537
	 �� ����� � ������ �������� ����. 
	 ��������� ������� ������������:
	 	������������ ����� �������� BF537 BMODE (���������);
	 	�� ���� RESET`�;
	 	����������� UART ����������;
	 	�������;
	 	 
	 ����� ��������� LoadRequest() ���������� ���������� � �������� ��������������� 
	 ������������ ���� � ������� ������ LoadBlock(byte *data, dword count);
	 ������������ ������ ����� ��������� ������ ����� ��������� count.
	 ���������� ������ ��� �������� ������������.
	 
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







