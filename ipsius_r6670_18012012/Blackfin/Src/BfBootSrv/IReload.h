#ifndef IRELOAD_H
#define IRELOAD_H

#include "Platform/PlatformTypes.h"
#include "Utils/IBasicInterface.h"
#include "BfBootCore/ImgDescriptor.h"

namespace BfBootSrv
{   
    class BfStorage;        

    class IReload : Utils::IBasicInterface
    {
    public:
        // ������������ �����
        virtual void Reboot() = 0;

        // ��������� ����� ���������� � �������� ��������� �� ����������.
        virtual void LoadToMain(BfStorage& storage, const BfBootCore::ImgDescriptor& img) = 0; 

        // ��������� ����� ���������� �� ��������������� ���������.
        virtual void LoadToSpi(BfStorage& storage, const BfBootCore::ImgDescriptor& img) = 0;
        virtual void LoadToSpi(Platform::dword size, const Platform::byte *data) = 0;
    };
} // namespace BfBootSrv

#endif
