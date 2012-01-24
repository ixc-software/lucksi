#ifndef __TDM_PROFILE__
#define __TDM_PROFILE__


#include "Utils/IBasicInterface.h"
#include "TdmRegMapping.h"

namespace BfTdm
{
    using namespace Platform;

    
    enum TdmLaw
    {
        CNoCompanding = 0,
        CAlawCompanding,
        CUlawCompanding,

        CTdmLawEnumLimit
    };
    

    class ITdmProfiler : public Utils::IBasicInterface
    {
    public:
        virtual void OnProcessingBegin() = 0;
        virtual void OnProcessingEnd() = 0;
    };
    
    /*
        Tdm profile
    */
    
	class TdmProfile
	{ 
    
	public:	    
	
        TdmProfile(word DMABlocksCount, word DMABlockCapacity, bool extSync, TdmLaw compandingLaw, bool t1mode = false):
            m_DMABlocksCount(DMABlocksCount),
            m_DMABlockCapacity(DMABlockCapacity),
            m_DMABuffSize(TdmRegMap::GetSportTimeSlotsCount() * m_DMABlockCapacity *2),
            m_extSync(extSync),
            m_T1Mode(t1mode),
            m_compandingLaw(compandingLaw),
            m_pProfiler(0)
        {
		    ESS_ASSERT(compandingLaw < CTdmLawEnumLimit && "Unknown companding Law");	
        }
            	
	    word GetDMABlocksCount() const { return m_DMABlocksCount; }	    	    
	    word GetDMABlockCapacity() const { return m_DMABlockCapacity; }	    
	    bool isExtSync() const { return m_extSync; }
	    word GetDMABuffSize() const { return m_DMABuffSize; }
	    TdmLaw GetCompandingLaw() const { return m_compandingLaw; }	    

        void Name(const std::string &val) { m_name = val; }
        const std::string& Name() const { return m_name; }

        bool isT1Mode() const  { return m_T1Mode; }	    
        void T1Mode(bool mode) { m_T1Mode = mode; }

        ITdmProfiler* Profiler() const { return m_pProfiler; }
        void Profiler(ITdmProfiler *p) { m_pProfiler = p; }

        std::string getAsString() const
        {
            std::ostringstream ss;

            ss << "DMABlocksCount = " << m_DMABlocksCount
                << ", DMABlockCapacity = " << m_DMABlockCapacity
                << ", DMABuffSize = " << m_DMABuffSize
                << ", extSync = " << (m_extSync ? "true" : "false");

            return ss.str();
        }
	     
    private:
    
  	    word    m_DMABlocksCount;
	    word    m_DMABlockCapacity;	    
		word	m_DMABuffSize;
	    bool    m_extSync;
	    bool    m_T1Mode;	    
	    TdmLaw  m_compandingLaw;
        std::string m_name;
        ITdmProfiler *m_pProfiler;        
	};

};

#endif
