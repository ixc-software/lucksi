#include "stdafx.h"

#include "PsbCofidecDrv.h"

namespace DevIpPult
{
    class DevIpPultSpiMng {};
}

// ----------------------------------------------------

namespace PsbCofidec
{
    class Psb4851 {};
    class Psb2170 {};
    
    PsbCofidecDrv::PsbCofidecDrv(IPsbDebug *iDebug)
    {
        // nothing
    }
    
    PsbCofidecDrv::~PsbCofidecDrv()
    {
        // nothing        
    }
    
    void PsbCofidecDrv::SetState(const iPult::CofidecState &state)
    {
        // nothing        
    }
    
    void PsbCofidecDrv::SetDebugMode(bool toHandsfree)
    {
        // nothing 
    }
    
    std::string PsbCofidecDrv::StateInfo() const
    {
        return "(no info, emulation)";
    }
   
            
    
}  // namespace PsbCofidec

