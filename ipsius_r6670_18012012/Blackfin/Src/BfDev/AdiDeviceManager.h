#ifndef _BF_ADI_DEVICE_MANAGER_H_
#define _BF_ADI_DEVICE_MANAGER_H_

#include "stdafx.h"
#include <drivers/adi_dev.h>
#include "Platform/PlatformTypes.h"
#include "Utils/IVirtualDestroy.h"

namespace BfDev
{
	class AdiDevice;
	
    class AdiDeviceManager : boost::noncopyable,
        public Utils::IVirtualDestroyOwner
    {
    public:
        AdiDeviceManager(int countDevice, void *pEnterCriticalParam = 0);
        ~AdiDeviceManager();
        
        int GetCountDevice() const {    return m_countDevice; }
        
    // IVirtualDestroyOwner 
    private:
        void Add(Utils::IVirtualDestroy*);
        void Delete(Utils::IVirtualDestroy*);
    private:
		friend class AdiDevice;    
        
		ADI_DEV_MANAGER_HANDLE Handle() {	return m_deviceManagerHandle;	}    
    private:        
    	typedef std::vector<Utils::IVirtualDestroy*> ListOfDevice;
    	ListOfDevice m_devices;

    	u32 m_countAllDevice;
        u32 m_countDevice;

        std::vector<Platform::byte> m_deviceManagerMemory;
        ADI_DEV_MANAGER_HANDLE m_deviceManagerHandle;
    };
};

#endif
