#ifndef __TDM_H__
#define __TDM_H__

#include "Platform/Platform.h"
#include "Utils/IBasicInterface.h"

/*
#include "TdmRegMapping.h"
#include "DMADirBlockList.h"
#include "UserChBlockList.h"
#include "UserBlock.h"
 */

#include "ITdmObserver.h"
#include "TdmProfile.h"
#include "TdmStatistic.h"

#include "BfEmul/E1PcEmul.h"

namespace BfTdm
{
                       
    using namespace Platform;   
                	    
	// ---------------------------------------------------------------------    	    		    
                
    // Tdm class emulation
	class Tdm  : boost::noncopyable
	{	  
        int                 m_sportNum;
        const TdmProfile    m_profile;		
        ITdmObserver        &m_observer;
        bool		        m_enabled;
        TdmStatistic        m_statistic;

        BfEmul::TdmEmulPoint *m_pEmulPoint;

	public:
	
        Tdm(ITdmObserver &observer, word sportNum, const TdmProfile &profile) :
          m_sportNum(sportNum),
          m_profile(profile),
          m_observer(observer),
          m_pEmulPoint(0)
        {
            m_enabled = false;
        }

        ~Tdm()
        {

        }

		void Enable()
        {
            ESS_ASSERT(!m_enabled);

            m_enabled = true;
            m_pEmulPoint = BfEmul::PcEmul::Instance().TDM().RegisterTdmPoint(m_profile, m_observer);
        }

		void Disable()
        {
            ESS_ASSERT(m_enabled);

            m_enabled = false;
            BfEmul::PcEmul::Instance().TDM().UnregisterTdmPoint(m_pEmulPoint);
        }

		bool isEnabled()
        {
            return m_enabled;
        }
        
		const TdmStatistic &PeekStatistic() const
        {
            return m_statistic;
        }

        int GetSportNum() const { return m_sportNum; }
        
        const TdmProfile& Profile() const { return m_profile; }         
	                          		
	};		
			
} //namespace BFTdm

#endif

