#include "stdafx.h"
#include "NObjCommonBfTaskProfile.h"
#include "Utils/QtHelpers.h"
#include "NObjBroadcastReceiver.h"
#include "Utils/VirtualInvoke.h"
#include "ResolveComAddr.h"



namespace 
{
    const bool CAlwaysComplete = false;
    //const bool CCout = false;
} // namespace 

namespace BfBootDRI
{    
    void NObjCommonBfTaskProfile::AllTaskFinished( const BfBootCli::TaskResult& result )
    {       
        if (CAlwaysComplete)
        {            
			*m_taskLog << "Stub! TaskFinished. ResultInfo: " << result.Info.c_str() << iLogW::EndRecord;
            std::cout << "Stub! TaskFinished. ResultInfo: " << result.Info.c_str() << std::endl;
            AsyncComplete(true);
            return;
        }
        
        QString logMsg;
        logMsg += "Current task finished with result: ";
        logMsg += result.ToString();

        Log(logMsg); 
        *m_taskLog << logMsg << iLogW::EndRecord;

        m_taskLog.reset();

        if (result.Ok && m_postValidator)
        {
            m_postValidator->StartWaiting(*this, 30000);
            Log("Wait broadcast after reboot...");
            return; //wait ExpectedMessageReceived           
        }

        if (m_progress.IsActive())
        {
            m_progress.Finish(result.Ok);
        }

        if (m_pNetTask && result.Ok)
        {
            RunNext();
        }
        else
        {
            UnlockChanges();
            AsyncComplete(result.Ok, result.Info.c_str());        
        }
    }

    // ------------------------------------------------------------------------------------

    void NObjCommonBfTaskProfile::ExpectedMessageReceived( const BfBootCore::BroadcastMsg& msg )
    {                
        bool tryNext = false;

        // state wait post validation
        if (m_postValidator) 
        {
            tryNext = true;            
            PutMsg(this, &NObjCommonBfTaskProfile::OnDeleteRec, m_postValidator);
            m_postValidator = 0;
        }

        // state wait addInfo
        if (m_addInfoReceiver)         
        {
            m_addr = msg.CbpAddress.Address().c_str();
            m_cbpPort = msg.BooterPort;
            
            if (m_cbpPort <= 0) ThrowRuntimeException("Wrong CbpPort in broadcast!");
            PutMsg(this, &NObjCommonBfTaskProfile::OnDeleteRec, m_addInfoReceiver);            
            m_addInfoReceiver = 0;
        }

        QString logMsg = 
            QString("Expected message received. \nMessage info:\n%1").arg(msg.Info().c_str());                    

        if (tryNext && m_pNetTask)
        {
            RunNext();
        }
        else
        {
            UnlockChanges();
            AsyncComplete(true, logMsg);             
        }
    }

    // ------------------------------------------------------------------------------------

    void NObjCommonBfTaskProfile::Info( QString eventInfo, int val )
    {        
        ESS_ASSERT(m_taskLog);

        //if (val >= 0) eventInfo.append(QString().setNum(val));        
        
        *m_taskLog << eventInfo;
        if (val != -1) *m_taskLog << val;
        *m_taskLog << iLogW::EndRecord;

        if (val == -1 && m_pNetTask == 0) Log(eventInfo);        

        if (val != -1)
        {
            if(!m_progress.IsActive()) m_progress.Start(100);
            m_progress.PrintStep(val);
        }
        
    }

    // ------------------------------------------------------------------------------------

    void NObjCommonBfTaskProfile::Log(QString msg)
    {
        LogProgressBar(msg.toStdString(), true);
    }

    // -------------------------------------------------------------------------------------

    void NObjCommonBfTaskProfile::LogProgressBar( const std::string& msg, bool eof )
    {
        AsyncOutput(msg.c_str(), eof);
        AsyncFlush();

        /*
        if (CCout && getDomain().Params().SuppressVerbose()) // ?
        {
            std::cout << msg;
            if (eof) std::cout << std::endl;
        }
        */
    }

    // ------------------------------------------------------------------------------------

    void NObjCommonBfTaskProfile::RunTask( DRI::IAsyncCmd *pAsyncCmd, ITask& task, const Domain::ObjectName& taskName, NObjBroadcastReceiver* postValidator)
    {        
        if (pAsyncCmd)
        {
            AsyncBegin(pAsyncCmd, boost::bind(&NObjCommonBfTaskProfile::AbortAsync, this));       
        }
        else
        {
            // это не первая задача
            ESS_ASSERT(AsyncActive());
        }

        m_postValidator = postValidator;       

        m_taskLog.reset(getDomain().LogCreator()->CreateSession(taskName.Name().toStdString(), m_traceClient));
        *m_taskLog << "RunTask" << iLogW::EndRecord;

        QString msg;
        msg += "Task ";
        msg += taskName.Name();
        msg += " started...";

        Log(msg);

        try
        {
            bool runAsLastTask;
            task.Run(runAsLastTask);
            if (!runAsLastTask) m_pNetTask = &task;
            else m_pNetTask = 0;
        }
        catch(const ESS::BaseException& e)
        {
            AsyncComplete(false, e.getTextMessage().c_str());
        }
    }

    // ------------------------------------------------------------------------------------

    QString NObjCommonBfTaskProfile::getBoardHostAddress()
    {        

        // Validate address format
        {
            Utils::HostInf host;
            host.Address(m_addr.toStdString()); // can throw
        }

        return m_addr;
    }

    // ------------------------------------------------------------------------------------

    BfBootCli::ITransportFactory& NObjCommonBfTaskProfile::getTransport()
    {
        ESS_ASSERT(!m_lockedBy.isEmpty());

        if (!m_transport)
        {
            int com;
            int comRate;
            if (ResolveComAddr(m_addr, com, comRate))             
                m_transport.reset(new BfBootCli::SbpUartCreator(com, comRate));            
            else 
            {
                ESS_ASSERT(m_cbpPort > 0);
                Utils::HostInf host(m_addr.toStdString(), m_cbpPort);
                m_transport.reset(new BfBootCli::SbpTcpCreator(host));
            }            
        }        
        
        return *m_transport;
    }

    // ------------------------------------------------------------------------------------

    bool NObjCommonBfTaskProfile::OnPropertyWrite( NamedObject *pObject, QString propertyName, QString val )
    {
        ESS_ASSERT(this == pObject);
        AssertUnlocked();

        // special case
        if (propertyName == "TraceClient") 
        {                
            m_prof.reset();
        }
        if (propertyName == "CbpPort")
        {
            if (val.toInt() <= 0) ThrowRuntimeException("CbpPort must be positive");
        }
        if (propertyName == "BoardAddress" || propertyName == "CbpPort")
        {
            m_transport.reset();
        }

        return true;
    }

    // ------------------------------------------------------------------------------------

    void NObjCommonBfTaskProfile::LockChanges(NamedObject& obj)
    {
        m_lockedBy = obj.Name().Name();
    }

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjCommonBfTaskProfile::ObtainCbpPort( DRI::IAsyncCmd *pAsyncCmd, int msec )
    {
        NObjBroadcastReceiver* addRec = BeginAddInfoObtain(pAsyncCmd);
        if (addRec == 0) return;
        addRec->m_srcAddress = getBoardHostAddress();
        addRec->m_cmpPort = 0;
        Log("ObtainCbpPort (wait broadcast)...");
        addRec->StartWaiting(*this, msec);
    }

    Q_INVOKABLE void NObjCommonBfTaskProfile::ObtainAddressByMac( DRI::IAsyncCmd *pAsyncCmd, QString mac, int msec /*= -1*/ )
    {        
        NObjBroadcastReceiver* addRec = BeginAddInfoObtain(pAsyncCmd);
        if (addRec == 0) return;
        addRec->m_mac = mac;
        Log("ObtainAddrByMac (wait broadcast)...");
        addRec->StartWaiting(*this, msec);
    }

    Q_INVOKABLE void NObjCommonBfTaskProfile::ObtainAddressByNum( DRI::IAsyncCmd *pAsyncCmd, int hwType, int hwNum, int msec /*= -1*/ )
    {
        NObjBroadcastReceiver* addRec = BeginAddInfoObtain(pAsyncCmd);    
        if (addRec == 0) return;
        addRec->m_hwType = hwType;
        addRec->m_hwNum = hwNum;
        Log("ObtainAddrByNum (wait broadcast)...");
        addRec->StartWaiting(*this, msec);
    }


    NObjBroadcastReceiver* NObjCommonBfTaskProfile::BeginAddInfoObtain( DRI::IAsyncCmd *pAsyncCmd )
    {
        ESS_ASSERT(m_lockedBy.isEmpty());
        ESS_ASSERT(!m_addInfoReceiver);
        AsyncBegin(pAsyncCmd);

        // if addr is COM ignore ?
        int dummy;
        if (ResolveComAddr(m_addr, dummy, dummy)) 
        {
            Log("ObtainCbp ignored (was set UART addr).");
            AsyncComplete(true);
            return 0; // ignore
        }

        m_transport.reset();        
        
        m_addInfoReceiver = new NObjBroadcastReceiver(this, "AddInfoReceiver");        
        return m_addInfoReceiver;        
    }

    // ------------------------------------------------------------------------------------

    BfBootCli::TaskMngProfile& NObjCommonBfTaskProfile::getMngProf()
    {
        if(!m_prof)
        {
            m_prof.reset(
                new BfBootCli::TaskMngProfile(getDomain(), *this, m_traceClient, 
				getDomain().LogCreator()));
        }
        return *m_prof;
    }

    // ------------------------------------------------------------------------------------

    bool NObjCommonBfTaskProfile::getTraceServer() const
    {
        return m_traceServer;
    }

    // ------------------------------------------------------------------------------------

    std::string NObjCommonBfTaskProfile::getPwd() const
    {
        return m_boardPwd.toStdString();
    }

    // ------------------------------------------------------------------------------------

    void NObjCommonBfTaskProfile::UnlockChanges()
    {
        m_lockedBy.clear();
    }

    // ------------------------------------------------------------------------------------

    void NObjCommonBfTaskProfile::AssertUnlocked()
    {
        if (!m_lockedBy.isEmpty()) ThrowRuntimeException("Object currently used in active task" + m_lockedBy.toStdString());
    }

    // ------------------------------------------------------------------------------------

    void NObjCommonBfTaskProfile::OnTimeout()
    {
        AsyncComplete(false, "Broadcast waiting timeout");
    }

    
    
} // namespace BfBootDRI

