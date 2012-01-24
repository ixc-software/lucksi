
#include "stdafx.h"

#include "SafeBiProtoTests.h"
#include "SafeBiProto/SafeBiProto.h"
#include "SafeBiProto/ISbpTransport.h"

#include "iCore/MsgTimer.h"
#include "iCore/ThreadRunner.h"
#include "Utils/AtomicTypes.h"
#include "Utils/Random.h"
// #include "Utils/QtDateTime.h" // for random seed

namespace 
{
    using namespace SBProto;
    using namespace SBPTests;
    
    const int CTestTimeoutMs = 900 * 1000;

    const dword CCheckTimeoutIntervalMs = 200;
    const dword CRecvEndTimeoutMs = 2 * 1000;
    const dword CResponceTimeoutMs = 1 * 1000;

    const int CPackCount = 1;
    const dword CRandomRange = 40;
    const dword CMinSentDataSize = 2; 

    const dword CEach2dPackData = 125463;
    /*const*/ std::string CEach3rdPackData = "Each 3rd information packet data";
    const int32 COtherPackData = -5678;

    // ---------------------------------------------------

    class TestParams {};

    // ---------------------------------------------------

    // Base interface for test transports
    class ITestSbpTransport : public ISbpTransport
    {
    public:
        virtual void SetSendingComplete() = 0;
    };
    
    // ---------------------------------------------------

    // Base class for parsing test transports
    class BaseTestTransport : public ITestSbpTransport
    {
        ISbpTransportToProto *m_pProto;
        Utils::MemWriterDynStream m_sentDataStream;
        Utils::BinaryWriter<Utils::MemWriterDynStream> m_sentDataWriter;

    // ITestSbpTransport impl
    private:

        size_t MaxSendSize() const { return 0; }

        void BindProto(ISbpTransportToProto *pProto)
        {
            TUT_ASSERT(pProto != 0);
            m_pProto = pProto;
        }

        void UnbindProto() { m_pProto  = 0; }
        
        void Connect() { TUT_ASSERT(0 && "Don't call connect"); }
        void Disconnect() { TUT_ASSERT(0 && "Don't call disconnect"); }
        void BindUser(ISbpTransportToUser *pUser) { TUT_ASSERT(0 && "Don't bind user"); }
        std::string Info() const
        {
        	return "BaseTestTransport";
        }
        void Process(){}

        void Send(const void *pData, size_t size)
        {
            TUT_ASSERT(m_pProto != 0);
            // collecting data until SetSendingFinished() will be called
            m_sentDataWriter.WriteData(pData, size);
        }

        void SetSendingComplete() 
        {
            TUT_ASSERT(m_pProto != 0);
            if (m_sentDataStream.GetBuffer() == 0) return;
            SendToProto(m_sentDataStream.DetachBuffer());
        }

    protected:
        ISbpTransportToProto& Protocol()
        {
            TUT_ASSERT(m_pProto != 0);
            return *m_pProto;
        }
        
        virtual void SendToProto(boost::shared_ptr<Utils::ManagedMemBlock> data) = 0;

    public:
        BaseTestTransport() 
        : m_pProto(0), m_sentDataStream(0), m_sentDataWriter(m_sentDataStream)
        {
        }

        virtual ~BaseTestTransport() 
        {}
    };

    // ---------------------------------------------------
    // Base class for SafeBiProto parsing tests
    class SbpParsingTestBase:
        public iCore::MsgObject, // for timer
        public ISafeBiProtoEvents
    {
        iCore::MsgTimer m_timer;
        Utils::AtomicBool &m_done;

        boost::shared_ptr<ITestSbpTransport> m_transport;
        SafeBiProto m_proto;

    protected:
        void Finish() { AsyncSetReadyForDeleteFlag(m_done); }
        void SetSendingComplete() { m_transport->SetSendingComplete(); }

        SafeBiProto &Protocol() { return m_proto; }

        static SbpSettings CreateDefaultSettings()
        {
            SbpSettings settings;
            settings.setReceiveEndTimeoutMs(CRecvEndTimeoutMs);
            settings.setResponceTimeoutMs(CResponceTimeoutMs);
            settings.setTimeoutCheckIntervalMs(CCheckTimeoutIntervalMs);
            return settings;
        }
        
    // ISafeBiProtoEvents impl
    protected:
        virtual void InfoReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            TUT_ASSERT(0 && "Info received");
        }

        virtual void CommandReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            TUT_ASSERT(0 && "Command received");
        }

        virtual void ResponseReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            TUT_ASSERT(0 && "Responce received");
        }

        virtual void ProtocolError(boost::shared_ptr<SbpError> err)
        {
            ESS_HALT(err->ToString());
        }

    private:
        // timer event
        void OnProtoProcess(iCore::MsgTimer *pT)
        {
            m_proto.Process();
        }

    public:
        SbpParsingTestBase(iCore::IThreadRunner &runner, 
            const SbpSettings &sbpSettings, boost::shared_ptr<ITestSbpTransport> transport) 
        : iCore::MsgObject(runner.getThread()), 
            m_timer(this, &SbpParsingTestBase::OnProtoProcess), 
            m_done(runner.getCompletedFlag()),
            m_transport(transport), m_proto(*this, *m_transport, sbpSettings)
        {
            int checkTimeout = sbpSettings.getTimeoutCheckIntervalMs();
            if(checkTimeout) m_timer.Start(checkTimeout, true);
            m_proto.Activate();
        }

        virtual ~SbpParsingTestBase() {}
    };
    
    // ---------------------------------------------------
    // Test fragmented packets parsing
    // ---------------------------------------------------
    
    // Collect data into buffer. Split data to the parts with random size 
    // and send them to prototcol after setSendingFinished()
    class ParsingTestTransport : public BaseTestTransport
    {
        void SendToProto(boost::shared_ptr<Utils::ManagedMemBlock> data) // override
        {
            // split data to parts and send back to proto
            Utils::MemReaderStream readerStream(data->getData(), 
                                                data->Size());
            Utils::BinaryReader<Utils::MemReaderStream> reader(readerStream);

            // Utils::QtDateTime t;
            // t.Capture();
            dword randSeed = Platform::GetSystemTickCount();
            Utils::Random rand(randSeed);
            while (readerStream.HasToRead())
            {
                size_t randSize = rand.Next(CRandomRange) + CMinSentDataSize;
                size_t sendSize = (readerStream.HasToRead() <= randSize)?
                                   readerStream.HasToRead() : randSize;
                
                std::string tmp;
                reader.ReadString(tmp, sendSize);

                Protocol().DataReceived(tmp.data(), tmp.size());
            }
        }
    };

    // ---------------------------------------------------

    class SbpParsingTest : public SbpParsingTestBase
    {
        int m_recvPackCounter;
        
        void InfoReceived(boost::shared_ptr<SbpRecvPack> data) // override
        {
            TUT_ASSERT(m_recvPackCounter < CPackCount);

            // check packets're right parsed
            if (m_recvPackCounter % 2 == 0)
            {
                TUT_ASSERT(data->Curr().AsDword() == CEach2dPackData);
            } 
            else if (m_recvPackCounter % 3 == 0)
            {
                TUT_ASSERT(data->Curr().AsString() == CEach3rdPackData);
            }
            else
            {
                TUT_ASSERT(data->Curr().AsInt32() == COtherPackData);
            }

            ++m_recvPackCounter;

            if (m_recvPackCounter != CPackCount) return;

            Finish();
        }

        static SbpSettings CreateSettings()
        {
            SbpSettings settings(CreateDefaultSettings());
            settings.setReceiveEndTimeoutMs(2000);
            return settings;
        }

    public:
        SbpParsingTest(iCore::IThreadRunner &runner, TestParams &dummiParams)
        : SbpParsingTestBase(runner, CreateSettings(), boost::shared_ptr<ParsingTestTransport>(
                                                            new ParsingTestTransport)),
            m_recvPackCounter(0)
        {
            // send packets
            for (int i = 0; i < CPackCount; ++i)
            {
                if ((i % 2) == 0)
                {
                    SbpSendPackInfo pack(Protocol());
                    pack.WriteDword(CEach2dPackData);
                    continue;
                }// send

                if ((i % 3) == 0)
                {
                    SbpSendPackInfo pack(Protocol());
                    pack.WriteString(CEach3rdPackData);
                    continue;
                }// send

                SbpSendPackInfo pack(Protocol());
                pack.WriteInt32(COtherPackData);
            } // send
            
            SetSendingComplete();
        }
    };
    
    // ---------------------------------------------------
    // Test parsing timeout
    // ---------------------------------------------------

    // Using for sending not all data
    class ParsingTimeoutTestTransport : public BaseTestTransport
    {
        void SendToProto(boost::shared_ptr<Utils::ManagedMemBlock> data) // override
        {
            const size_t dontSendPartSize = 2; 
            TUT_ASSERT(data->Size() > dontSendPartSize);

            Protocol().DataReceived(data->getData(), (data->Size() - dontSendPartSize));
        }
    };

    // ---------------------------------------------------

    class SbpParsingTimeoutTest : public SbpParsingTestBase
    {
        void ProtocolError(boost::shared_ptr<SbpError> err) // override
        {
            TUT_ASSERT(typeid(*err.get()) == typeid(SbpWaitingEndOfReceiveTimeout));
            Finish();
        }
        static SbpSettings CreateSettings()
        {
            SbpSettings settings(CreateDefaultSettings());
            // set short timeout 
            settings.setReceiveEndTimeoutMs(200);
            return settings;
        }

    public:
        SbpParsingTimeoutTest(iCore::IThreadRunner &runner, TestParams &dummiParams) : 
            SbpParsingTestBase(runner, CreateSettings(), 
                boost::shared_ptr<ParsingTimeoutTestTransport>(new ParsingTimeoutTestTransport))
        {
            {
                SbpSendPackCmd pack(Protocol());
                std::string cmd = "get_var";
                pack.WriteString(cmd);
            } // send

            SetSendingComplete();
        }
    };
    
    // ---------------------------------------------------
    // Tests with simple transport:
    // ---------------------------------------------------
    
    // Using just for sending data back to proto
    class SimpleTestTransport : public BaseTestTransport
    {
        void SendToProto(boost::shared_ptr<Utils::ManagedMemBlock> data) // override
        {
            Protocol().DataReceived(data->getData(), data->Size());
        }
    };
    

    // ---------------------------------------------------
    // Test proto version error
    // ---------------------------------------------------

    class SbpInvalidProtoVerErrTest: public SbpParsingTestBase
    {
        void ProtocolError(boost::shared_ptr<SbpError> err) // override
        {
            TUT_ASSERT(typeid(*err.get()) == typeid(SbpInvalidVersion));
            Finish();
        }

    public:
        SbpInvalidProtoVerErrTest(iCore::IThreadRunner &runner, TestParams &dummiParams) : 
            SbpParsingTestBase(runner, CreateDefaultSettings(),
                boost::shared_ptr<SimpleTestTransport>(new SimpleTestTransport))
        {
            {
                SbpSendPackCmd pack(Protocol());
                pack.WriteByte(5);
            } //send

            // change proto version
            byte newMaxProtoVer = Protocol().Settings().getMaxProtoVersion() + 1;
            Protocol().SettingsForTest().setProtoVersions(newMaxProtoVer, newMaxProtoVer, newMaxProtoVer);

            SetSendingComplete();
        }
    };

    // ---------------------------------------------------
    // Test too-big-packet error
    // ---------------------------------------------------

    class SbpTooBigPacketErrTest: public SbpParsingTestBase
    {
        void ProtocolError(boost::shared_ptr<SbpError> err) // override
        {
            TUT_ASSERT(typeid(*err.get()) == typeid(SbpSendPackSizeIsTooBig));
            Finish();
        }

    public:
        SbpTooBigPacketErrTest(iCore::IThreadRunner &runner, TestParams &dummiParams) : 
            SbpParsingTestBase(runner, CreateDefaultSettings(), 
                boost::shared_ptr<SimpleTestTransport>(new SimpleTestTransport))
        {
            // set max size
            size_t newMaxPackSize = 10;
            Protocol().SettingsForTest().setMaxSendSize(newMaxPackSize); //  setPacketMaxSizeBytes(newMaxPackSize);

            // send pack with size > maxSize
            {
                SbpSendPackInfo pack(Protocol());
                std::string data(newMaxPackSize, 'a');
                pack.WriteString(data);

            } // send: packet size = newMaxPackSize + headerSize

            SetSendingComplete();
        }
    };
    
    // ---------------------------------------------------
    // Test with empty packet
    // ---------------------------------------------------

    class SbpEmptyPacketTest: public SbpParsingTestBase
    {
        void InfoReceived(boost::shared_ptr<SbpRecvPack> data) // override
        {
            TUT_ASSERT(!data->Next());
            Finish();
        }
        
    public:
        SbpEmptyPacketTest(iCore::IThreadRunner &runner, TestParams &dummiParams) : 
            SbpParsingTestBase(runner, CreateDefaultSettings(), 
                boost::shared_ptr<SimpleTestTransport>(new SimpleTestTransport))
        {
            {
                SbpSendPackInfo pack(Protocol());
            } // send
            
            SetSendingComplete();
        }
    };
   
} // namespace

// ---------------------------------------------------

namespace SBPTests
{
    void SbpRecvParsingTest(bool silentMode)
    {
        TestParams params;
        iCore::ThreadRunner test(Platform::Thread::LowPriority, CTestTimeoutMs);
      
        if (!test.Run<SbpParsingTest>("SbpParsingTest", params)) 
            TUT_ASSERT(0 && "SafeBiProto fragmented packets parsing test timeout");

        if (!test.Run<SbpParsingTimeoutTest>("SbpParsingTimeoutTest", params)) 
            TUT_ASSERT(0 && "SafeBiProto parsing timeout test timeout");

        if (!test.Run<SbpInvalidProtoVerErrTest>("SbpInvalidProtoVerErrTest", params)) 
            TUT_ASSERT(0 && "SafeBiProto Invalid proto version errror test timeout");

        if (!test.Run<SbpTooBigPacketErrTest>("SbpTooBigPacketErrTest", params)) 
            TUT_ASSERT(0 && "SafeBiProto too big packet error test timeout");
/*
        if (!test.Run<SbpEmptyPacketTest>(params)) 
            TUT_ASSERT(0 && "SafeBiProto empty packet sending test timeout");
*/
        if (silentMode) return;
        std::cout << "SafeBiProto received data parsing tests okay." << std::endl;
    }
    
} // namespace SBPTests






    // ---------------------------------------------------
    /*
        Test stages:
            -- StParsingTimeout -- test waiting end of received data timeout,
            -- StParsing -- test parsing, when received packets splited to fragments,
            -- StInvalidProtoVer -- test invalid protocol version error,
            -- StTooBigPacket -- test with too big packet,
            -- StEmptyPacket -- test with empty packet,
            -- StFinished -- finish.

        * in this test protocol Activate()/Deactivate() is tested also: deactivate on error,
        activate on new stage
    */
    
    /*
    class SbpParsingTest:
        public iCore::MsgObject, // for timer
        public ISafeBiProtoEvents
    {
        SbpSettings m_settings;
        iCore::MsgTimer m_timer;
        Utils::AtomicBool &m_done;

        // tranport changed depend on test stage
        boost::shared_ptr<ITestSbpTransport> m_transport;
        boost::shared_ptr<SafeBiProto> m_proto;

        int m_recvPackCounter;

        enum TestStages
        {
            StParsingTimeout = 0,
            StParsing = 1,
            StInvalidProtoVer = 2,
            StTooBigPacket = 3,
            StEmptyPacket = 4,
            StFinished = 5,
        };

        TestStages m_testStage;
        
    // ISafeBiProtoEvents impl
    private:
        void InfoReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            if (m_testStage == StEmptyPacket)
            {
                TUT_ASSERT(data->EoF());

                // Finish
                ChangeStage(StFinished);
                AsyncSetReadyForDeleteFlag(m_done);
                return;
            }
            
            TUT_ASSERT(m_testStage == StParsing);
            TUT_ASSERT(m_recvPackCounter < CPackCount);

            // check packets're right parsed
            if (m_recvPackCounter % 2 == 0)
            {
                dword checkData = data->ReadDword();
                TUT_ASSERT(checkData == CEach2dPackData);
            } 
            else if (m_recvPackCounter % 3 == 0)
            {
                std::string checkData;
                data->ReadString(checkData);
                TUT_ASSERT(checkData == CEach3rdPackData);
            }
            else
            {
                int32 checkData = data->ReadInt32();
                TUT_ASSERT(checkData == COtherPackData);
            }

            ++m_recvPackCounter;

            if (m_recvPackCounter != CPackCount) return;

            
            // Stage 2
            ChangeStage(StInvalidProtoVer);
            TestInvalidProtoVerError();
        }
        
        void CommandReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            TUT_ASSERT(0 && "Command received");
        }

        void ResponceReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            TUT_ASSERT(0 && "Responce received");
        }

        void ProtocolError(boost::shared_ptr<SbpError> err)
        {
            switch (m_testStage)
            {
            case StParsingTimeout:
                TUT_ASSERT(m_recvPackCounter == 0);
                TUT_ASSERT(typeid(*err.get()) == typeid(SbpWaitingEndOfReceiveTimeout));

                // Stage 1 
                ChangeStage(StParsing);
                SendPacketsToTransport();
                break;

            case StInvalidProtoVer:
                TUT_ASSERT(typeid(*err.get()) == typeid(SbpInvalidVersion));

                // Stage 3
                ChangeStage(StTooBigPacket);
                TestTooBigPacketError();
                break;

            case StTooBigPacket:
                TUT_ASSERT(typeid(*err.get()) == typeid(SbpSendPackSizeIsTooBig));

                // Stage 4
                ChangeStage(StEmptyPacket);
                TestWithEmptyPacket();
                break;

            default:
                ESS_HALT(err->ToString());
            }
        }
        
    private:
        template<class TTransport>
        void RecreateProtoAndTransport()
        {
            m_proto.reset();
            m_transport.reset(new TTransport);
            m_proto.reset(new SafeBiProto(*this, *m_transport, m_settings));
            m_proto->Activate();
        }
        
        void ChangeStage(TestStages stage)
        {
            m_testStage = stage;
            // std::cout << "Stage -- " << m_testStage << "\n";
        }
        
        void SendIncompletePacket()
        {
            // set short timeout 
            m_settings.setReceiveEndTimeoutMs(200);

            RecreateProtoAndTransport<ParsingTimeoutTestTransport>();
            {
                SbpSendPackCmd pack(*m_proto);
                std::string cmd = "get_var";
                pack.WriteString(cmd);
            } // send

            m_transport->SetSendingComplete();
        }
        
        void SendPacketsToTransport()
        {
            // reset timeout
            m_settings.setReceiveEndTimeoutMs(2000);
            
            RecreateProtoAndTransport<ParsingTestTransport>();
            
            // send packets
            for (int i = 0; i < CPackCount; ++i)
            {
                if ((i % 2) == 0)
                {
                    SbpSendPackInfo pack(*m_proto);
                    pack.WriteDword(CEach2dPackData);
                    continue;
                }// send

                if ((i % 3) == 0)
                {
                    SbpSendPackInfo pack(*m_proto);
                    pack.WriteString(CEach3rdPackData);
                    continue;
                }// send

                SbpSendPackInfo pack(*m_proto);
                pack.WriteInt32(COtherPackData);
            } // send
            
            m_transport->SetSendingComplete();
        }

        void TestInvalidProtoVerError()
        {
            RecreateProtoAndTransport<SimpleTestTransport>();

            {
                SbpSendPackCmd pack(*m_proto);
                pack.WriteByte(5);
            } //send

            // change proto version
            byte newMaxProtoVer = m_settings.getMaxProtoVersion() + 1;
            m_settings.setProtoVersions(newMaxProtoVer, newMaxProtoVer, newMaxProtoVer);

            m_transport->SetSendingComplete();
        }

        void TestTooBigPacketError()
        {
            RecreateProtoAndTransport<SimpleTestTransport>();

            // set max size
            size_t newMaxPackSize = 10;
            m_settings.setPacketMaxSizeBytes(newMaxPackSize);

            // send pack with size > maxSize
            {
                SbpSendPackInfo pack(*m_proto);
                std::string data(newMaxPackSize, 'a');
                pack.WriteString(data);

            } // send: packet size = newMaxPackSize + headerSize
            m_transport->SetSendingComplete();
            
        } // send

        void TestWithEmptyPacket()
        {
            RecreateProtoAndTransport<SimpleTestTransport>();
            {
                SbpSendPackInfo pack(*m_proto);
            } // send
            
            m_transport->SetSendingComplete();
        }

        // timer event
        void OnProtoProcess(iCore::MsgTimer *pT)
        {
            if (m_proto.get() == 0) return;
            m_proto->Process();
        }

    public:
        SbpParsingTest(iCore::IThreadRunner &runner, TestParams &dummiParams) 
        : iCore::MsgObject(runner.getThread()), 
            m_timer(this, &SbpParsingTest::OnProtoProcess), 
            m_done(runner.getCompletedFlag()), 
            m_recvPackCounter(0), m_testStage(StParsingTimeout)
        {
            m_timer.Start(m_settings.getTimeoutCheckIntervalMs(), true);
            
            // Stage 0
            SendIncompletePacket();
        }

        ~SbpParsingTest()
        {
            TUT_ASSERT(m_testStage == StFinished);
        }
    };
    */
