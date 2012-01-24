#include "stdafx.h"

#include "TdmRegMapping.h"


namespace BfTdm
{
    
    enum
    {
        //количество SPORT портов
        Number_of_SPORTs  = 2,        
        //количество каналов в кадре            
        Number_of_Channels = 32,
    };        
    
        
    word TdmRegMap::GetSportsCount()
    {
        return Number_of_SPORTs;
    }
	    
    //---------------------------------------------------------------------
    word TdmRegMap::GetSportTimeSlotsCount()
    {
        return Number_of_Channels;
    }
	    
    //---------------------------------------------------------------------
    
	    
}; //namespace BfTdm

