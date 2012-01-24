#ifndef IMGDESCRIPTOR_H
#define IMGDESCRIPTOR_H

#include "stdafx.h"
#include "Platform/PlatformTypes.h"

namespace BfBootCore
{
    using Platform::dword;
    using Platform::word;

    // идентификатор образа приложения
    typedef word ImgId;

    //Описатель образа приложения.
    struct ImgDescriptor
    {
        ImgId Id; // Порядковый номер файла (уникальный идентификатор).
        dword Size; 
        std::string Name; // Имя файла. Служит для описания (версия, назначение и т.п.)
        
        // used only in tests
        bool operator ==(const ImgDescriptor& other) const;
        bool operator !=(const ImgDescriptor& other) const;                 
     
        std::string ToString() const;                
        bool setFromString(const std::string& str);
    };          

} // namespace BfBootCore

#endif
