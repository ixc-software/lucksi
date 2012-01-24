#ifndef _UART_CONST_H_
#define _UART_CONST_H_

namespace iUart
{
    enum Direction
    {
        ReadOnly = 0,
        WriteOnly,
        Bidirectional
    };

    // Number of data bits for the constructor
    enum DataBits
    {
        DataBits_5  = 5,
        DataBits_6   = 6,
        DataBits_7 = 7,
        DataBits_8 = 8
    };

    // Number of stop bits for the constructor
    enum StopBits
    {
        StopBits_1 = 1,
        StopBits_2,
        StopBits_1_5
    };
    enum ParityType
    {
        ParityNone,
        ParityOdd,
        ParityEven,
        ParityMark,               //WINDOWS ONLY
        ParitySpace
    };
    enum FlowType {
        FlowOff,
        FlowHardware,
        FlowXonXoff
    };
} // namespace Uart

#endif

