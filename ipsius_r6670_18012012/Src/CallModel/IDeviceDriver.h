#ifndef __IDEVICEDRIVER__

#define __IDEVICEDRIVER__

#include "stdafx.h"
#include "Utils/IBasicInterface.h"

namespace CallModel
{

    // ���������, �������������� ����� ���������� ���������
    class IDeviceDriver : public Utils::IBasicInterface
    {
    };

    // ���������, �������������� ����� ���������� ������� ���������
    class IDeviceDriverChannel : public Utils::IBasicInterface
    {
    };
		
}  // namespace CallModel

#endif
