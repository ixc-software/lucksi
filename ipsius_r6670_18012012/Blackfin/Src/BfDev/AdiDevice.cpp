#include "stdafx.h"
#include "AdiDeviceCommand.h"
#include "AdiDeviceManager.h"
#include "AdiDevice.h"

namespace BfDev
{	

	AdiDevice::AdiDevice(u32 number,
		AdiDeviceManager &deviceManager, 
		const AdiDeviceSettings &settings) :
        m_isOpen(false),
        m_deviceManager(deviceManager),
        m_deviceNumber(number),
        m_settings(settings)
    {
        static_cast<Utils::IVirtualDestroyOwner*>(&m_deviceManager)->Add(this);
        ESS_ASSERT(m_settings.m_clientCallback != 0);
    }
    
    //-------------------------------------------------------------
    
    AdiDevice::~AdiDevice()
    {
        if (!IsOpened()) Close();
        
		static_cast<Utils::IVirtualDestroyOwner*>(&m_deviceManager)->Delete(this);        
    }   

    //-------------------------------------------------------------

    AdiDeviceResult AdiDevice::Open()
    {
    	ESS_ASSERT(!IsOpened());    	

        AdiDeviceResult result = adi_dev_Open(m_deviceManager.Handle(), 
            m_settings.m_pEntryPoint, m_deviceNumber, m_settings.m_clientHandle,
            &m_deviceHandle, m_settings.m_direction, m_settings.m_DMAHandle,
            m_settings.m_DCBHandle, m_settings.m_clientCallback);
        
        if (result.isSuccess())	m_isOpen = true;

        return result;
    }

    //-------------------------------------------------------------

    AdiDeviceResult AdiDevice::Close()
    {
    	ESS_ASSERT(IsOpened());


        AdiDeviceResult result= adi_dev_Close(m_deviceHandle);
        
        if (result.isSuccess())	m_isOpen = false;
        
        return result;
    }

    //-------------------------------------------------------------

    AdiDeviceResult AdiDevice::Read(ADI_DEV_BUFFER_TYPE bufferType, 
        ADI_DEV_BUFFER *pBuffer) 
    {
        ESS_ASSERT(IsOpened());
        
        return adi_dev_Read(m_deviceHandle, bufferType, pBuffer);
    }

    //-------------------------------------------------------------

    AdiDeviceResult AdiDevice::Write(ADI_DEV_BUFFER_TYPE bufferType, 
        ADI_DEV_BUFFER *pBuffer)
    {
        ESS_ASSERT(IsOpened());
        
        return adi_dev_Write(m_deviceHandle, bufferType, pBuffer);
    }

    //-------------------------------------------------------------

    AdiDeviceResult AdiDevice::RunCommand(AdiDeviceCommand &command)
    {
        return adi_dev_Control(m_deviceHandle, ADI_DEV_CMD_PAIR, command.Command());
    }

    //-------------------------------------------------------------

    AdiDeviceResult AdiDevice::RunCommand(AdiDeviceCommandsTable &commands) 
    {
    	ESS_ASSERT(commands.IsCorrect());

        return adi_dev_Control(m_deviceHandle, ADI_DEV_CMD_TABLE, commands.Table());
    }

    //-------------------------------------------------------------

    AdiDeviceResult AdiDevice::SynchronousOn()
    {
        return adi_dev_Control(m_deviceHandle, ADI_DEV_CMD_SET_SYNCHRONOUS, (void *)TRUE);
    }

    //-------------------------------------------------------------

    AdiDeviceResult AdiDevice::SynchronousOff()
    {
        return adi_dev_Control(m_deviceHandle, ADI_DEV_CMD_SET_SYNCHRONOUS, (void *)FALSE);
    }

    //-------------------------------------------------------------

    AdiDeviceResult AdiDevice::DataFlowOn()
    {
        return adi_dev_Control(m_deviceHandle, ADI_DEV_CMD_SET_DATAFLOW, (void *)TRUE);
    }

    //-------------------------------------------------------------

    AdiDeviceResult AdiDevice::DataFlowOff()
    {
        return adi_dev_Control(m_deviceHandle, ADI_DEV_CMD_SET_DATAFLOW, (void *)FALSE);
    }

    //-------------------------------------------------------------

    AdiDeviceResult AdiDevice::ErrorReportingOn()
    {
        return adi_dev_Control(m_deviceHandle, ADI_DEV_CMD_SET_ERROR_REPORTING, (void *)TRUE);
    }

    //-------------------------------------------------------------

    AdiDeviceResult AdiDevice::ErrorReportingOff()
    {
        return adi_dev_Control(m_deviceHandle, ADI_DEV_CMD_SET_ERROR_REPORTING, (void *)FALSE);
    }
};




