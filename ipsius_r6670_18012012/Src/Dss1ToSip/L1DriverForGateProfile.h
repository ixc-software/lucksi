#pragma once

#include "Utils/HostInf.h"

namespace Utils {   class MetaConfig; };

namespace Dss1ToSip
{
    class L1DriverForGateProfile 
    {
    public:
        L1DriverForGateProfile ();
        bool IsTraceOn() const;
        bool IsAllTrace() const;
        bool IsShortTrace() const;
        static int  MinLenForShortTrace();
        bool IsDataTrace() const;
    private:
        std::string m_typeTrace;
    };
}



