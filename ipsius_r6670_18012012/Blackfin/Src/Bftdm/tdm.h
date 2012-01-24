#ifndef __TDM_H__
#define __TDM_H__

#include "Platform/Platform.h"
#include "Utils/IBasicInterface.h"
#include "TdmRegMapping.h"
#include "TdmProfile.h"

#include "DMADirBlockList.h"
#include "UserChBlockList.h"
#include "UserBlock.h"
#include "ITdmObserver.h"
#include "TdmStatistic.h"

namespace BfTdm
{
                       
    using namespace Platform;   
                	    
	// ---------------------------------------------------------------------    	    		    
                
    /*
        Tdm class
        .....
    */           
	class Tdm  : boost::noncopyable
	{	                  	    	        	
	public:

        enum InterruptDirection
        {
            TxInterrupt,
            RxInterrupt
        };
	
        Tdm(ITdmObserver &observer, word sportNum, const TdmProfile &profile);	
        ~Tdm();	        
		static void DMAInterrupt(word sportNum, InterruptDirection dir);		
		void Enable();
		void Disable();		
		bool isEnabled();
        word GetSportNum() const;
		const TdmStatistic &PeekStatistic() const;        
        const TdmProfile& Profile() const { return m_profile; } 
	    
    private:               
    
    	static Tdm &Instance(word sportNum);    
        void DMAInterrupt(InterruptDirection dir);    	
        void BlockProcess();
        void InitSport();
        void InitSPORTDMA();
        void EnableDMASport();        
		void DisableDMASport();        
        void AllocBlocks();        
        
               		
    private:
            
		const TdmProfile m_profile;		
    	ITdmObserver &m_observer;
		TdmRegMap     m_regMap;		
		bool		  m_enabled;
		bool		  m_started;		
        UserChBlockList   m_Ch0;
        UserChBlockList   m_Ch1;                		
        DMADirBlockList   m_Tx;
        DMADirBlockList   m_Rx;
        TdmStatistic  m_statistic;		                                        
	};		
			
} //namespace BFTdm

#endif

