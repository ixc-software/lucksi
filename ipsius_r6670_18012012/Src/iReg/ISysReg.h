#pragma once

#include "Utils/IBasicInterface.h"
#include "Domain/ObjectName.h"
#include "CallRecord.h"

namespace iReg
{

    class IDriSessionLog : public Utils::IBasicInterface
    {
    public:
        virtual void Add(const QString &data) = 0;
    };

    // ---------------------------------------------------------

    class ISysReg : public Utils::IBasicInterface
    {
    public:
        virtual void CallInfo(const Domain::ObjectName &sender, 
            const CallRecord &rec) = 0;

        virtual void EventInfo(const Domain::ObjectName &sender, const QString &msg) = 0;

        virtual IDriSessionLog* CreateDriSessionLog(const QString &login) = 0;
    };
        
}  // namespace iReg

