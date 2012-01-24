
#ifndef __SAFEBIPROTOTESTBASE__
#define __SAFEBIPROTOTESTBASE__

#include "SafeBiProto/SafeBiProto.h"

#include "iCore/ThreadRunner.h"
#include "iCore/MsgTimer.h"
#include "Utils/AtomicTypes.h"
#include "safebiprototestparams.h"
#include "ITestTransportsCreator.h"

namespace SBPTests
{
    using namespace SBProto;
    
    // -----------------------------------------------------------
    // Test logger
    class MiniLogger
    {
        std::string m_tag;
        bool m_silentMode;
    
    public:
        MiniLogger(const std::string &ownerTag, bool silentMode) 
        : m_tag(ownerTag), m_silentMode(silentMode)
        {}
    
        MiniLogger& operator<<(const std::string &data)
        {
            if(!m_silentMode) std::cout << m_tag << " : " << data << std::endl;
    
            return *this;
        }

        MiniLogger& Add(const std::string &str, int i)
        {
            if(!m_silentMode) std::cout << m_tag << " : " << str << i << std::endl;
    
            return *this;
        }
    };
  
    // -----------------------------------------------------------

    class ClientSideBase:
        public iCore::MsgObject,
        public ISafeBiProtoEvents,
        public ISbpTransportToUser,
        public boost::noncopyable
    {
        typedef ClientSideBase T;
        
        // SbpTcpTransport m_transport;
        boost::shared_ptr<ISbpTransport> m_transport;
        
        SafeBiProto m_proto;
        iCore::MsgTimer m_timer;

        bool m_testWithMsg;

    // events
    private:
        // timer event
        void OnProtoProcess(iCore::MsgTimer *pT)
        {
            m_proto.Process();
        }

    // ISafeBiProtoEvents impl
    private:
        void CommandReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            if (m_testWithMsg) PutMsg(this, &T::OnCommandReceived, data);
            else OnCommandReceived(data);
        }
        
        void ResponseReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            if (m_testWithMsg) PutMsg(this, &T::OnResponseReceived, data);
            else OnResponseReceived(data);
        }
        
        void InfoReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            if (m_testWithMsg) PutMsg(this, &T::OnInfoReceived, data);
            else OnInfoReceived(data);
        }

        void ProtocolError(boost::shared_ptr<SbpError> err)
        {
            OnProtocolError(err);
        }

    // ISbpTransportEventsForUser impl
    private:
        void TransportConnected()
        {
            OnConnected();
        } 
        
        void TransportDisconnected(const std::string &desc)
        {
            OnDisconnected(desc);
        }

        void TransportError(const std::string &err)
        {
            // ESS_HALT(err->ToString()); <-- undefined type
            TUT_ASSERT(0 && "Transport error");
        }

    protected:
        SafeBiProto& Protocol() { return m_proto; }
        
        ISbpTransport& Transport()
        {
            ESS_ASSERT(m_transport.get() != 0);
            return *m_transport.get();
        }
        
        virtual void OnCommandReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            TUT_ASSERT(0 && "Client side: OnCommandReceived"); 
        }
        
        virtual void OnInfoReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            TUT_ASSERT(0 && "Client side: OnInfoReceived"); 
        }
        
        virtual void OnResponseReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            TUT_ASSERT(0 && "Client side: OnResponceReceived");
        }
        
        virtual void OnProtocolError(boost::shared_ptr<SbpError> err)
        {
            TUT_ASSERT(0 && "Client side: OnProtocolError");
        }

        virtual void OnConnected()
        {
            TUT_ASSERT(0 && "Client side: OnConnected");
        }

        virtual void OnDisconnected(const std::string &desc)
        {
            TUT_ASSERT(0 && "Client side: OnDisconnected");
        }

    public:
        ClientSideBase(iCore::IThreadRunner &runner, SafeBiProtoTestParams &params) :
            iCore::MsgObject(runner.getThread()), 
            m_transport(params.getTransportsCreator()
                              .CreateTransport(*this, runner.getThread())),
            m_proto(*this, *m_transport.get(), params.getSettings()), 
            m_timer(this, &T::OnProtoProcess),
            m_testWithMsg(params.getTestWithMsgs())
        {
            TUT_ASSERT(m_transport.get() != 0);
            if (params.getSettings().getTimeoutCheckIntervalMs() != 0)
            {
                m_timer.Start(params.getSettings().getTimeoutCheckIntervalMs(), true);
            }
            m_proto.Activate();
        }
    };

    // -----------------------------------------------------------

    // One-session server with sent packets monitoring
    class ServerSideBase:
        public iCore::MsgObject,
        public ISafeBiProtoEvents,
        public ISbpTransportToUser,
        public ISbpUserTransportServerEvents,
        public ISpbProtoMonitor,
        public boost::noncopyable
    {
        typedef ServerSideBase T;
        
        iCore::MsgThread &m_thread;
        boost::shared_ptr<ISbpUserTransportServer> m_server;
        boost::shared_ptr<ISbpTransport> m_transport;
        boost::scoped_ptr<SafeBiProto> m_proto; // create after connection
        iCore::MsgTimer m_timer;
        SafeBiProtoTestParams &m_params;
        
        
    // events
    private:
        // timer event
        void OnProtoProcess(iCore::MsgTimer *pT)
        {
            if (m_proto.get() == 0) return;

            m_proto->Process();
        }

        
    // ISafeBiProtoEvents impl
    private:
        void CommandReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            if (m_params.getTestWithMsgs()) PutMsg(this, &T::OnCommandReceived, data);
            else OnCommandReceived(data);
        }
        
        void ResponseReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            if (m_params.getTestWithMsgs()) PutMsg(this, &T::OnResponseReceived, data);
            else OnResponseReceived(data);
        }
        
        void InfoReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            if (m_params.getTestWithMsgs()) PutMsg(this, &T::OnInfoReceived, data);
            else OnInfoReceived(data);
        }

        void ProtocolError(boost::shared_ptr<SbpError> err)
        {
            OnProtocolError(err);
        }

    // ISbpUserTransportServerEvents impl
    private:
        void NewConnection(boost::shared_ptr<ISbpTransport> transport)
        {
            TUT_ASSERT(transport.get() != 0);
            m_transport = transport;
            m_transport->BindUser(this);

            // Init proto
            m_proto.reset(new SafeBiProto(*this, *m_transport.get(), m_params.getSettings(),
                                          this)); 
            m_proto->Activate();

            OnNewTransport();
            
        } // cmd sent

        void TransportServerError(const std::string &err)
        {
            // ESS_HALT(err->ToString()); <-- undefined type
            TUT_ASSERT(0 && "Transport server error");
        }
        
    // ISbpTransportToUser impl
    private:
        void TransportConnected()
        {
            OnConnected();
        }
        
        void TransportDisconnected(const std::string &desc)
        {
            OnDisconnected(desc);
        }

        void TransportError(const std::string &err)
        {
            TUT_ASSERT(0 && "Transport error");
        }

    // ISpbProtoMonitor impl
    private: 

        /*
        void SentPacketInfo(const SbpPackInfo &data, bool isSentPack)
        {
            OnSentPacketInfo(data, isSentPack);
        } */

        void OnSpbPacketIn(const SbpPackInfo &data)
        {
            OnSentPacketInfo(data, false);
        }

        void OnSpbPacketOut(const Utils::MemWriterDynStream &header, const Utils::MemWriterDynStream &body)
        {
			SbpPackInfo data(header, body);
            OnSentPacketInfo(data, true);
        }

        void OnRawDataRecv(const void *pData, size_t size)
        {
            // nothing 
        }
        
    protected:
        SafeBiProto& Protocol() 
        {
            ESS_ASSERT(m_proto.get() != 0); 
            return *m_proto.get(); 
        }

        ISbpTransport& Transport()
        {
            ESS_ASSERT(m_transport.get() != 0);
            return *m_transport.get();
        }
        
        virtual void OnCommandReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            TUT_ASSERT(0 && "Server side: OnCommandReceived");
        }
        
        virtual void OnInfoReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            TUT_ASSERT(0 && "Server side: OnInfoReceived");
        }
        
        virtual void OnResponseReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            TUT_ASSERT(0 && "Server side: OnResponceReceived");
        }
        
        virtual void OnProtocolError(boost::shared_ptr<SbpError> err)
        {
            TUT_ASSERT(0 && "Server side: OnProtocolError");
        }
        
        virtual void OnConnected()
        {
            TUT_ASSERT(0 && "Server side: OnConnected");
        }
        
        virtual void OnDisconnected(const std::string &desc)
        {
            TUT_ASSERT(0 && "Server side: OnDisconnected");
        }
        
        virtual void OnNewTransport()
        {
            TUT_ASSERT(0 && "Server side: OnNewTransport");
        }

        virtual void OnSentPacketInfo(const SbpPackInfo &data, bool isSentPack)
        {
            TUT_ASSERT(0 && "Server side: OnSentPacketInfo");
        }
        
    public:
        ServerSideBase(iCore::IThreadRunner &runner, SafeBiProtoTestParams &params) :
            iCore::MsgObject(runner.getThread()), 
            m_thread(runner.getThread()), 
            m_server(params.getTransportsCreator()
                           .CreateTransportServer(*this, runner.getThread())), 
            m_timer(this, &T::OnProtoProcess), 
            m_params(params)
        {
            m_server->Start();
            
            if (params.getSettings().getTimeoutCheckIntervalMs() != 0)
            {
                m_timer.Start(params.getSettings().getTimeoutCheckIntervalMs(), true);
            }
        }
    };


    
        
    
} // namespace SBPTests


#endif
