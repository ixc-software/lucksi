#ifndef NOBJDIRECTCALL_H
#define NOBJDIRECTCALL_H

#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "DirectCall.h"

namespace Dss1ToSip	{ class NObjDss1Interface; }

namespace TestRealEcho
{

    using boost::scoped_ptr;
    class DirectCall;

    // DirectCall wrapper
    class NObjDirectCall 
        : public Domain::NamedObject,        
        public IDirectCallToOwner
    {
        Q_OBJECT;

        Utils::SafeRef<Dss1ToSip::NObjDss1Interface> m_ctrl;
        iCore::MsgTimer m_tDuration;
        iCore::MsgTimer m_tReadyPolling;
        int m_waitReadyTimout;

        //this fields exist only if enabled 
        scoped_ptr<iRtp::RtpPcInfra> m_rtpInfra;
        scoped_ptr<DirectCall> m_call;

        // profile:        
        int m_minRtpPort;
        int m_maxRtpPort;
        QString m_localHost;
        int m_channel;        
        QString m_outFile;
        bool m_waveOut;
        iMedia::Codec m_codec;
        scoped_ptr<iRtpUtils::IRtpTestFactory> m_source;
        
        QString CodecInfo() const;
        void OnEndDuration(iCore::MsgTimer*);

        void OnCtrlIsReady(iCore::MsgTimer*);

    // IDirectCallToOwner impl:
    private:
        void ErrorInd(QString errInfo);


    public:        
        NObjDirectCall(Domain::IDomain *pDomain, const Domain::ObjectName &name);
        Q_INVOKABLE void Init(QString objName);

        ~NObjDirectCall();        

        // general 
        Q_PROPERTY(int MinRtpPort READ m_minRtpPort WRITE m_minRtpPort);
        Q_PROPERTY(int MaxRtpPort READ m_maxRtpPort WRITE m_maxRtpPort);
        Q_PROPERTY(QString LocalHost READ m_localHost WRITE m_localHost);

        // call
        Q_PROPERTY(int Channel READ m_channel WRITE m_channel);
        Q_PROPERTY(QString OutFile READ m_outFile WRITE m_outFile);        
        Q_PROPERTY(bool WaveOut READ m_waveOut WRITE m_waveOut);
        Q_INVOKABLE void SetCodec(QString name);
        Q_PROPERTY(QString Codec READ CodecInfo);
        Q_INVOKABLE void SetSourceFile(QString file, bool isWave);
        Q_INVOKABLE void SetSourceGen(int amplitude, int frec); 
        Q_INVOKABLE void WaitInterfaceReady(DRI::IAsyncCmd* pAsyncCmd, int timeout);
        Q_INVOKABLE void Run(DRI::IAsyncCmd* pAsyncCmd, int duration);                      

    };
} // namespace TestRealEcho

#endif


/*
ScriptExample:
ObjCreate NObjDirectCall objCall true
    .Init("bfCtrlObj");
    
    //optional settings
    .MinRtpPort = 1000
    .MaxRtpPort = 10000
    .Channel = 0
    .OutFile = "../TestResult/RecWave.bin"
    .SetCodec("PCMCA", 0, 8000) // используется источником сигнала
    .SetSourceFile("file") || .SetSourceGen(1, 600) amplitude in percent

    .WaitInterfaceReady(30000)

    //Start
    .Run(120000)

*/
