#include "stdafx.h"
#include "NObjBfTaskSetParams.h"

namespace BfBootDRI
{

    Q_INVOKABLE void NObjBfTaskSetParams::Init( QString profile )
    {
        if (!m_prof.Init(profile)) ThrowRuntimeException(profile.append(" is unknown objectName"));       
    }

    // ------------------------------------------------------------------------------------

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjBfTaskSetParams::SetFactoryParams( int hwType, int hwNum, QString mac )
    {
        m_factorySettings.reset(
            new BfBootCore::DefaultParam(hwType, hwNum, mac.toStdString())
            );
    }

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjBfTaskSetParams::SetOptUdpLogAddr( QString val )
    {
        m_optSettings.UdpLogHost = val.toStdString();
    }

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjBfTaskSetParams::SetOptUdpLogPort( int port )
    {
        m_optSettings.UdpLogPort = port;
    }

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjBfTaskSetParams::SetOptUseTimestamp( bool use )
    {
        m_optSettings.UseTimestamp = use;
    }

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjBfTaskSetParams::SetOptCountToStore( int count )
    {
        m_optSettings.CountToStore = count;
    }

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjBfTaskSetParams::SetOptNetwork( bool useDhcp, QString ip, QString gateway, QString mask )
    {
        if (useDhcp)
        {
            m_optSettings.Network = E1App::BoardAddresSettings();
            return;
        }               

        m_optSettings.Network = 
            E1App::BoardAddresSettings(ip.toStdString(), gateway.toStdString(), mask.toStdString());

        if (!m_optSettings.Network.get().IsValid()) ThrowRuntimeException("Invalid network settings");
    }

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjBfTaskSetParams::SetOptUserPwd( QString pwd )
    {
        m_optSettings.UserPwd = pwd.toStdString();
    }

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjBfTaskSetParams::SetOptWaitLoginMsec( int msec )
    {
        m_optSettings.WaitLoginMsec = msec;
    }

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjBfTaskSetParams::SetOptCmpAppPort( int port )
    {
        m_optSettings.CmpPort = port;
    }

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjBfTaskSetParams::Run( DRI::IAsyncCmd *pAsyncCmd )
    {
        NObjBroadcastReceiver* receiver = 0;                        
        
        bool setDhcp = m_optSettings.Network.IsInited() && m_optSettings.Network.get().DHCP();        
        
        if (m_checkAfter && setDhcp && !m_factorySettings)
        {
            m_waitView = true;
        }
        else
        {
            if (m_checkAfter) receiver = CreateReceiverForValidation();
        }
        

        m_prof->RunTask(pAsyncCmd, *this, Name(), receiver);        
    }

    // ------------------------------------------------------------------------------------

    void NObjBfTaskSetParams::Run(bool& isLastTask)
    {
        // если нужно добавить шаг получения HwNum, тк не достаточно
        if (m_waitView)
        {
            ESS_ASSERT(!m_task);         
            ESS_ASSERT(!m_waitBoardParams);
            isLastTask = false;

            m_waitBoardParams.reset(new std::vector<BfBootCore::ConfigLine>);

            m_task = BfBootCli::TaskComposer::ViewBoardParams(
                m_prof->getMngProf(), 
                m_prof->getPwd(),                
                m_prof->getTransport(), 
                m_waitBoardParams,
                m_prof->getTraceServer()
                );

            return;
        }

        isLastTask = true;
        if (m_factorySettings)
        {
            m_factorySettings->OptionalSettings = m_optSettings;
            m_task = BfBootCli::TaskComposer::SetParams(
                m_prof->getMngProf(),
                m_prof->getTransport(),
                m_prof->getPwd(),
                *m_factorySettings,
                m_useAutoReset,
                m_prof->getTraceServer()
                );
        }
        else
        {
            m_task = BfBootCli::TaskComposer::SetParams(
                m_prof->getMngProf(), 
                m_prof->getTransport(),
                m_prof->getPwd(),
                m_optSettings, 
                m_prof->getTraceServer()
                );
        }
    }

    // ------------------------------------------------------------------------------------

    
    void NObjBfTaskSetParams::OnRunNext()
    {
        ESS_ASSERT(m_waitView);
        ESS_ASSERT(m_task);         
        ESS_ASSERT(m_waitBoardParams);

        int hwType;
        int hwNum;

        NObjBroadcastReceiver* receiver = 0;

        if (ExtractParamsByName("HwType", hwType) && ExtractParamsByName("HwNumber", hwNum))
        {
            receiver = new NObjBroadcastReceiver(this, "CompleteValidator");
            receiver->m_hwType = hwType;
            receiver->m_hwNum = hwNum;
        }

        m_task.reset();
        m_waitBoardParams.reset();

        m_waitView = false;
        m_prof->RunTask(0, *this, Name(), receiver);  //запуск второго (основного) этапа
    }

    bool NObjBfTaskSetParams::ExtractParamsByName( const std::string& name, int& res )
    {
        ESS_ASSERT(m_waitBoardParams);
        for (int i = 0; i < m_waitBoardParams->size(); ++i)
        {
            if (m_waitBoardParams->at(i).Name() == name)
            {
                std::istringstream ss;
                ss.str(m_waitBoardParams->at(i).Value());
                ss >> res;
                if (!ss.fail()) return true;
            }           
        }
        return false;
    }

    void NObjBfTaskSetParams::RunNext()
    {
        PutMsg(this, &NObjBfTaskSetParams::OnRunNext);
    }

    // ------------------------------------------------------------------------------------

    NObjBroadcastReceiver* NObjBfTaskSetParams::CreateReceiverForValidation()
    {
        NObjBroadcastReceiver* receiver = new NObjBroadcastReceiver(this, "CompleteValidator");

        if (m_factorySettings)
        {
            receiver->m_mac = m_factorySettings->Mac.c_str();
            receiver->m_hwType = m_factorySettings->HwType;
            receiver->m_hwNum = m_factorySettings->HwNumber;
            //receiver->m_softNum = BfBootCore::BroadcastMsg::NoRelNum; - теперь пользовательские настройки не сбрасываем
        }            

        if (m_optSettings.Network.IsInited() && !m_optSettings.Network.get().DHCP())
        {
            receiver->m_srcAddress = m_optSettings.Network.get().IP().c_str();
        }
        else 
        {
            if (!m_optSettings.Network.IsInited()) receiver->m_srcAddress = m_prof->getBoardHostAddress();
        }
        return receiver;
    }

    // ------------------------------------------------------------------------------------

    NObjBfTaskSetParams::NObjBfTaskSetParams( Domain::IDomain *pDomain, const Domain::ObjectName &name ) 
        : Domain::NamedObject(pDomain, name),
        m_prof(*this),
        m_useAutoReset(true),
        m_checkAfter(false),
        m_waitView(false)
    {
    }

    
  

    
    

} // namespace BfBootDRI

