#include "stdafx.h"

#include "TdmRegMapping.h"


namespace BfTdm
{
    
    enum
    {
        //���������� SPORT ������
        Number_of_SPORTs  = 2,        
        //���������� ������� � �����            
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

