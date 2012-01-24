#include "stdafx.h"

#include <drivers/adi_dev.h>
#include <drivers/uart/adi_uart.h>
#include "Platform/Platform.h"
#include "Utils/ErrorsSubsystem.h"

#include "BfDev/AdiDevice.h"
#include "BfDev/AdiDeviceCommand.h"
#include "UartReturnCode.h"
#include "BfUartProfile.h"
#include "BfUartManagement.h"

namespace iUart
{
	using namespace BfDev;
	
	using Platform::byte;

    
    BfUartManagement::BfUartManagement(
        AdiDevice &device, 
    	const BfUartProfile &profile) :
        m_device(device)
    {
        UartReturnCode result = m_device.Open();
        if (!result.isSuccess())
        {
            ESS_THROW_MSG(InitializationFailed, result.ToString());
        }

        AdiDeviceCommandsTable table;
        profile.FillCommandTable(table);
        result = m_device.RunCommand(table); 
        if (!result.isSuccess())
        {
            ESS_THROW_MSG(InitializationFailed, result.ToString());
        }
    }

    BfUartManagement::~BfUartManagement()
    {
    	Dataflow(false);
	}
            
    UartReturnCode BfUartManagement::Open()
    {
        return m_device.Open();
    }

    UartReturnCode BfUartManagement::Close()
    {
        return m_device.Close();
    }

    UartReturnCode BfUartManagement::SetBoudRate(int rate)
    {
        AdiDeviceCommand command(ADI_UART_CMD_SET_BAUD_RATE, rate);
        return m_device.RunCommand(command);
    }

    UartReturnCode BfUartManagement::RunAutobaud(char c)
    {
        AdiDeviceCommandsTable table;
        table.Add(AdiDeviceCommand(ADI_UART_CMD_AUTOBAUD));
        table.Add(AdiDeviceCommand(ADI_UART_CMD_SET_AUTOBAUD_CHAR, c));
        return m_device.RunCommand(table);
    }

    UartReturnCode  BfUartManagement::RunAutobaud(int divisorBits)
    {
        AdiDeviceCommandsTable table;
        table.Add(AdiDeviceCommand(ADI_UART_CMD_AUTOBAUD));
        table.Add(AdiDeviceCommand(ADI_UART_CMD_SET_DIVISOR_BITS, divisorBits));
        return m_device.RunCommand(table);
    }

    UartReturnCode BfUartManagement::EnableAutocomplete(char c)
    {
        AdiDeviceCommand command(ADI_UART_CMD_ENABLE_AUTO_COMPLETE, c);
        return m_device.RunCommand(command);
    }

    UartReturnCode BfUartManagement::DisableAutocomplete()
    {
        AdiDeviceCommand command(ADI_UART_CMD_DISABLE_AUTO_COMPLETE);
        return m_device.RunCommand(command);
    }

    UartReturnCode BfUartManagement::Dataflow(bool par)
    {
        AdiDeviceCommand command(ADI_DEV_CMD_SET_DATAFLOW, par);
        return m_device.RunCommand(command);
    }
    
    UartReturnCode BfUartManagement::DataflowChained()
    {
        AdiDeviceCommand command(ADI_DEV_CMD_SET_DATAFLOW_METHOD, ADI_DEV_MODE_CHAINED);
        return m_device.RunCommand(command);
    }

    UartReturnCode BfUartManagement::DataflowLoopback()
    {
		AdiDeviceCommand command(ADI_DEV_CMD_SET_DATAFLOW_METHOD, ADI_DEV_MODE_CHAINED_LOOPBACK);
        return m_device.RunCommand(command);
    }

    UartReturnCode BfUartManagement::EnableOddParityCheck()
    {
        AdiDeviceCommandsTable table;
        table.Add(AdiDeviceCommand(ADI_UART_CMD_ENABLE_PARITY, true));
        table.Add(AdiDeviceCommand(ADI_UART_CMD_SET_PARITY, false));
        return m_device.RunCommand(table);
    }

    UartReturnCode BfUartManagement::EnableEvenParityCheck()
    {
        AdiDeviceCommandsTable table;
        table.Add(AdiDeviceCommand(ADI_UART_CMD_ENABLE_PARITY, true));
        table.Add(AdiDeviceCommand(ADI_UART_CMD_SET_PARITY, true));
        return m_device.RunCommand(table);
    }

    UartReturnCode BfUartManagement::DisableParityCheck()
    {
        AdiDeviceCommand command(ADI_UART_CMD_ENABLE_PARITY, false);
        return m_device.RunCommand(command);
    }
} // namespace BfDev

