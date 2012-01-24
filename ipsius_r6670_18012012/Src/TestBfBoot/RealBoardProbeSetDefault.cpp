#include "stdafx.h"
#include "RealBoardProbeSetDefault.h"

#include "iCore/ThreadRunner.h"
#include "iLog/LogManager.h"
#include "BfBootCli/ITaskManagerReport.h"
#include "BfBootCli/TaskComposer.h"

#include "Domain/DomainClass.h"
#include "Domain/DomainTestRunner.h"

#include "BoardProbeConsts.h"
#include "BfBootCli/TransportFactory.h"
#include "BfBootCore/GeneralBooterConst.h"

namespace
{
    using namespace BfBootCli;
    using namespace TestBfBoot;

    class SetDefault
        : public ITaskManagerReport
    {
    public:
        struct Prof
        {            
            bool TraceActive;
            std::string Prfix;
            int Com;          
        };
        SetDefault(Domain::DomainClass& domain, const Prof& prof)
            : m_domain(domain)
        {
            //iLogW::ILogSessionCreator& logCreate = m_logMng; 
            m_log.reset(m_domain.Log().CreateLogSesion("HwCatcher", prof.TraceActive, prof.Prfix));   

            BfBootCli::TaskMngProfile taskProf(m_domain, *this, prof.TraceActive, prof.Prfix);
            BfBootCli::DefaultParam param(CHwType, CHwNumber, CMac);
            param.Mac = CMac;
            param.HwType = CHwType;
            param.HwNumber = CHwNumber;
            param.OptionalSettings.Network = E1App::NetworkSettings(CBoardIP, CBoardGateway, CBoardNetMask);

            BfBootCli::SbpUartCreator transport(prof.Com, BfBootCore::CComRate);

            m_task = BfBootCli::TaskComposer::SetParams(taskProf, transport, param, false);                               
        }

    // ITaskMngReport
    private:
        void AllTaskFinished (const BfBootCli::TaskResult& result)
        {
            if (result.Ok)
            {
                *m_log << "Task complete." << iLogW::EndRecord;
                //m_runner.getCompletedFlag().Set(true);
                m_domain.Stop(Domain::DomainExitOk);
            }
            else
            {
                *m_log << "Task failed." << result.Info << iLogW::EndRecord;
                TUT_ASSERT(0 && "TaskFailed");
            }
        }

        void Info(QString eventInfo, int progress = -1)
        {}
        
    private:
        Domain::DomainClass& m_domain;        
        boost::scoped_ptr<iLogW::LogSession> m_log; 
        boost::shared_ptr<BfBootCli::ITaskManager> m_task;
    };
}

namespace TestBfBoot
{
    void RealBoardProbeSetDefault()
    {
        SetDefault::Prof settings;
        iLogW::LogSettings log;

        log.CountSyncroToStore(1);
        log.out().Cout().TraceInd = true;
        
        log.out().Udp().DstHost = Utils::HostInf ("192.168.0.144", 56001);

        settings.Com = 4;
        settings.Prfix = "RealBoardProbeSetDefault";
        settings.TraceActive = true;

        Domain::DomainTestRunner runner(log, -1);  
        runner.Run<SetDefault>(settings);
    }
} // namespace TestBfBoot
