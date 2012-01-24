#include "stdafx.h"
#include "RealBoardProbe.h"

#include "BfBootCli/BroadcastReceiver.h"

#include "iCore/MsgTimer.h"
#include "Domain/DomainClass.h"
#include "Domain/DomainTestRunner.h"

#include "iLog/LogManager.h"
#include "BfBootCli/ITaskManagerReport.h"
#include "BfBootCli/TaskComposer.h"
#include "BfBootCore/ScriptList.h"
#include "BfBootCore/ScriptCmdList.h"

#include "BoardProbeConsts.h"
#include "BfBootCore/GeneralBooterConst.h"
#include "TestBfBoot/FirmwareEmul.h"

#include "Utils/QtHelpers.h"

namespace 
{
    using  boost::shared_ptr;
    using  boost::scoped_ptr;
    
    // базовый тип действий выполняемый при обработке бродкаста
    class Action : public BfBootCli::IBroadcastReceiverToOwner
    {
        BfBootCli::TaskMngProfile* m_pTaskProf;
        shared_ptr<BfBootCli::ITaskManager> m_task;

    protected:
        Action() : m_pTaskProf(0) {}
        BfBootCli::TaskMngProfile& getProfile()
        {
            ESS_ASSERT(m_pTaskProf);
            return *m_pTaskProf;
        }        
        void SetTask(shared_ptr<BfBootCli::ITaskManager> pNewTask)
        {
            ESS_ASSERT(!m_task && pNewTask);
            m_task = pNewTask;
        }

    public:
        void DestroyTask()
        {
            m_task.reset();
        }
        void Set(BfBootCli::TaskMngProfile& prof)
        {
            m_pTaskProf = &prof;
        }
        bool TaskAssigned() const {return m_task;}
        BfBootCli::ITaskManager& getTaskMng()
        {
            ESS_ASSERT(m_task);
            return *m_task;
        }
    };

    // ------------------------------------------------------------------------------------

    class ActionFwUpdate : public Action
    {
        shared_ptr<BfBootCore::IHwFirmware> m_fwu;
        bool m_traceServer;

    // IBroadcastReceiverToOwner
    private:
        void MsgReceived(const BfBootCore::BroadcastMsg& msg)
        {
            if(TaskAssigned() || msg.BooterPort == 0) return;
           
            SetTask( BfBootCli::TaskComposer::UpdateFirmware(getProfile(), BfBootCli::SbpTcpCreator(msg.CbpAddress), TestBfBoot::CBoardPwd, *m_fwu, true) );
            //SetTask( BfBootCli::TaskComposer::UpdateFirmware(getProfile(), BfBootCli::SbpUartCreator(4), TestBfBoot::CBoardPwd, *m_fwu, true) );
        }
    public:
        ActionFwUpdate(shared_ptr<BfBootCore::IHwFirmware> fwu, bool traceServer)
            : m_fwu(fwu),
            m_traceServer(traceServer)
        {
            ESS_ASSERT(fwu);  
            ESS_ASSERT( BfBootCore::ScriptCmdList::Validate( m_fwu->Script(0).Value(),  m_fwu->ImageCount()) );
        }
    };

    // ------------------------------------------------------------------------------------

    class ActionBootImgUpdate : public Action
    {
        shared_ptr<BfBootCore::IHwFirmwareImage> m_img;
        bool m_traceServer;
    
    // IBroadcastReceiverToOwner
    private:
        void MsgReceived(const BfBootCore::BroadcastMsg& msg)
        {
            if(TaskAssigned() || msg.BooterPort == 0) return;
            //BfBootCli::SbpTcpCreator trnsport(msg.CbpAddress);
            BfBootCli::SbpTcpCreator trnsport(Utils::HostInf("192.168.0.142", 1111));
            SetTask( BfBootCli::TaskComposer::UpdateBootImg(getProfile(), TestBfBoot::CBoardPwd, *m_img, trnsport, m_traceServer) );            
        }

    public:
        ActionBootImgUpdate(BfBootCore::IHwFirmwareImage* pImg, bool traceServer)
            : m_img(pImg),
            m_traceServer(traceServer)
        {
            ESS_ASSERT(pImg);
        }
    };

    // ------------------------------------------------------------------------------------

    struct CatcherProf 
    {            
        std::string Prfix;
        bool TraceActive;  
        shared_ptr<Action> RcvAction;
        
        CatcherProf(Action* newAction)
            : RcvAction(newAction), TraceActive(false)
        {
            ESS_ASSERT(newAction);            
        }
    };

    // Ожидает широковещательного сообщения на порту CBcListenPort от платы CHwNumber, CHwType.
    // Создает задание обновления, ожидает завершения.
    class HwCatcher : 
        public iCore::MsgObject,        
        public BfBootCli::ITaskManagerReport
    {
        CatcherProf m_prof;
        //iCore::IThreadRunner& m_runner;
        Domain::DomainClass& m_domain;        
        boost::scoped_ptr<iLogW::LogSession> m_log;
        BfBootCli::BroadcastReceiver m_bcReceiv;

        //boost::scoped_ptr<BfBootCli::ITaskManager> m_task;
        BfBootCli::TaskMngProfile m_taskProf;
        
        iCore::MsgTimer m_poolTimer;

        void OnPooling(iCore::MsgTimer*)
        {            
            if (m_log->LogActive() && m_prof.RcvAction->TaskAssigned()) 
            {
                *m_log << m_prof.RcvAction->getTaskMng().StateInfo() << iLogW::EndRecord;
            }            
        }      

        void Close()
        {            
            m_prof.RcvAction->DestroyTask();
            m_domain.Stop(Domain::DomainExitOk);
        }
  
    // ITaskMngReport
    private:

        
        void AllTaskFinished (const BfBootCli::TaskResult& result)
        {
            if (result.Ok)
            {
                *m_log << "Task complete." << iLogW::EndRecord;                
                PutMsg(this, &HwCatcher::Close);                                               
            }
            else
            {
                *m_log << "Task failed." << result.Info << iLogW::EndRecord;
                PutMsg(this, &HwCatcher::Close);
                TUT_ASSERT(0 && "TaskFailed");
            }
        }     

        void Info(QString eventInfo, int progress = -1)
        {            
            *m_log << m_tagTaskEvent << eventInfo;
            if (progress != -1) *m_log << " - " << progress;
            *m_log << iLogW::EndRecord;            

            /*
            std::cout << eventInfo;
            if (progress != -1) std::cout << " - " << progress;
            std::cout << std::endl;
            */

        }

    public:
        HwCatcher(Domain::DomainClass& domain, const CatcherProf& prof)
            : iCore::MsgObject(domain.getMsgThread()),
            m_prof(prof),
            m_domain(domain),            
            m_bcReceiv(domain.getMsgThread(), *m_prof.RcvAction, BfBootCore::CBroadcastClientPort),
            m_taskProf(domain, *this, prof.TraceActive, prof.Prfix),
            m_poolTimer(this, &HwCatcher::OnPooling)     
        {           
            iLogW::ILogSessionCreator& logCreate = m_domain.Log(); 
            m_log.reset(logCreate.CreateLogSesion("HwCatcher", prof.TraceActive, prof.Prfix));                        

            m_prof.RcvAction->Set(m_taskProf);

            *m_log << "Wait broadcast ..." << iLogW::EndRecord;
            m_poolTimer.Start(2000, true);
        }
    };

    void RunHwCatcher(const CatcherProf& prof)
    {
        iLogW::LogSettings log;
        log.CountSyncroToStore(1);
        log.TimestampInd(true);
        log.out().Cout().TraceInd = true;
        
        log.out().Udp().DstHost = Utils::HostInf ("192.168.0.144", 56001);
        log.out().Udp().TraceInd = true;

        Domain::DomainTestRunner test(log, -1);

        test.Run<HwCatcher>(prof);
    }
    

    // stub!
    class SendLog : public iCore::MsgObject
    {
        //Domain::DomainClass& m_domain;        
        boost::scoped_ptr<iLogW::LogSession> m_log;
        iCore::MsgTimer m_t;

        void OnSend(iCore::MsgTimer*)
        {
            *m_log << "MsgBody." << iLogW::EndRecord;
        }

    public:

        struct Prof{};
        SendLog(Domain::DomainClass& domain, const Prof&)            
            : MsgObject(domain.getMsgThread()),
            m_log( domain.Log().CreateLogSesion("Probe", true, "LogSender") ),
            m_t(this, &SendLog::OnSend)
        {
            m_t.Start(1000, true);
        }
    };

} // namespace 



namespace TestBfBoot
{
    // stub!
    void SendLogFn(const std::string& addr)
    {
        iLogW::LogSettings log;
        log.CountSyncroToStore(1);
        log.TimestampInd(true);
        log.out().Cout().TraceInd = true;
        
        log.out().Udp().DstHost = Utils::HostInf (addr, 56001);
        log.out().Udp().TraceInd = true;

        Domain::DomainTestRunner test(log, -1);
        test.Run<SendLog>(SendLog::Prof());
    }

    void RealUpdateBootImg(QString filename)
    {                   
        QFile file(filename);
        ESS_ASSERT( file.open(QIODevice::ReadOnly) );
        QByteArray buff;
        buff = file.readAll();        

        bool traceServer = true;
        CatcherProf prof( new ActionBootImgUpdate( new BfBootCli::BinaryImage(filename.toStdString(), buff), traceServer ) ); 
        prof.Prfix = "RealUpdateBootImg";
        prof.TraceActive = true;  

        RunHwCatcher(prof);
    }


    void RealUpdateFw()
    {          
        shared_ptr<FirmwareEmul> fwu(new FirmwareEmul());

        fwu->AddDataFromFile("HWTestEcho.bin");  // idx 0
        fwu->AddDataFromFile("HWTestSingle.bin"); // idx 1                
        //fwu->AddDataFromFile("BfBoot.ldr"); // idx 1                

        fwu->AddScript(BfBootCore::NamedScript::NameOfBootScript(), "Load(SPI, 0); Load(MAIN, 1)");
        fwu->SetRelease(1);
        
        bool traceServer = true;
        CatcherProf prof( new ActionFwUpdate(fwu, traceServer) ); 

        prof.Prfix = "CatchRealBoardPrb";
        prof.TraceActive = true;                     

        RunHwCatcher(prof);                     
    }

   

    
} // namespace TestBfBoot
