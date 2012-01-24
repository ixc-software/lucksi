#ifndef _BF_ADI_DEVICE_H_
#define _BF_ADI_DEVICE_H_

#include "stdafx.h"
#include <drivers/adi_dev.h>
#include "Utils/IBasicInterface.h"
#include "Utils/IVirtualDestroy.h"
#include "AdiBufferHelper.h"
#include "AdiDeviceResult.h"


namespace BfDev
{
    class AdiDeviceManager;
	class AdiDeviceCommand;
	class AdiDeviceCommandsTable;
	 
    class AdiDeviceSettings 
    {
	public:
        ADI_DEV_PDD_ENTRY_POINT *m_pEntryPoint; 
        void                    *m_clientHandle;
        ADI_DEV_DIRECTION       m_direction; 
        ADI_DMA_MANAGER_HANDLE  m_DMAHandle; 
        ADI_DCB_HANDLE          m_DCBHandle;
        ADI_DCB_CALLBACK_FN     m_clientCallback;
    };

    class AdiDevice : boost::noncopyable,
    	public Utils::IVirtualDestroy
    {
    public:
		AdiDevice(u32 number,
			AdiDeviceManager &deviceManager, 
			const AdiDeviceSettings &settings);
        
		~AdiDevice();

        AdiDeviceResult Open();
        AdiDeviceResult Close();
        
        template <class Buffer> AdiDeviceResult Read(Buffer &buffer) 
        {
			return Read(AdiBufferHelper<Buffer>::Id, (ADI_DEV_BUFFER*)&buffer);
        }
        template <class Buffer> AdiDeviceResult Write(Buffer &buffer) 
        {
			return Write(AdiBufferHelper<Buffer>::Id, (ADI_DEV_BUFFER*)&buffer);
        }
        
        
        AdiDeviceResult RunCommand(AdiDeviceCommand &command);
        AdiDeviceResult RunCommand(AdiDeviceCommandsTable &commands); 
        
        AdiDeviceResult DataFlowOn();
        AdiDeviceResult DataFlowOff();

        AdiDeviceResult SynchronousOn();
        AdiDeviceResult SynchronousOff();
        
        AdiDeviceResult ErrorReportingOn();
        AdiDeviceResult ErrorReportingOff();

        bool IsOpened() const 
        {
            return m_isOpen;
        }
	private:
        AdiDeviceResult Read(ADI_DEV_BUFFER_TYPE bufferType, 
            ADI_DEV_BUFFER *pBuffer); 

        AdiDeviceResult Write(ADI_DEV_BUFFER_TYPE bufferType, 
            ADI_DEV_BUFFER *pBuffer); 
    private:
        bool m_isOpen;
        AdiDeviceManager &m_deviceManager;
		u32 m_deviceNumber;
        AdiDeviceSettings m_settings;
        ADI_DEV_DEVICE_HANDLE m_deviceHandle; 
    };
};

#endif


