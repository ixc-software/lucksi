#ifndef NOBJTESTECHOCHIPCTRL_H
#define NOBJTESTECHOCHIPCTRL_H

#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"

#include "IEchoChipCtrlOwner.h"
#include "iUart/Uart.h"
#include "EchoChipCtrl.h"

namespace TestRealEcho
{   
    class NObjEchoDataInit : public Domain::NamedObject, DRI::INonCreatable
    {
        Q_OBJECT;
        EchoApp::CmdInitData m_dataInit;

    public:
        NObjEchoDataInit( Domain::NamedObject *pParent, const Domain::ObjectName &name )
            : Domain::NamedObject(&pParent->getDomain(), name, pParent)
        {}
        
        bool getUseAlaw() const {return m_dataInit.UseAlaw;}
        void setUseAlaw(bool aLaw) {m_dataInit.UseAlaw = aLaw;}

        QString getVoiceChMask() const;
        void setVoiceChMask(QString mask);

        bool getT1Mode() const {return m_dataInit.T1Mode;}
        void setT1Mode(bool t1Mode) {m_dataInit.T1Mode = t1Mode;}

        //Q_PROPERTY(int ProtoVersion READ WRITE);
        Q_PROPERTY(bool UseAlaw READ getUseAlaw WRITE setUseAlaw);
        Q_PROPERTY(QString VoiceChMask READ getVoiceChMask WRITE setVoiceChMask);
        Q_PROPERTY(bool T1Mode READ getT1Mode WRITE setT1Mode);

        EchoApp::CmdInitData& get() {return m_dataInit;}
    };

    class NObjTestEchoChipCtrl
        : public Domain::NamedObject,                
        public IEchoChipCtrlOwner,
        EchoApp::IMainRecvSide
    {
        Q_OBJECT;        

        int m_com;
        int m_boudRate;
        NObjEchoDataInit *m_implDataInit;
        int m_echoTapsForAll; // -1 off
        bool m_EchoOn;

        //boost::scoped_ptr<EchoApp::IEchoRecvSide> m_chipCtrl;
        boost::scoped_ptr<EchoChipCtrl> m_chipCtrl;
        iCore::MsgTimer m_waitResp;
        iCore::MsgTimer m_processTimer;
        bool m_testErrorReport;
        
        boost::scoped_ptr<iUart::Uart> m_uart;

        boost::scoped_ptr<iLogW::LogSession> m_log;

        enum State
        {
            st_initial,
            st_waitInitResp,
            
            st_waitStats,
            st_sendReqByTimer,
            st_uartErrListen,

            st_complete
        };
        State m_state;
        

        void StartEcho(int taps, Platform::dword mask);
        EchoApp::IEchoRecvSide& EchoIntf() {return *m_chipCtrl;}

    // EchoApp::IMainRecvSide impl
    private:
        void OnEchoResponse(const EchoApp::EchoResp &resp); 
        void OnEchoStateResp( const EchoApp::EchoAppStats &stats);        
		void OnEchoAsyncEvent(const std::string &,int,const std::string &);
    // IEchoChipCtrlOwner impl:
    private:
        void Error(const std::string& err);
        EchoApp::IMainRecvSide& MainIntf();
        void Activated();

    private:
        void WaitTimeout(iCore::MsgTimer*);        
        //void StartEchoForAll
        void OnResetCtrl()
        {
            m_chipCtrl.reset();
        }
        void Process(iCore::MsgTimer*);
        void StartUartListen();
        
        void Trace(QString str, bool console = false);


    public:
        NObjTestEchoChipCtrl(Domain::IDomain *pDomain, const Domain::ObjectName &name);
        ~NObjTestEchoChipCtrl();
        
        Q_PROPERTY(int COM READ m_com WRITE m_com);
        Q_PROPERTY(int BodeRate READ m_boudRate WRITE m_boudRate);        
        Q_PROPERTY(int EchoTaps READ m_echoTapsForAll WRITE m_echoTapsForAll);
        Q_PROPERTY(bool TestErrorReport READ m_testErrorReport WRITE m_testErrorReport);
        
        // Q_PROPERTY obj.DataInit

        Q_INVOKABLE void Run(DRI::IAsyncCmd* pAsyncCmd, int timeout);
    };
} // namespace TestRealEcho

#endif
