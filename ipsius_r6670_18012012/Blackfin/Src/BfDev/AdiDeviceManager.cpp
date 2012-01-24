#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "AdiDeviceManager.h"

namespace BfDev
{
    AdiDeviceManager::AdiDeviceManager(int countDevice,  void *pEnterCriticalParam) : 
        m_devices(countDevice),
        m_countAllDevice(countDevice),
        m_countDevice(0),
        m_deviceManagerMemory(ADI_DEV_BASE_MEMORY + ADI_DEV_DEVICE_MEMORY*countDevice)
    {
        u32 errorCode = adi_dev_Init(
            &m_deviceManagerMemory[0],      // pointer to driver memory
            m_deviceManagerMemory.size(),   // memory size in bytes
            &m_countAllDevice,              // number of supported devices
            &m_deviceManagerHandle,         // memory for the device manager handle storing
            pEnterCriticalParam             // parameter for enter critical region
            );
    }

    //-----------------------------------------------------------------------------

    AdiDeviceManager::~AdiDeviceManager()
    {
        ESS_ASSERT(!GetCountDevice());

        u32 errorCode = adi_dev_Terminate(m_deviceManagerHandle);
    }

    //-----------------------------------------------------------------------------
    // IVirtualDestroyOwner

    void AdiDeviceManager::Add(Utils::IVirtualDestroy *device)
    {
    	ESS_ASSERT(m_countDevice != m_countAllDevice);
    	
		ListOfDevice::iterator i = std::find(m_devices.begin(), m_devices.end(), device);
		
		ESS_ASSERT(i == m_devices.end());
		
		Utils::IVirtualDestroy *nullPtr = 0;
		i = std::find(m_devices.begin(), m_devices.end(), nullPtr);

		ESS_ASSERT(i!=m_devices.end());
		*i = device; 
    }
    
    //-----------------------------------------------------------------------------    
    
    void AdiDeviceManager::Delete(Utils::IVirtualDestroy *device)
    {
		ListOfDevice::iterator i = std::find(m_devices.begin(), m_devices.end(), device);
		
		ESS_ASSERT(i != m_devices.end());
		
		*i = 0;
    }
    
};


