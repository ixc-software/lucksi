#ifndef __HDLC_REGMAP__
#define __HDLC_REGMAP__

#include "Platform\platform.h"

namespace Ds2155
{
    using namespace Platform;
//    class Tdm;
    
    /*
        HDLC register profile
		.....
    */
    
	class HdlcRegMap
	{
	public:	    
	    static byte GetControllersCount();
	
        HdlcRegMap(byte ctrlNum);        
        byte GetCtrlNum() const;        
        
        void EnableInt();
    	void DisableInt();        
					
	private: 	
	
        void FillByCtrl0Reg();
        void FillByCtrl1Reg();
        void EnableIntCtrl0();
        void EnableIntCtrl1();
    	void DisableIntCtrl0();
    	void DisableIntCtrl1();   
        word m_ctrlNum;
        
	public:         
		
		byte HTC;
		byte HFC;
		byte HRCS1;
		byte HRCS2;
		byte HRCS3;
		byte HRCS4;
		byte HRTSBS;
		byte HTCS1;
		byte HTCS2;
		byte HTCS3;
		byte HTCS4;
		byte HTTSBS;
		byte HRPBA;
		byte HTF;
		byte HRF;
		byte HTFBA;
		byte HRC;  
		byte SRH;
		byte IMH;  
		byte INFH;
		byte INFO4;
	};
	
	typedef enum
	{
		WM4    =  0x00,
		WM16   =  0x01,
		WM32   =  0x02,  
		WM48   =  0x03,    
		WM64   =  0x04,    
		WM80   =  0x05,    
		WM96   =  0x06,    
		WM112  =  0x07    
	} WaterMark; 	

};

#endif
