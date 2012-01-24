#include "stdafx.h"
#include "HDLCRegMap.h"
#include "boardds2155.h"


namespace
{
 
	//для прерываний   
     
}    


namespace Ds2155
{
	
    enum
    {
        //количество HDLC контроллеров в DS2155
        Number_of_Ctrls  = 2,        
    };        
		    
   	//--------------------------------------------    
    
    HdlcRegMap::HdlcRegMap(byte ctrlNum):
        m_ctrlNum(ctrlNum)
    {
        if (ctrlNum)
            FillByCtrl1Reg();
        else
            FillByCtrl0Reg();                
    }
    
   	//--------------------------------------------
	      
    void HdlcRegMap::FillByCtrl0Reg()
    {
    	
		HTC    = 0x90;
		HFC    = 0x91;
		HRCS1  = 0x92;
		HRCS2  = 0x93;
		HRCS3  = 0x94;
		HRCS4  = 0x95;
		HRTSBS = 0x96;
		HTCS1  = 0x97;
		HTCS2  = 0x98;
		HTCS3  = 0x99;
		HTCS4  = 0x9a;
		HTTSBS = 0x9b;
		HRPBA  = 0x9c;
		HTF    = 0x9d;
		HRF    = 0x9e;
		HTFBA  = 0x9f;
		HRC    = 0x31;  
		SRH    = 0x20;
		IMH    = 0x21;  
		INFH   = 0x2e;
		INFO4  = 0x2d;    	    	       
		
    }    
    
	//---------------------------------------------------------------------
	
    void HdlcRegMap::FillByCtrl1Reg()
    {
    	    	   	
		HTC    = 0xa0;
		HFC    = 0xa1;
		HRCS1  = 0xa2;
		HRCS2  = 0xa3;
		HRCS3  = 0xa4;
		HRCS4  = 0xa5;
		HRTSBS = 0xa6;
		HTCS1  = 0xa7;
		HTCS2  = 0xa8;
		HTCS3  = 0xa9;
		HTCS4  = 0xaa;
		HTTSBS = 0xab;
		HRPBA  = 0xac;
		HTF    = 0xad;
		HRF    = 0xae;
		HTFBA  = 0xaf;
		HRC    = 0x32;  
		SRH    = 0x22;
		IMH    = 0x23;  
		INFH   = 0x2f;
		INFO4  = 0x2d;
		
    }    

	//---------------------------------------------------------------------

    void HdlcRegMap::EnableIntCtrl0()
    {
        
    }    
	
	//---------------------------------------------------------------------

    void HdlcRegMap::DisableIntCtrl0()
    {
    			    	    	
        
    }        
    
	//---------------------------------------------------------------------
	
    void HdlcRegMap::EnableIntCtrl1()
    {
        
    	
    }    
    
	//---------------------------------------------------------------------
	
    void HdlcRegMap::DisableIntCtrl1()
    {
        
    	
    }    
        
	//---------------------------------------------------------------------        
	
    void HdlcRegMap::DisableInt()
    {
        if (m_ctrlNum) DisableIntCtrl1();
        		  else DisableIntCtrl0();
    }

	//---------------------------------------------------------------------        
	
    void HdlcRegMap::EnableInt()
    {
        if (m_ctrlNum) EnableIntCtrl1();
        		  else EnableIntCtrl0();
    }    
    
    //---------------------------------------------------------------------
    
    byte HdlcRegMap::GetCtrlNum() const
    {
        return m_ctrlNum;
    }
    
    //---------------------------------------------------------------------    
    
	byte HdlcRegMap::GetControllersCount()
	{

        return Number_of_Ctrls;				
        
	}

    //---------------------------------------------------------------------            
	    
}; //namespace BfTdm

