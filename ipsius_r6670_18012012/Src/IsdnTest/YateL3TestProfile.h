#ifndef YATEL3TESTPROFILE_H 
#define YATEL3TESTPROFILE_H 

#include "YateL2TestProfile.h"
#include "ISDN/IsdnStackSettings.h"
#include "isdn/L3Profiles.h"
#include "LogOutSettings.h"

namespace IsdnTest
{

    using ISDN::IsdnStackSettings;

    class YateL3TestProfile : public LogOutSettings
    {        
        CfgHost m_cfgHost;
        
        bool m_traceL2;
        bool m_traceL3;
        shared_ptr<ISDN::L2Profile> m_l2prof;
        shared_ptr<ISDN::L3Profile> m_l3prof;
        

        IsdnStackSettings m_stackProf;

    public:

        YateL3TestProfile(iLogW::LogStore& logStore)
            : LogOutSettings(true, "YateL3test.log", &logStore),                    
            m_cfgHost(5061, "192.168.0.158:5061"), //self port, dest. address:port 
            
            m_traceL2(false),
            m_traceL3(true),
            m_l2prof( ISDN::L2Profile::CreateAsTE( ISDN::LapdTraceOption(true)) ),
            m_l3prof( ISDN::L3Profile::CreateAsUser( ISDN::DssTraceOption(true)) ),

            m_stackProf(m_l2prof, m_l3prof)
        {
            m_l2prof->setTei(0);
            m_l2prof->setInitialState(false);
        }
       
        const CfgHost& getCfgHost()
        {
            return m_cfgHost;
        }

        const IsdnStackSettings& getStackProf()
        {
            return m_stackProf;
        }

        bool getTraceOn()
        {
            return true;
        }        

        ISDN::IeConstants::Location getLocation() const
        {
            return m_l3prof->GetOptions().location;
        }
    };

} // namespace IsdnTest

#endif
