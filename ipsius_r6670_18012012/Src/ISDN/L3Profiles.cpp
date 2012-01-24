#include "stdafx.h"
#include "Utils/IBasicInterface.h"
 
#include "L3Profiles.h"
#include "BDistributionStrategies.h"
#include "IsdnIntfGroup.h"

namespace ISDN
{
   
//    const char *L3Profile::DefaultChannelsSet = "1..15, 17..31";

    void L3Profile::DefaultInit(const DssTraceOption& trace)
    {       
        m_option = Options();
       
        m_trace = trace;
        m_maxCallRefValue = CTwoByteCRef; //COneByteCRef/CTwoByteCRef;
    }  

    boost::shared_ptr<L3Profile> L3Profile::CreateAsUser(const DssTraceOption& trace, 
        Utils::SafeRef<IDssToGroup> intfGroup, 
		const std::string &prefix,
        bool statActive)
    {
        ESS_ASSERT(!intfGroup.IsEmpty());

        boost::shared_ptr<L3Profile> ret( new L3Profile );
        
        ret->DefaultInit(trace);
        ret->m_timers.InitUserDefault();
        ret->m_L3Name = prefix + "/L3_TE";
        ret->m_userSide = true;
        ret->m_option.SetSide(false);       
        ret->m_intfGroup = intfGroup;
        ret->m_statActive = statActive;        
        return ret;
    }

    boost::shared_ptr<L3Profile> L3Profile::CreateAsNet(const DssTraceOption& trace, 
        Utils::SafeRef<IDssToGroup> intfGroup, 
		const std::string &prefix,
        bool statActive)
    {
        ESS_ASSERT(!intfGroup.IsEmpty());

        boost::shared_ptr<L3Profile> ret( new L3Profile );
        
        ret->DefaultInit(trace);
        ret->m_timers.InitNetDefault();
        ret->m_L3Name = prefix + "/L3_NT";
        ret->m_userSide = false;
        ret->m_option.SetSide(true);        
        ret->m_intfGroup = intfGroup;
        ret->m_statActive = statActive;        
        return ret;
    }               

     const L3Profile &L3Profile::ValidateFields() const
    {
        // if interface count more then 1, option m_AssignBChannalsIntfId mast be present 
        ESS_ASSERT(m_intfGroup->CountIntf() == 1 || m_option.m_AssignBChannalsIntfId);
        m_option.Validate();

        //todo other validate ...

        return *this;
    }

    Utils::SafeRef<IDssToGroup> L3Profile::getIntfGroupe()
    {
        return m_intfGroup;
    }

    const L3Profile::Options& L3Profile::GetOptions() const
    {
        m_option.Validate();
        return m_option;
    }

    L3Profile::Options& L3Profile::GetOptions()
    {
        return m_option;
    }
    bool L3Profile::IsUserSide() const
    {
        return m_userSide;
    }

    DssTimersProf& L3Profile::GetTimers()
    {
        return m_timers;
    }

    const DssTimersProf& L3Profile::GetTimers() const
    {
        return m_timers;
    }

    const std::string& L3Profile::Name() const
    {
        return m_L3Name;
    }

    int L3Profile::GetMaxCallrefValue() const
    {
        return m_maxCallRefValue;
    }

    const DssTraceOption& L3Profile::getTraceOption() const
    {
        return m_trace;
    }

    DssTraceOption& L3Profile::getTraceOption()
    {
        return m_trace;
    }
    bool L3Profile::getStatActive() const
    {
        return m_statActive;
    }
    // ------------------------------------------------------------------------------------

    L3Profile::Options::Options()
    {        
        m_SendCallingIfExist = true;
        m_SendProgInd = true;        
        m_AssignBChannalsIntfId = false;
        m_SendConnectAck = true;
        m_Location = IeConstants::SpecNonStandart; // reserved
        m_AutoReActivation = true;
    }

    void L3Profile::Options::Validate() const
    {        
        ESS_ASSERT(m_Location <= IeConstants::SpecNonStandart);
    }

    void L3Profile::Options::SetSide( bool forNetwork )
    {        
        m_Location = forNetwork ? IeConstants::RLN : IeConstants::U;                
        m_SendConnectAck = forNetwork ? true : true;// в стандарте на сетевой стороне опция не используется!
    }
};



