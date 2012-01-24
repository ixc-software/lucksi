#ifndef _UART0_
#define _UART0_

//--------------------------------------------------------------------------//
// Header files																//
//--------------------------------------------------------------------------//
#include <sys\exception.h>
#include <cdefBF537.h>

//--------------------------------------------------------------------------//
// Prototypes																//
//--------------------------------------------------------------------------//
void Init_UART(unsigned long speed);
int PutChar(const char c);
void UART_SendByte(byte val);
int GetChar(char *const c);
byte ReadCom();
byte UART_ReadByte();
void SendAnswer(byte c);


#endif 

