#ifndef YATEL2TESTPROFILE_H
#define YATEL2TESTPROFILE_H

#include "iLog/LogWrapper.h"
#include "isdn/L2Profiles.h"

#include "Utils/HostInf.h"

#include "LogOutSettings.h"



namespace IsdnTest
{
    using iLogW::LogSessionProfile;
    using iLogW::ILogSessionToLogStore;
    using ISDN::L2Profile;

    /*struct UdpDriverSettings
    {
        UdpDriverSettings(bool teSide, bool trace)
            : traceRaw(trace),
            traceExcange(trace),
            isTeSide(teSide)
        {}

        bool traceRaw;
        bool traceExcange;
        bool isTeSide;
    };*/

    class CfgHost
    {
        Utils::PortNumber m_localPort;
        Utils::HostInf m_hostInf;
    public:
        CfgHost(Utils::PortNumber localPort, QString cfg) : m_localPort(localPort)
        {
            if(cfg.contains(':'))                    
            {
                QHostAddress addr(cfg.section(':', 0, 0)); 
                m_hostInf.Set(addr, cfg.section(':', 1, 1).toInt());
            }
            else
            {
                QHostAddress addr("127.0.0.1"); 
                m_hostInf.Set(addr, cfg.toInt());
            }
        }
        Utils::PortNumber localPotr() const {    return m_localPort; }
        const Utils::HostInf &hostInf() const {  return  m_hostInf; }
    };

   

    //// Nt-side, without tei-assignment routine
    //class NTsideForYate : public ISDN::IL2Profile
    //{
    //    ISDN::RightL2Role m_role;
    //    bool m_traceOn;
    //    bool m_traceRaw;

    //    NTsideForYate(bool traceOn, bool traceRaw)
    //        : m_traceOn(traceOn),
    //        m_traceRaw(traceRaw)
    //    {}

    //public:                

    //    void Set(ISDN::IsdnL2* pL2) const// override
    //    {
    //        ISDN::IsdnL2::Layer2 *l2 = pL2->getL2();
    //        l2->m_flag.FIXED_TEI = 1;
    //        l2->m_flag.ORIG = 0;
    //        l2->tei = CTei;//m_tei;
    //        l2->SetST_L2_4();            
    //        l2->ChangeMaxWindow(10);            
    //        l2->m_pRole = &m_role;            
    //    }

    //    static NTsideForYate& Instance(bool traceOn, bool traceRaw)
    //    {
    //        static NTsideForYate ret(traceOn, traceRaw);
    //        return ret;
    //    }

    //    std::string getLogSessionName() const
    //    {
    //        return m_role.Name();
    //    }        
    //    
    //    bool getTracedOn() const
    //    {
    //        return m_traceOn;
    //    }

    //    bool getTracedRaw() const
    //    {
    //        return m_traceRaw;
    //    }

    //private:

    //    enum { CTei = 0 };        

    //};

    //==========================================================================

    // TE-side, fixed tei = 0
    //class TEsideForYate : public ISDN::IL2Profile
    //{
    //    ISDN::LeftL2Role m_role;
    //    bool m_traceOn;
    //    bool m_traceRaw;

    //    TEsideForYate(bool traceOn, bool traceRaw)
    //        : m_traceOn(traceOn),
    //        m_traceRaw(traceRaw)
    //    {}

    //public:

    //    void Set(ISDN::IsdnL2* pL2) const// override
    //    {
    //        ISDN::IsdnL2::Layer2 *l2 = pL2->getL2();
    //        l2->m_flag.FIXED_TEI = 1; // запрет назначения TEI
    //        l2->m_flag.ORIG = 1;
    //        l2->tei = 0;            
    //        l2->SetST_L2_4();
    //        l2->ChangeMaxWindow(10);            
    //        l2->m_pRole = (&m_role);

    //        
    //    }

    //    static const TEsideForYate& Instance(bool traceOn, bool traceRaw)
    //    {
    //        static TEsideForYate ret(traceOn, traceRaw);
    //        return ret;
    //    }

    //    std::string getLogSessionName() const
    //    {
    //        return m_role.Name();
    //    }

    //    bool getTracedOn() const
    //    {
    //        return m_traceOn;
    //    }

    //    bool getTracedRaw() const
    //    {
    //        return m_traceRaw;
    //    }
    //};

    //--------------------------------------------------------------------------


    class YateL2TestProfile : public LogOutSettings
    {               
        CfgHost m_cfgHost;    
        boost::shared_ptr<L2Profile> m_profile;

    public:

        YateL2TestProfile(iLogW::LogStore& store)
            : LogOutSettings(true, "YateL2Test.log", &store),            
            m_cfgHost(5061, "192.168.0.158:5061"),
            m_profile(L2Profile::CreateAsTE( ISDN::LapdTraceOption(true) ))
       {
           m_profile->setTei(0);
           m_profile->setInitialState(false);
       }      
        
        bool getTraceOn()
        {
            return true;
        }

        bool getTraceRaw()
        {
            return false;
        }

        boost::shared_ptr<L2Profile> getL2Prof()
        {
            return m_profile;
            //return TEsideForYate::Instance(getTraceOn(), getTraceRaw());
        }

        const CfgHost& getCfgHost() const
        {
            return m_cfgHost;
        }
        
    };
} // namespace IsdnTest

#endif
