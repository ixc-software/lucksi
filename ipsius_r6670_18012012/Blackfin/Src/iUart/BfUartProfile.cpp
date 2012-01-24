
#include "stdafx.h"
#include <drivers/adi_dev.h>
#include <drivers/uart/adi_uart.h>
#include "BfDev/AdiDeviceCommand.h"
#include "BfUartProfile.h"

namespace iUart
{

	using namespace BfDev;
    BfUartProfile::BfUartProfile( int port, int countBuffers, int sizeBuffer, 
    	int rate, 
    	DataBits dataBits /*= DbFive*/, 
    	StopBits stopBits /*= SbOne*/ ) :
    	m_logIsEnable(false),
        m_port(port),
        m_isSynchronousMode(false),
        m_dataflowIsLoopback(false),
        m_errorReporting(false),
        m_dataBits(dataBits),
        m_stopBits(stopBits),
        m_parityType(iUart::ParityNone),
        m_baudRate(rate),
        m_useDma(false),
        m_isAutoComplete(false),
        m_autoCompleteChar(0),
        m_isCtsRtsControlOn(false),
        m_isCtsRtsLowPolarity(false),
        m_isCtsRtsLowThreshold(true), 
        m_countBuffers(countBuffers),
        m_sizeBuffer(sizeBuffer)
    {
        Validation();
    }

    void BfUartProfile::Validation() const
    {
        ESS_ASSERT((m_dataBits == DataBits_5 ||
            m_dataBits == DataBits_6 ||
            m_dataBits == DataBits_7 ||
            m_dataBits == DataBits_8) && "Incorrect numbers of data bits");

        ESS_ASSERT((m_stopBits == StopBits_1 ||
            m_stopBits == StopBits_2 ||
            m_stopBits == StopBits_1_5) && "Incorrect numbers of stop bits");

        ESS_ASSERT( (m_stopBits == StopBits_1 ||
            (m_stopBits == StopBits_1_5 && m_dataBits == DataBits_5) ||
            (m_stopBits == StopBits_2 && m_dataBits != DataBits_5)) && "Uncorrespond numbers data and stop bits");
            
            
        ESS_ASSERT( (m_parityType == ParityNone ||
        	m_parityType == ParityOdd ||
        	m_parityType == ParityEven) && "Incorrect parity type.");
    }

    void BfUartProfile::FillCommandTable(AdiDeviceCommandsTable &table) const
    {
        
        table.Add(AdiDeviceCommand(ADI_UART_CMD_SET_DATA_BITS, m_dataBits));
        
		table.Add(AdiDeviceCommand(ADI_UART_CMD_ENABLE_PARITY, m_parityType != ParityNone));

		if(m_parityType != ParityNone)
        {
            table.Add(AdiDeviceCommand(ADI_UART_CMD_SET_PARITY, m_parityType == ParityEven));
        }
        
        int stopBits = (m_stopBits == StopBits_1) ? 1 : 2;
        table.Add(AdiDeviceCommand(ADI_UART_CMD_SET_STOP_BITS, stopBits));

        table.Add(AdiDeviceCommand(ADI_UART_CMD_SET_BAUD_RATE, m_baudRate));
        
        table.Add(AdiDeviceCommand(ADI_UART_CMD_SET_LINE_STATUS_EVENTS, TRUE));
        
                
    }

} // namespace BfDev


