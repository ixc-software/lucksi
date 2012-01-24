
#include "stdafx.h"

#include "SafeBiProtoTests.h"
#include "SafeBiProtoTestBase.h"
#include "TestTcpTransportCreator.h"

#include "SafeBiProto/SbpTypes.h"
#include "SafeBiProto/SbpException.h"
#include "SafeBiProto/SbpCRC.h"

#include "Utils/AtomicTypes.h"

namespace
{
    using namespace SBProto;
    using namespace SBPTests;
    
    const size_t CPackFieldsCount = 12;
    const size_t CInfoVectorByteSize = 5; // should be odd
    std::vector<byte> CInfoVectorByte = std::vector<byte>(CInfoVectorByteSize, 9);

    
    class TestParams: 
        public SafeBiProtoTestParams
    {
       bool m_wasBadFieldIndexException;
       size_t m_badFieldTypeExceptionCount;
       bool m_wasBufferTooSmallException;
        
    public:
        TestParams(ITestTransportsCreator &creator, bool silentMode) : 
            SafeBiProtoTestParams(creator, true, silentMode, true), 
            m_wasBadFieldIndexException(false), m_badFieldTypeExceptionCount(0),
            m_wasBufferTooSmallException(false) 
        {
        }

        bool WasAllTestedExceptions() const
        {
            return (m_wasBadFieldIndexException 
                    && (m_badFieldTypeExceptionCount == CPackFieldsCount)
                    && m_wasBufferTooSmallException);
        }

        void WasBadFieldIndexException() 
        {
            TUT_ASSERT(!m_wasBadFieldIndexException); 
            m_wasBadFieldIndexException = true; 
        }

        void WasBadFieldTypeException() 
        { 
            ++ m_badFieldTypeExceptionCount; 
        }

        void WasBufferTooSmallException() 
        { 
            TUT_ASSERT(!m_wasBufferTooSmallException);
            m_wasBufferTooSmallException = true; 
        }
    };
    
    class Client:
        public ClientSideBase
    {
        Utils::AtomicBool &m_done;
        TestParams &m_params;
        
        void OnInfoReceived(boost::shared_ptr<SbpRecvPack> data) // override
        {
            TUT_ASSERT(data.get() != 0);
            TUT_ASSERT(data->Count() == CPackFieldsCount);

            // count BadFieldType exception
            for(size_t i = 0; i < data->Count(); ++i)
            {
                try
                {
                    if (i == 0)  // first field is byte
                    {
                        bool b = (*data)[i].AsBool(); // failed
                    }
                    else 
                    {
                        byte bt = (*data)[i].AsByte(); // failed
                    }
                }
                catch(BadFieldType &e)
                {
                    m_params.WasBadFieldTypeException();
                }
            }
            
            // check BufferTooSmall exception
            byte array[CInfoVectorByteSize];
            (*data)[10].AsBinary(array, CInfoVectorByteSize); // ok
            try
            {
                (*data)[10].AsBinary(array, CInfoVectorByteSize - 1); // failed
            }
            catch (BufferTooSmall &e)
            {
                m_params.WasBufferTooSmallException();
            }

            // check BadFieldIndex exception
            try
            {
                bool b = (*data)[CPackFieldsCount].AsBool(); // failed
            }
            catch(BadFieldIndex &e)
            {
                m_params.WasBadFieldIndexException();
            }

            m_params.setClientFinished();

            AsyncSetReadyForDeleteFlag(m_done);
        }

        void OnConnected() // override
        {
            /*
            // to check WStringOddSize
            Frame frame(TypeBinary, &CInfoVectorByte.at(0), CInfoVectorByte.size());
            word crc = CRC(static_cast<const byte*>(frame.DataPtr()), 0, frame.Length()).get();
            dword dataLen = frame.Length();

            byte protoVersion = 1;

            Utils::MemWriterDynStream stream;
            PacketHeader::WriteHeaderToStream(stream, PacketHeader::PTInfo, 
                                              protoVersion, crc, dataLen);


            // send
            Transport().Send(stream.GetBuffer(), stream.BufferSize());
            Transport().Send(frame.DataPtr(), frame.Length());
            */
        }
        
    public:
        Client(iCore::IThreadRunner &runner, TestParams &params) :
            ClientSideBase(runner, params), m_done(runner.getCompletedFlag()), 
            m_params(params)
        {
        }
    };

    // ---------------------------------------------------------------------

    class Server :
        public ServerSideBase
    {
        TestParams &m_params;
        
        void OnNewTransport() // override
        {
            {
                SbpSendPackInfo pack(Protocol());
                    
                pack.WriteByte(25);
                pack.WriteBool(true);
                pack.WriteWord(1256);
                pack.WriteDword(12345678);
                pack.WriteInt32(321654);
                pack.WriteInt64(10203065);
                pack.WriteFloat(25.2f);
                pack.WriteDouble(25.23);
                pack.WriteString("string");
                pack.WriteWstring(L"wstring");
                pack.WriteBinary(CInfoVectorByte);
                pack.WriteBinary(&CInfoVectorByte.at(0), CInfoVectorByte.size());
            } // send

            m_params.setServerFinished();
        }

        /*
        void InfoReceived(boost::shared_ptr<SbpRecvPack> data) // override
        {
            TUT_ASSERT(data.get() != 0);
            
            // check WStringOddSize exception
            try
            {
                std::wstring w = (*data)[0].AsWstring();
            }
            catch(WStringOddSize &e)
            {
                m_params.WasWStringOddSizeException();
            }
            
            m_params.setServerFinished();
        }
        */

        void OnSentPacketInfo(const SbpPackInfo &data, bool isSentPack) // override
        {
            // do nothing
        }

    public:
        Server(iCore::IThreadRunner &runner, TestParams &params) :
            ServerSideBase(runner, params), m_params(params)
        {
        }
    };

    // ---------------------------------------------------------------------

    class SbpExceptionsTestClass : public iCore::MsgObject
    {
        Server m_server;
        Client m_client;
        
    public:
        SbpExceptionsTestClass(iCore::IThreadRunner &runner, TestParams &params)
        : iCore::MsgObject(runner.getThread()), 
            m_server(runner, params), m_client(runner, params)
        {
        }
    };
    
} // namespace

// ---------------------------------------------------------------------

namespace SBPTests
{
    void SbpExceptionsTest(bool silentMode)
    {
        TestTcpTransportsCreator creator;
        TestParams params(creator, silentMode);
        iCore::ThreadRunner runner(Platform::Thread::LowPriority, 60* 1000);
        
        if (runner.Run<SbpExceptionsTestClass>("SbpExceptionsTestClass", params))
        {
            ESS_ASSERT(params.WasAllTestedExceptions());
            
            if (!silentMode) std::cout << "SbpExceptions test OK." << std::endl;
            return;
        }

        TUT_ASSERT(0 && "SbpExceptionsTest timeout");
    }
    
} // namespace SBPTests


