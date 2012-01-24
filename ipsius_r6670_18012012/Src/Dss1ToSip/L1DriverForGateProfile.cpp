#include "stdafx.h"
#include "Utils/StringUtils.h"
#include "L1DriverForGateProfile.h"

namespace 
{
    enum
    {   
        None = 0, 
        All, 
        Short, 
        Data
    };
    const char* TypeTrace[4] = {"None", "All", "Short", "Data"};
};


namespace Dss1ToSip
{
    L1DriverForGateProfile::L1DriverForGateProfile() : 
        m_typeTrace(TypeTrace[None]){}

    bool L1DriverForGateProfile::IsTraceOn() const
    {
        return m_typeTrace != TypeTrace[None];
    }

    bool L1DriverForGateProfile::IsAllTrace() const
    {
        return m_typeTrace == TypeTrace[All];
    }

    bool L1DriverForGateProfile::IsShortTrace() const
    {
        return m_typeTrace == TypeTrace[Short];
    }

    int MinLenForShortTrace()
    {
        return 5;
    }

    bool L1DriverForGateProfile::IsDataTrace() const
    {
        return m_typeTrace == TypeTrace[Data];
    }
};

