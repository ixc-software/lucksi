#ifndef __USERBLOCKINTERFACE_H__
#define __USERBLOCKINTERFACE_H__

#include "Platform/Platform.h"
#include "Utils/IBasicInterface.h"

namespace TdmMng
{

    class TdmUserBlockExt;

}  // namespace TdmMng

namespace BfTdm
{                       
    using namespace Platform;   
       
    // ���� ������ TDM
  	class IUserBlockInterface : public Utils::IBasicInterface
    {
        TdmMng::TdmUserBlockExt *m_pExt;

    public:

        IUserBlockInterface() : m_pExt(0) {}

        // ����������� � ���� ����� �������� ������
        virtual void CopyRxTSBlockTo(byte timeSlot, byte *tsBuff,  word offset, word tsSampleCount) = 0;
        virtual void CopyRxTSBlockTo(byte timeSlot, word *tsBuff,  word offset, word tsSampleCount) = 0;

        // ��������� ������ ��� ��������
        virtual void PutTxTSBlockFrom(byte timeSlot, const byte *tsBuff, word offset, word tsSampleCount) = 0;
        virtual void PutTxTSBlockFrom(byte timeSlot, const word *tsBuff, word offset, word tsSampleCount) = 0;

        virtual void CopyTxTSBlockTo(byte timeSlot, byte *tsBuff, word buffSize) = 0;  

        // ��������� ������ � ����� ���������
        virtual void UserFree() = 0;

        // ����������� ������
        virtual word GetTSBlockSize() const = 0;

        // ����������� ������ TX, ���������� � RX
        virtual void CopySyncTxTo(byte timeSlot, byte *tsBuff,  word offset, word tsSampleCount) = 0;
        virtual void CopySyncTxTo(byte timeSlot, word *tsBuff,  word offset, word tsSampleCount) = 0;        
        
        // ��������� �� ������ ������ ������, �� ����������: Frame0TS0, Frame0TS1 ... Frame0TS31, Frame1TS0
        virtual const word *GetRxBlock(word frameNum) const = 0;
        
        // ��������� �� ������ ������ ��������, �� ����������: Frame0TS0, Frame0TS1 ... Frame0TS31, Frame1TS0        
        virtual word *GetTxBlock(word frameNum) const = 0;
        
                        
        // ������ � ���������� ����������� 
        void SetExtInfo(TdmMng::TdmUserBlockExt *p) { m_pExt = p; }
        TdmMng::TdmUserBlockExt* GetExtInfo() { return m_pExt; }
    };
        
			
} //namespace Bftdm

#endif

