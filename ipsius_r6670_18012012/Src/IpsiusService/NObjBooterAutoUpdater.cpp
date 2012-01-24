#include "stdafx.h"
#include "NObjBooterAutoUpdater.h"
#include "BfBootCli/TaskComposer.h"
#include "IpsiusService/BootImage.h"


namespace IpsiusService
{
    Platform::dword CTaskProcessPeriod = 500;

    class NObjBooterAutoUpdater::TaskUpdate :
        public BfBootCli::ITaskManagerReport,
        boost::noncopyable
    {        

        enum{CBroadcastTimeoutSec = 30};

        bool m_allTaskFinished;
        BfBootCore::BroadcastMsg m_msg;
        NObjBooterAutoUpdater& m_owner;
        const BfBootCore::IHwFirmwareImage& m_img;
        std::string m_pwd;
        boost::shared_ptr<BfBootCli::ITaskManager> m_task;

        QDateTime m_lastRecv;

        /*   
        enum State
        {
            stInitial,
            stWaitEnterBooter,
            stUpdate,
            stNotAwailable,
            stReady,
        };
        State m_state;
        QString ResolveState() const
        {
            //...
        }
        */
        

        void BeginUpdate(const BfBootCore::BroadcastMsg& msg)
        {   
            ESS_ASSERT(msg.IsBooter());
            const BfBootCli::TaskMngProfile prof(m_owner.getDomain(), *this, true, m_owner.Log().LogCreator());

            BfBootCli::SbpTcpCreator transportFactory(msg.CbpAddress);
            m_task = BfBootCli::TaskComposer::UpdateBootImg(
                prof, m_pwd, m_img, transportFactory);

            m_owner.LogDri(BoardInfo() + " BeginUpdate...");
        }

    // ITaskManagerReport:
    private:
        void AllTaskFinished(const BfBootCli::TaskResult& result)
        {
            m_allTaskFinished = true;
            m_owner.TaskFinished(this, result);
        }
        void Info(QString eventInfo, int val)
        {
            // noting            
        }

        void UpdateTs()
        {
            m_lastRecv = QDateTime::currentDateTime();
        }

    public:
        TaskUpdate(
            NObjBooterAutoUpdater& owner,
            const BfBootCore::BroadcastMsg& msg,
            const BfBootCore::IHwFirmwareImage& img,
            const std::string& pwd = ""
            ) 
            : m_allTaskFinished(false), m_msg(msg), m_owner(owner), m_img(img), m_pwd(pwd)
        {                        
            UpdateTs();
        }
        void TimeProcess()
        {
            QDateTime curr = QDateTime::currentDateTime();
            if (m_lastRecv.secsTo(curr) > CBroadcastTimeoutSec)
            {
                m_task.reset();
                AllTaskFinished(BfBootCli::TaskResult(false, "MsgTimout"));                
            }
        }
        void MsgReceived(const BfBootCore::BroadcastMsg& msg)
        {
            if (m_allTaskFinished) return;
            if (m_task) return;
            
            if (msg.HwNumber != m_msg.HwNumber || msg.HwType != m_msg.HwType) return;
            //if (msg.SrcAddress.Address() != m_msg.SrcAddress.Address()) return;

            UpdateTs();

            if (!msg.IsBooter()) 
            {
                if (m_task) return; // board in state UpdateBooter                

                m_owner.LogDri(BoardInfo() + " send cmd ResetDevice");

                m_owner.ResetDevice(msg);
                //ChangeState(waitBooter);
                return;
            }               
            BeginUpdate(msg);
        }
        QString BoardInfo() const
        {
            return QString("Num %1, Addr: %2").arg(m_msg.HwNumber).arg(m_msg.SrcAddress.Address().c_str());
        }
    };

    // ------------------------------------------------------------------------------------

    void NObjBooterAutoUpdater::Update(const BfBootCore::BroadcastMsg& msg)
    {      
         m_tasks.Add(new TaskUpdate(*this, msg, *m_bootImg));        
    }

    // ------------------------------------------------------------------------------------

    void NObjBooterAutoUpdater::MsgReceived( const BfBootCore::BroadcastMsg& msg )
    {
        for (int i = 0; i < m_tasks.Size(); ++i)
        {
            m_tasks[i]->MsgReceived(msg);
        }
    }

    // ------------------------------------------------------------------------------------

    void NObjBooterAutoUpdater::TaskFinished( const TaskUpdate* from, const BfBootCli::TaskResult& result )
    {
        //if (!result.Ok) return AllFinished(result.Ok, result.Info.c_str()); // ?
        QString msg = QString("Board %1 update finished %2")
            .arg(from->BoardInfo())
            .arg(result.Ok ? "successfully." : "unsuccessfully.");
        if (!result.Ok) msg = msg + "Additional info: " + result.Info.c_str();

        LogDri(msg);

        int index = m_tasks.Find(from);
        ESS_ASSERT(index != -1);
        
        //m_tasks.Delete(index);
        PutMsg(this, &NObjBooterAutoUpdater::OnDelMsg, m_tasks.Detach(index));

        if (m_tasks.IsEmpty()) AllFinished(true);
    }

    // ------------------------------------------------------------------------------------

    void NObjBooterAutoUpdater::AccumPeriodFinished( iCore::MsgTimer* p )
    {
        m_finder->Scan(*this, m_minVer);

        QString msg = QString("Scan finished: %1 boards was discovered.").arg(m_tasks.Size());
        LogDri(msg);                 
        for (int i = 0; i < m_tasks.Size(); ++i)
        {
            LogDri(m_tasks[i]->BoardInfo());
        }

        m_finder->SetEnabled(false);
        m_finder.Clear();

        if (m_tasks.Size() == 0) return AsyncComplete(true);

        // начало прослушивания сокета
        m_receiver.reset( new BfBootCli::BroadcastReceiver(getMsgThread(), *this, BfBootCore::CBroadcastClientPort) );
        m_taskProcess.Start(CTaskProcessPeriod, true);
    }

    // ------------------------------------------------------------------------------------

    void NObjBooterAutoUpdater::Process( iCore::MsgTimer* p )
    {
        for (int i = 0; i < m_tasks.Size(); ++i)
        {
            m_tasks[i]->TimeProcess();
        }
    }

    // ------------------------------------------------------------------------------------

    void NObjBooterAutoUpdater::ResetDevice( const BfBootCore::BroadcastMsg& msg )
    {
        m_receiver->ResetDevice(msg);
    }

    // ------------------------------------------------------------------------------------

    void NObjBooterAutoUpdater::AllFinished( bool ok, QString info /*= ""*/ )
    {
        m_tasks.Clear();
        m_bootImg.reset();
        m_receiver.reset();

        AsyncComplete(ok, info);
    }

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjBooterAutoUpdater::Run( DRI::IAsyncCmd* pCmd, int minVer, int accumMsec )
    {
        QString err;
        NObjHwFinder* pObj = getDomain().FindFromRoot<NObjHwFinder>("HwFinder", &err);
        if (pObj == 0) ThrowRuntimeException(err);                        
        m_finder = pObj; 

        m_minVer = minVer;

        try
        {
            m_bootImg.reset(new IpsiusService::BootImage(m_imgFile));
        }
        catch(const IpsiusService::BootImage::Err& err)
        {
            ThrowRuntimeException(err.getTextMessage());                
        }

        AsyncBegin(pCmd);

        m_finder->AllowUpdate(false);
        m_finder->SetEnabled(true);
        m_accumTimer.Start(accumMsec);

        LogDri("Wait HwFinder result ...");
    }

    // ------------------------------------------------------------------------------------

    void NObjBooterAutoUpdater::LogDri( QString line, bool lf /*= true*/ )
    {
        AsyncOutput(line, lf);
        AsyncFlush();
    }

    NObjBooterAutoUpdater::~NObjBooterAutoUpdater()
    {
    }

    NObjBooterAutoUpdater::NObjBooterAutoUpdater( Domain::IDomain *pDomain, const Domain::ObjectName &name ) : Domain::NamedObject(pDomain, name),
        m_accumTimer(this, &NObjBooterAutoUpdater::AccumPeriodFinished),
        m_taskProcess(this, &NObjBooterAutoUpdater::Process),
        m_minVer(0)
    {
    }
} // namespace IpsiusService
