#ifndef CBOOTERVERSION_H
#define CBOOTERVERSION_H

#include "Platform/PlatformTypes.h"

namespace BfBootCore
{    
    //static Platform::word CBooterVersion = 0; // � ����� ������� ������, ��� �������?   
    
    const std::string& BooterVersionInfo();
    Platform::dword BooterVersionNum();

} // namespace BfBootCore

#endif
