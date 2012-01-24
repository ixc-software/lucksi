#ifndef __ECHOHWCLIENT__
#define __ECHOHWCLIENT__

#include "DevIpTdm/ZL38065.h"
#include "BfDev/BfUartSimpleCore.h"
#include "BfDev/SysProperties.h"
#include "iCmp/ChMngProto.h"
#include "TdmMng/TdmEvents.h"

#include "IEchoClient.h"

namespace E1App
{
    using Platform::byte;
    using iCmp::BfTdmEvent;
    
    // echo client based on ZL38065 chip, used thru IEchoClient
    class EchoHwClient : 
        public IEchoClient,
        boost::noncopyable
    {

        class Transport : 
            public DevIpTdm::IZl38065Transport,
            boost::noncopyable
        {
            const int m_sysFreq;
            boost::scoped_ptr<BfDev::BfUartSimpleCore> m_uart;
            int m_errorCount;

            enum
            {
                CPort = 1,
                CSpeed = 115200,
            };

        // IZl38065Transport impl
        private:

            bool WriteRead(byte valWrite, byte &valRead)
            {
                // if (m_errorCount > 0) return false;  // ?!

                m_uart->Send(valWrite);

                Platform::dword t = Platform::GetSystemTickCount();

                // 1 ms can switch uncontrolled quickly 
                while(Platform::GetSystemTickCount() - t < 2)
                {
                    if (m_uart->ReadyToRead()) 
                    {
                        valRead = m_uart->Read();
                        return true;
                    }
                }

                ++m_errorCount;
                return false;
            }

            void ClearRxBuff()
            {
                while( m_uart->ReadyToRead() ) 
                {
                    m_uart->Read();
                }
            }

        public:

            Transport() : 
              m_sysFreq( BfDev::SysProperties::Instance().getFrequencySys() ),
              m_uart( new BfDev::BfUartSimpleCore(m_sysFreq, CPort, CSpeed) ),
              m_errorCount(0)
            {
            }

        };

        boost::shared_ptr<Transport> m_transport;
        DevIpTdm::ZL38065 m_chip;
        TdmMng::EventsQueue m_events;
        std::vector<bool> m_chEchoEnabled;
        int m_errorsCount;

        EchoHwClient(boost::shared_ptr<Transport> transport) : 
            m_transport(transport),
            m_chip(*m_transport),
            m_events(BfTdmEvent::SrcQueueEcho()),
            m_chEchoEnabled(32, false),
            m_errorsCount(0)
        {            
        }

        void PushEvent(const std::string &e, const std::string &params = "")
        {
            m_events.Push( TdmMng::TdmAsyncEvent(BfTdmEvent::SrcEcho(), e, params) );
        }

        static std::string BitsetToHex(const std::vector<bool> &v)
        {
            ESS_ASSERT(v.size() <= 32);

            Platform::dword res = 0;

            for(int i = 0; i < v.size(); ++i)
            {
                if (v.at(i)) res |= (1 << i);
            }

            return Utils::IntToHexString(res);
        }

        static std::string BoolToStr(bool val)
        {
            return val ? "1" : "0";
        }

    // IEchoClient impl
    private:

        void CmdInit(const EchoApp::CmdInitData &data)
        {
            // nothing
        }

        void CmdEcho(int chNum, int taps)
        {
            const bool CEnableDebug = true;
            const bool CTraceOnlyErrors = true;

            // check params
            bool chOk = (chNum >= 0) && (chNum < m_chEchoEnabled.size());
            bool tapsOk = (taps == 0) || (taps == 64);
            if (!chOk || !tapsOk) 
            {
                ++m_errorsCount;
                PushEvent( BfTdmEvent::CEchoHwBadMode() );
                return;
            }

            // store current mask for debug
            std::vector<bool> prevMask;
            if (CEnableDebug) prevMask = m_chEchoEnabled;

            // update buffer
            m_chEchoEnabled.at(chNum) = (taps != 0) ? true : false;

            // do cmd
            int chNumAlignedIndex = chNum & (~1);  // clear last bit
            bool chLo = m_chEchoEnabled.at(chNumAlignedIndex);
            bool chHi = m_chEchoEnabled.at(chNumAlignedIndex + 1);

            int chNumShifted = chNum >> 1;
            std::string extErrorInfo;
            bool res = m_chip.SetMode(chNumShifted, false, chLo, chHi, extErrorInfo);

            // error notify
            if (!res) 
            {
                ++m_errorsCount;
                PushEvent( BfTdmEvent::CEchoHwError() );
            }

            // debug
            bool resFlag = CTraceOnlyErrors ? (!res) : true;
            if ( CEnableDebug && resFlag )
            {
                std::ostringstream oss;
                oss << "CmdEcho (ch " << chNum << ", taps " << taps << ")";

                if (res)
                {
                    oss << "; mask " << BitsetToHex(prevMask) << " -> " << BitsetToHex(m_chEchoEnabled);
                    oss << "; SetMode (chPair " << chNumShifted << ", " <<
                        BoolToStr(chLo) << " " << BoolToStr(chHi) << ")";
                }
                else
                {
                    oss << "; FAIL";
                    if (!extErrorInfo.empty()) oss << " " << extErrorInfo;
                }

                PushEvent( BfTdmEvent::CTraceDebug(), oss.str() );
            }
        }

        void CmdHalt()
        {
            // nothing
        }

        void CmdShutdown()
        {
            // nothing
        }

        void CmdSetFreqRecvMode(int chNum, const std::string &recvName, 
            const std::string &params, iCmp::FreqRecvMode mode)
        {
            // nothing
        }


        void Process()
        {
            // nothing
        }

        EchoApp::EchoAppStats GetStats()
        {
            EchoApp::EchoAppStats s;

            s.IsEmpty = false;
            s.BuildInfo = "EchoHwClient/DevIpTdm::ZL38065";
            s.BlockCollisions = m_errorsCount;
            s.ChannelsWithEcho = std::count(m_chEchoEnabled.begin(), m_chEchoEnabled.end(), true);

            return s;
        }

        void SendAllEvents(SBProto::ISafeBiProtoForSendPack &send, 
                           TdmMng::IIEventsQueueItemDispatch &hook)
        {
            m_events.SendAll(send);
        }

        void RouteAozEvent(int chNum, TdmMng::AozLineEvent e)
        {
            // nothing
        }


    public:

        // return 0 if fail 
        static IEchoClient* Create()
        {
            boost::shared_ptr<Transport> transport( new Transport() );

            if ( !DevIpTdm::ZL38065::TryDetect(*transport) ) return 0;

            return new EchoHwClient(transport);
        }


    };

}  // namespace E1App

#endif
