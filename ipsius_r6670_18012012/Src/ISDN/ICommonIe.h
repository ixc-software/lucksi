#ifndef ICOMMONIE_H
#define ICOMMONIE_H

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "IeId.h"


namespace ISDN
{    
    class ICommonIe : public Utils::IBasicInterface
    {        
    public:        
        virtual const std::vector<byte>& Get() const = 0; // чтение исход€щего
        virtual IeId GetIeId() const = 0; // дл€ пользователей неизвестных и≈ - ValidateIeMt        
        virtual void WriteContent(std::stringstream& out) const = 0;
        virtual const std::string& GetName()const = 0;        
    };
} // namespace ISDN

#endif
