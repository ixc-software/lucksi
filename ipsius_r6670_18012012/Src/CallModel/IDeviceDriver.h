#ifndef __IDEVICEDRIVER__

#define __IDEVICEDRIVER__

#include "stdafx.h"
#include "Utils/IBasicInterface.h"

namespace CallModel
{

    // Интерфейс, поддерживаемый всеми драйверами устройств
    class IDeviceDriver : public Utils::IBasicInterface
    {
    };

    // Интерфейс, поддерживаемый всеми драйверами каналов устройств
    class IDeviceDriverChannel : public Utils::IBasicInterface
    {
    };
		
}  // namespace CallModel

#endif
