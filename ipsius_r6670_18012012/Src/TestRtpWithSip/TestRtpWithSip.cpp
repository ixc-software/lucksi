#include "stdafx.h"
#include "CallTestRtpWithSip.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "iCore/ThreadRunner.h"
#include "Utils/ExeName.h"
#include "iLog/LogManager.h"
#include "Sip/ProfileSip.h"
#include "Sip/InfraSip.h"
#include "Sip/GateSip.h"
#include "Sip/ISipToRtp.h"
#include "iRtp/RtpParams.h"
#include "iRtp/RtpPcInfra.h"
#include "TestProfile.h"
#include "PlayFile.h"
#include "GenerateSinus.h"
#include "WriteToFile.h"
#include "RtpClient.h"



namespace TestRtpWithSip
{
    //todo use standard log
    using iCore::MsgTimer;

    class Test : public boost::noncopyable,
        public iCore::MsgObject,
        public Sip::IGateSipEvents,
        public Utils::IVirtualDestroyOwner
    {
        typedef Test T;
        enum {CScanKeyPeriod = 200};
        static const int CScanCode = 27; // esc

        void OnScanKey(MsgTimer*)
        {
            //std::cout << Platform::KeyWasPressed();            
            if (CScanCode == Platform::KeyWasPressed())
                m_done.Set(true);
        }

    public:

        Test(iCore::IThreadRunner &runner, const TestProfile& profile)
            : iCore::MsgObject(runner.getThread()),
            m_profile(profile),
            m_done(runner.getCompletedFlag()),
            m_log(profile.getLogSettings()),
            m_infraSip(m_log),
            m_sip(runner.getThread(), &m_infraSip, profile.getSipProfile(), this),
            m_codec(m_profile.getCodec()),
			m_rtpInfra(runner.getThread(), m_profile.getRtpInfraParams(), m_log),
            m_scanKeyTimer(this, &T::OnScanKey)
        {
            m_scanKeyTimer.Start(CScanKeyPeriod, true);
            m_rtpProfile.CoreTraceInd = true; // look todo.txt
            std::cout << std::endl 
                << profile.ReflectHelp() << std::endl
                << std::endl 
                << "Test started." << std::endl
                << "Press Esc to close." << std::endl;            
        }

        ~Test(){}

    // impl IGateSipEvents
    private:    

        void ShutdownConfirm(const Sip::IGateSip *id)
        {
            ESS_ASSERT(&m_sip == id);
        }

        void IncomingCall(const Sip::IGateSip *id, 
            Utils::SafeRef<Sip::ISipCall> incomingCall, 
            const Sip::SipMessageHandler& msg)
        {
            //std::cout << "Incoming call nickname " << msg.To().user() << std::endl;
            ESS_ASSERT(&m_sip == id);

            if(m_call.get() || m_rtp.get())
            {
                incomingCall->Release(0, 480);
                return; 
            }

            QString userName( msg.To().user().c_str() );                        

            std::string dir(Utils::ExeName::GetExeDir());
            
            boost::shared_ptr<IRtpTestFactory> testFactory;
            int timeout = 0; 
            if(m_profile.IsScnWritingToDefaultFile(userName))
            {
                testFactory.reset(new WriteToFileFactory(
                    m_profile.getDefaultFileName().prepend(dir.c_str())));
            }
            else if(m_profile.IsScnPlayDefaultFile(userName))
            {
                testFactory.reset(new PlayFileFactory(m_log,
                    m_profile.getDefaultFileName().prepend(dir.c_str())));
                timeout = m_profile.getDurationPlayDefaultFile();
            }
            else if(m_profile.IsScnPlayAssignedFile(userName))
                testFactory.reset(new PlayFileFactory(m_log,
                    m_profile.getAssignedFileName().prepend(dir.c_str())));
            else if (m_profile.IsScnGenerator(userName))
                testFactory.reset(CreateGenerateSinusFactory(userName));

            if(!testFactory.get())
            {   
                incomingCall->Release(0, 480);
                return; 
            }             
            
            try
            {
                m_rtp.reset(new RtpClient(getMsgThread(),
                    *this,
                    m_codec,
                    m_rtpInfra.Infra(),
                    m_rtpProfile,
                    m_log,
                    testFactory));
            } 
            catch(IRtpTest::RtpTestError &e)
            {
                std::cout << "We can't make operation." << std::endl;
                std::cout << e.getTextMessage() << std::endl;
                incomingCall->Release(0, 480);
                return;
            }

            m_call.reset(new CallTestRtpWithSip(getMsgThread(),
                *this,
                incomingCall,
                m_rtp.get(),
                timeout));
        }
    // Utils::IVirtualDestroyOwner
    private:        
        void Add(Utils::IVirtualDestroy*)
        {
            ESS_ASSERT(0);
        }

        void Delete(Utils::IVirtualDestroy *item)
        {
            if (m_rtp.get() == item)
                m_rtp.reset();
            else if (m_call.get() == item)
                m_call.reset();
            else
                ESS_HALT("Unknown source of delete request");
        }
    private:        
        IRtpTestFactory *CreateGenerateSinusFactory(QString userName)
        {
            userName.remove(m_profile.getGeneratorPrefix()); // remove key
            bool ok = false;
            int amplitude = userName.section(',', 0, 0).toInt(&ok);
            if(!ok || amplitude > 100 || amplitude < 0)
                return 0;
            int frec = userName.section(',', 1, 1).toInt(&ok);
            if (!ok || frec < 10 || frec > 4000)        
                return 0;

            return new GenerateSinusFactory(m_log, amplitude, frec);
        }

    private:        
        const TestProfile m_profile;
        Utils::AtomicBool &m_done; 
        iLogW::LogManager m_log;
        Sip::InfraSip m_infraSip;
        Sip::GateSip m_sip;
        Sip::Codec m_codec;
        iRtp::RtpPcInfra m_rtpInfra;
        boost::scoped_ptr<CallTestRtpWithSip> m_call;
        boost::scoped_ptr<RtpClient> m_rtp;
        MsgTimer m_scanKeyTimer;
        iRtp::RtpParams m_rtpProfile;
    };
} // namespace TestRtpWithSip

namespace TestRtpWithSip 
{
    void RunTest(const TestProfile& prof)
    {
        iCore::ThreadRunner test;

        if(!test.Run<Test>(prof))
		{
           TUT_ASSERT(0 && "Timeout");
		}
    }
}

