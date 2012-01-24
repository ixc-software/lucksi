#include "stdafx.h"
#include "NObjTestEchoChipCtrl.h"

#include "EchoChipCtrl.h"
#include "Utils/IntSet.h"

#include "iUart/Uart.h"


namespace 
{  
    enum
    {        
        CSendRequestInterval = 900,
        CErrUartBaudeRate = 115200,
    }; 

} // namespace 

namespace TestRealEcho
{    
    void NObjEchoDataInit::setVoiceChMask( QString mask )
    {
        Utils::IntSet chSet(32);
        if (!chSet.Parse(mask.toStdString())) ThrowRuntimeException("Wrong mask");

        m_dataInit.VoiceChMask = 0;        
        for(int i = 0; i < chSet.Capacity(); ++i)
        {
            if (chSet.IsExist(i)) m_dataInit.VoiceChMask |= (1 << i);
        }        
    }

    QString NObjEchoDataInit::getVoiceChMask() const
    {
        std::stringstream ss;
        ss << std::hex << m_dataInit.VoiceChMask;
        return ss.str().c_str();
    }

    // ------------------------------------------------------------------------------------

    NObjTestEchoChipCtrl::NObjTestEchoChipCtrl( Domain::IDomain *pDomain, const Domain::ObjectName &name ) : Domain::NamedObject(pDomain, name),
        m_waitResp(this, &NObjTestEchoChipCtrl::WaitTimeout),
        m_processTimer(this, &NObjTestEchoChipCtrl::Process),
        m_log( getDomain().LogCreator()->CreateSession(Name().GetShortName().toStdString()) )
    {
        m_log->LogActive(true);
        getDomain().ThreadMonitor().Disable();

        m_implDataInit = new NObjEchoDataInit(this, "DataInit");
        m_state = st_initial;
        m_com = 0;
        m_boudRate = 38400;        
        m_echoTapsForAll = -1;
        m_testErrorReport = false;

        m_EchoOn = false;
    }

    NObjTestEchoChipCtrl::~NObjTestEchoChipCtrl()
    {
    }


    Q_INVOKABLE void NObjTestEchoChipCtrl::Run( DRI::IAsyncCmd* pAsyncCmd, int timeout )
    {        
        *m_log << "TestStarted." << iLogW::EndRecord;
        AsyncBegin(pAsyncCmd);                
        m_chipCtrl.reset(new EchoChipCtrl(getMsgThread(), *this, getDomain().LogCreator(), m_com, m_boudRate));                
        if (timeout > 0) m_waitResp.Start(timeout);    
    }

    void NObjTestEchoChipCtrl::OnEchoResponse( const EchoApp::EchoResp &resp )
    {        
        if (m_state != st_waitInitResp) 
        {
            Error("Response without init."); 
            return;
        }                        
      
        QString msg = QString("OnEchoResponse: %1").arg(resp.ToString().c_str());
        Trace(msg, true);        

        if (m_testErrorReport)
        {
            m_state = st_sendReqByTimer;
            m_processTimer.Start(CSendRequestInterval, true);
        }
        else
            m_state = st_waitStats;


        EchoIntf().OnCmdRequestState();                      
    }

    void NObjTestEchoChipCtrl::OnEchoStateResp( const EchoApp::EchoAppStats &stats )
    {           
        if (m_state != st_waitStats && m_state != st_sendReqByTimer)
        {
            Error("Unexpected OnEchoStateResp."); 
            return;
        }        

        QString msg = QString("OnEchoStateResp: %1").arg(stats.ToString().c_str());
        Trace(msg, true);
                
        if (m_echoTapsForAll > 0) 
        {
            StartEcho(m_EchoOn ? 0 : m_echoTapsForAll, m_implDataInit->get().VoiceChMask);  
            m_EchoOn = !m_EchoOn;
        }

        if (m_state == st_sendReqByTimer) return;
               
        EchoIntf().OnCmdShutdown();

        m_state = st_complete;
        Trace("Test Complete");
        AsyncComplete(true, "Test Complete");        
    }

	void NObjTestEchoChipCtrl::OnEchoAsyncEvent(const std::string &,int,const std::string &)
	{
		ESS_UNIMPLEMENTED;
	}
    
	void NObjTestEchoChipCtrl::Error( const std::string& err )
    {   
        if (m_state == st_sendReqByTimer) return;

        m_state = st_initial;
        m_waitResp.Stop();
        Trace(err.c_str());
        AsyncComplete(false, err.c_str());
    }

    EchoApp::IMainRecvSide& NObjTestEchoChipCtrl::MainIntf()
    {
        return *this;
    }

    void NObjTestEchoChipCtrl::Activated()
    {
        ESS_ASSERT(m_state == st_initial);
        //m_chipCtrl->SendUnknownCmd();

        EchoIntf().OnCmdInit(m_implDataInit->get());
        m_state = st_waitInitResp;
    }

    void NObjTestEchoChipCtrl::WaitTimeout( iCore::MsgTimer* )
    {
        if (!AsyncActive()) return;
        if (m_state == st_sendReqByTimer)
        {
            m_processTimer.Stop();
            OnResetCtrl();
            StartUartListen();
            return;
        }
        Error("Test timeout.");
    }    

    void NObjTestEchoChipCtrl::Process(iCore::MsgTimer*)
    {        
        if(m_state == st_sendReqByTimer && m_chipCtrl) 
        {
            EchoIntf().OnCmdRequestState();
            return;
        }
        
        if(m_state == st_uartErrListen)
        {            
            int size = m_uart->IO().AvailableForRead();                        
            if (size == 0) return;

            QByteArray buff(size, 0);            
            m_uart->IO().Read(buff.data(), buff.size());
            Trace(buff, true);            
        }
        return;
    }


    void NObjTestEchoChipCtrl::StartEcho(int taps, Platform::dword mask )
    {
        for(int i = 0; i < 32; ++i)
        {
            if (mask & (1 << i))
            {
                EchoIntf().OnCmdEcho(i, taps);
            }
        }
    }

    void NObjTestEchoChipCtrl::StartUartListen()
    {
        iUart::UartProfile profile = 
            iUart::UartProfile(m_com, CErrUartBaudeRate, iUart::DataBits_8);

        m_uart.reset(new iUart::Uart(profile));
        m_state = st_uartErrListen;
        m_processTimer.Start(CSendRequestInterval, true);
    }

    void NObjTestEchoChipCtrl::Trace( QString str, bool console )
    {
        *m_log << str << iLogW::EndRecord;
        if (!console) return;
        AsyncOutput(str);
        //AsyncFlush();
    }

} // namespace TestRealEcho
