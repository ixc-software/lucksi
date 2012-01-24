#include "stdafx.h"
#include "NObjDirectCall.h"
#include "DirectCall.h"
#include "Dss1ToSip/NObjDss1Interface.h"
#include "iRtpUtils/GenerateSinus.h"
#include "iRtpUtils/PlayFile.h"
#include "iRtpUtils/WriteToFile.h"
#include "Utils/GetDefLocalHost.h"

namespace 
{
    const int CPollInterval = 300;
} // namespace 

namespace TestRealEcho
{

    NObjDirectCall::NObjDirectCall( Domain::IDomain *pDomain, const Domain::ObjectName &name ) : Domain::NamedObject(pDomain, name),
        m_codec(iMedia::Codec::ALaw_8000),
        m_tDuration(this, &NObjDirectCall::OnEndDuration),
        m_tReadyPolling(this, &NObjDirectCall::OnCtrlIsReady)        
    {
        m_minRtpPort = 1000;
        m_maxRtpPort = 10000;
        m_localHost = Utils::GetDefLocalHost().toString();

        m_channel = 0;
        m_outFile = "RecRtpData.bin";
        m_waveOut = false;
        
        SetSourceGen(1, 600);
    }

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjDirectCall::Init( QString objName )
    {
        QString err;
        Dss1ToSip::NObjDss1Interface *p = getDomain().FindFromRoot<Dss1ToSip::NObjDss1Interface>(objName, &err);
        if (p == 0) ThrowRuntimeException(err); 
        m_ctrl = Utils::SafeRef<Dss1ToSip::NObjDss1Interface>(p);        
    }

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjDirectCall::SetCodec( QString name )
    {
        const iMedia::Codec* pCodec = iMedia::Codec::getStaticCodec(name.toStdString() );
        if (!pCodec) ThrowRuntimeException("Unknown codec");
        m_codec = *pCodec;
    }

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjDirectCall::SetSourceFile( QString file, bool isWave/* /Raw */ )
    {
        m_source.reset(new iRtpUtils::PlayFileFactory(file, isWave));
    }

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjDirectCall::SetSourceGen( int amplitude, int frec )
    {
        m_source.reset( new iRtpUtils::GenerateSinusFactory(amplitude, frec) );
    }

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjDirectCall::Run( DRI::IAsyncCmd* pAsyncCmd, int duration )
    {
        if (!m_ctrl->IsConnectionActive()) ThrowRuntimeException("Interface not ready!");

        if (m_codec.getName() != "PCMA" && m_codec.getName() != "PCMU") ThrowRuntimeException("Not supported codec");

        AsyncBegin(pAsyncCmd);                            

        iRtp::RtpInfraParams par(m_localHost.toStdString(), m_minRtpPort, m_maxRtpPort);
        m_rtpInfra.reset(new iRtp::RtpPcInfra(getMsgThread(), par));

        iRtpUtils::WriteToFileFactory receiver(m_outFile, m_waveOut);

        m_call.reset(
                    new DirectCall(
                    getMsgThread(),
                    *this,
                    Log(),
                    *m_rtpInfra,
					m_channel,
                    m_ctrl,
                    Name().Name(),
                    *m_source,
                    receiver,
                    m_codec) 
                    );                    

        m_tDuration.Start(duration);
    }

    // ------------------------------------------------------------------------------------

    NObjDirectCall::~NObjDirectCall()
    {
    }

    // ------------------------------------------------------------------------------------

    QString NObjDirectCall::CodecInfo() const
    {
        return QString("Name: %1 Payload: %2 Rate: %3")
            .arg(m_codec.getName().c_str())
            .arg(m_codec.getPayloadType())
            .arg(m_codec.getRate());
    }

    // ------------------------------------------------------------------------------------

    void NObjDirectCall::OnEndDuration( iCore::MsgTimer* )
    {
        AsyncComplete(true);
        m_call.reset();
        m_rtpInfra.reset();
    }

    // ------------------------------------------------------------------------------------

    void NObjDirectCall::ErrorInd( QString errInfo )
    {
        AsyncComplete(false, errInfo);
        m_call.reset();
        m_rtpInfra.reset();
    }

	// ------------------------------------------------------------------------------------

    void NObjDirectCall::WaitInterfaceReady(DRI::IAsyncCmd* pAsyncCmd, int timeout )
    {        
        AsyncBegin(pAsyncCmd);

        m_tReadyPolling.Start(CPollInterval);
        m_waitReadyTimout = timeout;
    }

	// ------------------------------------------------------------------------------------

    void NObjDirectCall::OnCtrlIsReady( iCore::MsgTimer* )
    {
        if (m_ctrl->IsConnectionActive()) 
            return AsyncComplete(true);

        m_waitReadyTimout -= CPollInterval;
        if (m_waitReadyTimout <= 0) 
            return AsyncComplete(false, "Waiting IsConnectionActive timeout was expired.");

        m_tReadyPolling.Start(CPollInterval);
    }
} // namespace TestRealEcho
