#include "stdafx.h"
#include "NObjBroadcastReceiver.h"
#include "iLog/LogManager.h"
#include "Utils/QtHelpers.h"

#include "IBroadcastReceiverReport.h"
#include "ResolveComAddr.h"


namespace 
{
    // used for debug
    const bool CAlwaysComplete = false;
    const bool CSendSelf = false;

    
    //const bool CCout = true;
} // namespace 

namespace BfBootDRI
{

    void NObjBroadcastReceiver::MsgReceived( const BfBootCore::BroadcastMsg& msg )
    {
        if (!m_active) return;

        if (CAlwaysComplete) return WaitingComplete(msg);

        // Filter msg:
        if (!IsSatisfy(msg)) return;

        // Process filtered msg:
        QString logMsg = 
            QString("Expected message received. \nMessage info:\n%1").arg(msg.Info().c_str());            
        Log(logMsg);

        if (m_resetAfter && !msg.IsBooter()) 
        {                                
            Log("Send cmd ResetDevice");
            m_receiver->ResetDevice(msg);
        }
        WaitingComplete(msg);        
    }

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjBroadcastReceiver::StartWaiting( DRI::IAsyncCmd *pAsyncCmd, bool resetAfter, int maxMsec /*= -1*/ )
    {           
        // need for .isc where using Ip & Com transport         
        int dummy;
        if (m_srcAddress.IsInited() && ResolveComAddr(m_srcAddress, dummy, dummy)) return;

        AsyncBegin(pAsyncCmd, boost::bind(&NObjBroadcastReceiver::AbortAsync, this));            
        
        Log("StartWaiting");
        m_resetAfter = resetAfter;


        Run(maxMsec);        
    }

    // ------------------------------------------------------------------------------------

    // External
    NObjBroadcastReceiver::NObjBroadcastReceiver( Domain::IDomain *pDomain, const Domain::ObjectName &name ) : Domain::NamedObject(pDomain, name),
        m_active(false),
        m_resetAfter(false),
        m_waitingTimeout(this, &T::OnTimeout),
        m_pRefObserver(0)
    {}

    // ------------------------------------------------------------------------------------

    //Internal
    NObjBroadcastReceiver::NObjBroadcastReceiver( Domain::NamedObject *pParent, const Domain::ObjectName &name/*, IBroadcastReceiverReport& observer*/) 
        : Domain::NamedObject(&pParent->getDomain(), name, pParent),
        m_active(false),
        m_resetAfter(false),
        m_waitingTimeout(this, &T::OnTimeout),
        m_pRefObserver(0) //(&observer)
    {
    }

    void NObjBroadcastReceiver::StartWaiting( IBroadcastReceiverReport& observer, int maxMsec )
    {        
        m_pRefObserver = &observer;
        m_resetAfter = false;        
        Run(maxMsec);
    }

    // ------------------------------------------------------------------------------------

    void NObjBroadcastReceiver::Log( QString msg )
    {
        msg = Name().Name() + ": " + msg;
        
        if (AsyncActive()) AsyncOutput(msg);        
    }

    // ------------------------------------------------------------------------------------

    void NObjBroadcastReceiver::WaitingComplete( const BfBootCore::BroadcastMsg& msg )
    {
        m_waitingTimeout.Stop();
        m_active = false; // synchro stop
        //if (m_resetAfter) m_receiver->ResetDevice(msg);
        PutMsg(this, &T::OnDeleteReceiver);                 

        m_lastFilteredMsg.reset( new BfBootCore::BroadcastMsg(msg) );

        if (m_pRefObserver)
        {
            IBroadcastReceiverReport* p = m_pRefObserver;
            m_pRefObserver = 0;
            p->ExpectedMessageReceived(msg);
            return;
        }
        AsyncComplete(true);
    }

    // ------------------------------------------------------------------------------------

    void NObjBroadcastReceiver::Run( int maxMsec )
    {
        if (m_mac.IsInited())
        {
            QString mac(m_mac);        
            m_mac = CCompareMacInLower ? mac.toLower() : mac.toUpper();
        }        

        m_lastFilteredMsg.reset();

        if (maxMsec != -1) m_waitingTimeout.Start(maxMsec);
        m_receiver.reset(new BfBootCli::BroadcastReceiver(getMsgThread(), *this, BfBootCore::CBroadcastClientPort));            
        m_active = true;

        if (CSendSelf) PutMsg(this, &T::MsgReceived, BfBootCore::BroadcastMsg());
    }

    // ------------------------------------------------------------------------------------

    void NObjBroadcastReceiver::OnDeleteReceiver()
    {
        m_receiver.reset();
    }

    // ------------------------------------------------------------------------------------

    void NObjBroadcastReceiver::OnTimeout( iCore::MsgTimer* p )
    {
        ESS_ASSERT(p = &m_waitingTimeout);    
        if (m_pRefObserver) m_pRefObserver->OnTimeout();        
        else AsyncComplete(false, "Expected broadcast message waiting timeout.");
    }

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjBroadcastReceiver::ViewLastWaitingResult( DRI::IAsyncCmd *pAsyncCmd )
    {
        AsyncBegin(pAsyncCmd, boost::bind(&NObjBroadcastReceiver::AbortAsync, this));

        if (m_lastFilteredMsg) Log( m_lastFilteredMsg->Info().c_str() );
        else Log("Don`t have actual mesage.");
    }

    int NObjBroadcastReceiver::Read( const Utils::InitProp<int>& prop )
    {
        if (!prop.IsInited()) return -1;
        return prop;
    }

    QString NObjBroadcastReceiver::Read( const Utils::InitProp<QString>& prop )
    {
        if (!prop.IsInited()) return "Don`t set";
        return prop;
    }

    void NObjBroadcastReceiver::setMac(QString mac)
    {
        m_mac = CCompareMacInLower ? mac.toLower() : mac.toUpper();
    }
    
} // namespace BfBootDRI

