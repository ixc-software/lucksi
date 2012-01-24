#ifndef __ANALOGDEVICEDRIVER__

#define __ANALOGDEVICEDRIVER__

#include "stdafx.h"
#include "Platform/PlatformTypes.h"

#include "Utils/HostInf.h"

#include "Domain/ObjectName.h"
#include "Domain/NamedObject.h"
#include "Domain/IDomain.h"

#include "IDeviceDriver.h"

namespace CallModel
{
    using Platform::dword;
    using namespace Domain;

    // ������� ��������� (������) ����������� �����
    class AnalogChannelDriver : public IDeviceDriverChannel
    {
    public:
    };

    // ������� ����������� �����
    class AnalogDeviceDriver : public NamedObject, public IDeviceDriver
    {
        std::vector<AnalogChannelDriver*> m_channels;

    public:

        /*
            RP -- ������ ������ ����� � ����������� ����������, ������� ����������� �������, �.�.
            �������� ������ ����������, ������� ������, ������������� �����������
            ��������� etc.
        */
        AnalogDeviceDriver(Domain::IDomain *pDomain, 
            const ObjectName &name, dword channelCount, const Utils::HostInf &rp)
            : NamedObject(pDomain, name)
        {
            // ...
        }

    };
	
}  // namespace CallModel


#endif
