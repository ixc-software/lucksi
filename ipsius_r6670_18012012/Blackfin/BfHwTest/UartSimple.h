#ifndef __UART__
#define __UART__

#include "macros.h"


	void Uart0Init();
	void Uart1Init();	
    void Send0Msg(const char *msg);
    void Send1Msg(const char *msg);    
    byte ReadByte(byte portNum);
    bool ReadyData(byte portNum);   

#endif







