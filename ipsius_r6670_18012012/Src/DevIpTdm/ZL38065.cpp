#include "stdafx.h"

#include "ZL38065.h"

namespace DevIpTdm
{

    
    ZL38065::ZL38065(IZl38065Transport &transport) :
    	m_transport(transport)
    {
    }
           
    bool ZL38065::SetMode(int pair, bool enableExtended, bool enableLoCh, bool enableHiCh,
        /* out */ std::string &extErrorInfo)
    {
    	return false;
    }
        
    // return true if ZL38065 detected
    bool ZL38065::TryDetect(IZl38065Transport &transport)
    {    	
		return false;
    }
    
} // namespace DevIpTdm
