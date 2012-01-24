#ifndef FLASH_M25P128_H
#define FLASH_M25P128_H


#include "BfDev/SysProperties.h"
#include "ExtFlash/M25P128.h"
#include "BfBootSrv/RangeIOBase.h"
#include "BfDev/SpiBusMng.h"

#include "SectorStorageDeviceBase.h"

namespace BfBootSrv
{

    // ����� ����������� ������ ������ �������� ?
    // ������� ��� ��������� ���� CBootImgSize ��� � �������� �����. ��������� �������� ������������ � ExtFlash::M25P128::getMap - ?
    // ��� ���� ����������� ���������������� ��������� �������� � ������� ������������ �����.

    using Platform::dword;    
    
    /*
        ������ �������: 16 777 216 ���� (16MB)
        ������ �������: 262144 ����
        ���������� ��������: 64
        
        �������� ������: speedKHz * 100 ����/��� - �������� �������� �� ���� spi 
        �������� ������: speedKHz * 100 ����/��� (+ 7���� �� ������ �������� 256����) 
        ����� �������� �������: 4���
        ����� �������� ���� �����: 256���        
    */

    class Flash_M25P128 : public IPhisicalSectorDev
    {                
        mutable ExtFlash::M25P128 m_extFlash;
        SectorMap m_sectors;
        SectorStorageDeviceBase m_base;
        scoped_ptr<RangeIOBase> m_rangeBase;

    // own
    private:
        void WaitExtFlashFree();               

    // IPhisicalSectorDev
    private:
        // ������ � data ������������ ������� � ��������.
        void PhRead(dword off, void* data, dword dataSize) const;
        // ������ ������ �� ��������.
        // ������ ��������������� �������� ������ ���� �������������� ����� (����������� �������-����������)!
        void PhWrite(dword off, const void* data, dword dataSize, bool withVerify); // can throw ErrWriteWithoutErase
        void PhErase(const SectorInfo &sector);
        void PhEraseFull();
        std::string getDeviceSignature();

    public:        

        Flash_M25P128(const BfDev::SpiBusPoint &point); 
        RangeIOBase& getRangeIO();
        
        /*SectorStorageDeviceBase& getSectorDev()
        {
            return m_base;
        }*/        
    };

    
} // namespace BfBootSrv

#endif
