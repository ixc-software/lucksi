#ifndef __TDM_REGMAP__
#define __TDM_REGMAP__

#include "Platform/Platform.h"

namespace BfTdm
{
    using namespace Platform;
    class Tdm;
    
    /*
        Tdm register profile
		.....
    */
    
	class TdmRegMap
	{
	public:	    
	    static word GetSportsCount();
        static word GetSportTimeSlotsCount();
	
        TdmRegMap(word sportNum);
        void InitDMAInt();
    	void DisableDMAInt();
        word GetSportNum() const;        
					
	private: 	
	
        void FillBySport0Reg();
        void FillBySport1Reg();
        void InitDMAIntSport0();
        void InitDMAIntSport1();
    	void DisableDMAIntSport0();
    	void DisableDMAIntSport1();   
        word m_sportNum;
        
	public:         
		
        volatile unsigned short  *m_pDMARx_IRQ_Status;
        volatile unsigned short  *m_pDMATx_IRQ_Status;
        volatile unsigned short  *m_pSPORT_RClkDiv;
        volatile unsigned short  *m_pSPORT_RFSDiv; 
        
    	volatile unsigned long   *m_pSPORT_MTCS0;  
    	volatile unsigned long   *m_pSPORT_MTCS1;  
    	volatile unsigned long   *m_pSPORT_MTCS2;  
    	volatile unsigned long   *m_pSPORT_MTCS3;  
    	volatile unsigned long   *m_pSPORT_MRCS0;  
    	volatile unsigned long   *m_pSPORT_MRCS1;  
    	volatile unsigned long   *m_pSPORT_MRCS2;  
    	volatile unsigned long   *m_pSPORT_MRCS3;      
        	
    	volatile unsigned short  *m_pSPORT_RCR1;       
    	volatile unsigned short  *m_pSPORT_RCR2;       
    	volatile unsigned short  *m_pSPORT_TCR1;       
    	volatile unsigned short  *m_pSPORT_TCR2;       
    	volatile unsigned short  *m_pSPORT_MCMC1;      
    	volatile unsigned short  *m_pSPORT_MCMC2;      
        
        volatile unsigned short  *m_pDMARx_Config;     
        void *volatile           *m_pDMARx_Next_Desc_Ptr;
        volatile unsigned short  *m_pDMARx_X_Count;   
        volatile signed short    *m_pDMARx_X_Modify;  
        volatile unsigned short  *m_pDMARx_Y_Count;   
        volatile signed short    *m_pDMARx_Y_Modify;  
        
        volatile unsigned short  *m_pDMATx_Config;
        void *volatile           *m_pDMATx_Next_Desc_Ptr;
        volatile signed short    *m_pDMATx_X_Modify;
        volatile unsigned short  *m_pDMATx_X_Count; 
        volatile signed short    *m_pDMATx_Y_Modify;
        volatile unsigned short  *m_pDMATx_Y_Count; 			
		
	};

};

#endif
