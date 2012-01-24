#ifndef IMGDESCRIPTOR_H
#define IMGDESCRIPTOR_H

#include "stdafx.h"
#include "Platform/PlatformTypes.h"

namespace BfBootCore
{
    using Platform::dword;
    using Platform::word;

    // ������������� ������ ����������
    typedef word ImgId;

    //��������� ������ ����������.
    struct ImgDescriptor
    {
        ImgId Id; // ���������� ����� ����� (���������� �������������).
        dword Size; 
        std::string Name; // ��� �����. ������ ��� �������� (������, ���������� � �.�.)
        
        // used only in tests
        bool operator ==(const ImgDescriptor& other) const;
        bool operator !=(const ImgDescriptor& other) const;                 
     
        std::string ToString() const;                
        bool setFromString(const std::string& str);
    };          

} // namespace BfBootCore

#endif
