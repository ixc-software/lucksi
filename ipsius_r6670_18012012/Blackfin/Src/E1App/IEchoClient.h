#ifndef __IECHOCLIENT__
#define __IECHOCLIENT__

#include "Utils/IBasicInterface.h"

#include "SafeBiProto/SbpSendPack.h"
#include "iCmp/ChMngProtoShared.h"
#include "DrvAoz/AozShared.h"
#include "TdmMng/EventsQueue.h"

#include "EchoApp/SfxProto.h"
#include "EchoApp/EchoAppStats.h"


namespace E1App
{
    
    class IEchoClient : public Utils::IBasicInterface
    {
    public:

        virtual void CmdInit(const EchoApp::CmdInitData &data) = 0;
        virtual void CmdEcho(int chNum, int taps) = 0;
        virtual void CmdHalt() = 0;
        virtual void CmdShutdown() = 0;
        virtual void CmdSetFreqRecvMode(int chNum, const std::string &recvName, 
            const std::string &params, iCmp::FreqRecvMode mode) = 0;

        virtual void Process() = 0;
        virtual EchoApp::EchoAppStats GetStats() = 0;
        virtual void SendAllEvents(SBProto::ISafeBiProtoForSendPack &send, 
                                   TdmMng::IIEventsQueueItemDispatch &hook) = 0;
        virtual void RouteAozEvent(int chNum, TdmMng::AozLineEvent e) = 0;
    };

    // ---------------------------------------

    class NullEchoClient : public IEchoClient
    {
        const std::string m_info;

    // IEchoClient impl
    public:

        void CmdInit(const EchoApp::CmdInitData &data) {}
        void CmdEcho(int chNum, int taps) {}
        void CmdHalt() {}
        void CmdShutdown() {}

        void Process() {}

        EchoApp::EchoAppStats GetStats()
        {
            EchoApp::EchoAppStats stats;
            stats.IsEmpty = false;
            stats.BuildInfo = m_info;
            return stats;
        }

        void SendAllEvents(SBProto::ISafeBiProtoForSendPack &send,
                           TdmMng::IIEventsQueueItemDispatch &hook) {}

        void CmdSetFreqRecvMode(int chNum, const std::string &recvName, 
            const std::string &params, iCmp::FreqRecvMode mode) {}

        void RouteAozEvent(int chNum, TdmMng::AozLineEvent e) {}

    public:

        NullEchoClient(const std::string &info = "NullEchoClient") 
          : m_info(info)
        {
        }

    };
    
    
}   // namespace E1App

#endif
