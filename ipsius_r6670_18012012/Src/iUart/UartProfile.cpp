#include "stdafx.h"
#include "qextserialport.h"
#include "Utils/ErrorsSubsystem.h"
#include "UartProfile.h"

namespace iUart
{
    UartProfile::UartProfile(int port,
    	int rate, 
    	DataBits dataBits, 
    	StopBits stopBits,
        ParityType parityType,
        FlowType flowType) :
        m_port(port),
        m_dataBits(dataBits),
        m_stopBits(stopBits),
        m_parityType(parityType),
        m_flowType(flowType),
        m_baudRate(rate),
        m_readWriteTimeoutMs(500)
    {
        Validation();
    }

    // -----------------------------------------------------------------------

    void UartProfile::Validation() const
    {
        ESS_ASSERT(IsValidDataBit(m_dataBits) && "Incorrect numbers of data bits");

        ESS_ASSERT(IsValidStopBit(m_stopBits) && "Incorrect numbers of stop bits");

        ESS_ASSERT( (m_stopBits == StopBits_1 ||
            (m_stopBits == StopBits_1_5 && m_dataBits == DataBits_5) ||
            (m_stopBits == StopBits_2 && m_dataBits != DataBits_5)) && "Uncorrespond numbers data and stop bits");

        ESS_ASSERT(IsValidBaudRate(m_baudRate) && "Unknown stop rate");
    }

    // -----------------------------------------------------------------------

    bool UartProfile::IsValidDataBit(DataBits val)
    {
        return (val == DataBits_5 ||
                val == DataBits_6 ||
                val == DataBits_7 ||
                val == DataBits_8);
    }

    // -----------------------------------------------------------------------

    bool UartProfile::IsValidStopBit(StopBits val)
    {
        return (val == StopBits_1 ||
                val == StopBits_2 ||
                val == StopBits_1_5);
    }

    // -----------------------------------------------------------------------

    bool UartProfile::IsValidBaudRate(int val)
    {
        bool res = true;
        switch(val)
        {
			case 50:
            case 75:
            case 110:
            case 134:
            case 150:
            case 200:
            case 300:
            case 600:
            case 1200:
            case 1800:
            case 2400:
            case 4800:
            case 9600:
            case 14400:
            case 19200:
            case 38400:
            case 56000:
            case 57600:
            case 76800:
            case 115200:
            case 128000:
            case 256000:
			break;
			default:
				res = false;
        }

        return res;
    }

} // namespace Uart


