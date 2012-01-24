
#include "stdafx.h"
#include "SafeBiProtoTests.h"

#include "SafeBiProtoTestBase.h"

namespace
{
    using namespace SBProto;
    using namespace SBPTests;
    
    const int CTestTimeoutMs = 10 * 1000;
    
    // -----------------------------------------------------------

    /*
        Test scheme:

        client                                        server
        | (1)                                                 
        OnConnected(): ---SendInfo()------> OnInfoReceived()
                                                           | (2): register cmd in monitor
        OnCommandReceived() <------SendCommand()-----------
        | (3)
        -----------SendResponce()-----> OnResponceReceived()
        (finish)                                           | (4)
                                                    (finish)

        * all packets' interfaces methods tests via info packet
    */
    
    // -----------------------------------------------------------

    const int CSendInfoCounter = 3;
    
    // test variables

    const byte CStage1InfoByte = 1;
    const bool CStage1InfoBool = false;
    const word CStage1InfoWord = 12456;
    const dword CStage1InfoDword = 12356874;
    const int32 CStage1InfoInt32 = -124568;
    const int64 CStage1InfoInt64 = -45821388;
    const float CStage1InfoFloat = 12.356f;
    const double CStage1InfoDouble = 12.57895444;
    const std::string CStage1InfoString = "string";
    const std::wstring CStage1InfoWstring = L"wstring";
    const size_t CStage1InfoVectorByteSize = 5;
    std::vector<byte> CStage1InfoVectorByte = std::vector<byte>(CStage1InfoVectorByteSize, 9);
    const size_t CStage1InfoFrameCount = 17; // 12 + 5 empty frames
    
    const std::string CStage2CmdName = "set_flag";
    const bool CStage2CmdParam = true;
    
    /*const*/ std::string CStage3Responce = "flag_set_true";
    
    // -----------------------------------------------------------
    
    class ClientSide: public ClientSideBase
    {
        MiniLogger m_log;
        SafeBiProtoTestParams &m_params;

    private:
        void OnCommandReceived(boost::shared_ptr<SbpRecvPack> data) // override
        {
            TUT_ASSERT(data.get() != 0);
            m_log << "Command received"; // << data->ToString();

            TUT_ASSERT(data->Curr().AsString() == CStage2CmdName);
            data->Next();
            TUT_ASSERT(data->Curr().AsBool() == CStage2CmdParam);

            // Stage 3:
            {
                SbpSendPackResp pack(Protocol());
                pack.WriteString(CStage3Responce);
                TUT_ASSERT(!pack.ToString().empty());
                
                m_log << "Sending responce"; // << pack.ToString();
                
            } // send

            m_log << "Finished";
            Protocol().Deactivate();
            m_params.setClientFinished();
        }

        void OnProtocolError(boost::shared_ptr<SbpError> err)  // override
        {
            TUT_ASSERT(err.get() != 0);
            ESS_HALT(err->ToString());
        }

        void OnConnected() // override
        {
            m_log << "Connected";

            // Stage 1:
            for (int i = 0; i < CSendInfoCounter; ++i)
            {
                SbpSendPackInfo pack(Protocol());
                
                pack.WriteByte(CStage1InfoByte);
                pack.WriteBool(CStage1InfoBool);
                pack.WriteWord(CStage1InfoWord);
                pack.WriteDword(CStage1InfoDword);
                pack.WriteInt32(CStage1InfoInt32);
                pack.WriteInt64(CStage1InfoInt64);
                pack.WriteFloat(CStage1InfoFloat);
                pack.WriteDouble(CStage1InfoDouble);
                pack.WriteString(CStage1InfoString);
                pack.WriteWstring(CStage1InfoWstring);
                pack.WriteBinary(CStage1InfoVectorByte);
                pack.WriteBinary(&CStage1InfoVectorByte.at(0), CStage1InfoVectorByte.size());
                // write frames with empty data
                pack.WriteString("");
                pack.WriteWstring(L"");
                pack.WriteBinary(std::vector<byte>());
                pack.WriteBinary(&CStage1InfoVectorByte.at(0), 0);
                pack.WriteBinary(0, 0);
                
                m_log << "Sending info";// << pack.ToString();
            } //send
        }
        
        
    public:
        ClientSide(iCore::IThreadRunner &runner, SafeBiProtoTestParams &params)
        : ClientSideBase(runner, params),
            m_log("Client", params.getSilentMode()), m_params(params)
        {
        }
    };

    // -----------------------------------------------------------

    // One-session server
    class ServerSide: 
        public ServerSideBase
    {
        MiniLogger m_log;
        Utils::AtomicBool &m_done;
        SafeBiProtoTestParams &m_params;
        
        int m_infoCounter;
        bool m_hasSendCallback;
        bool m_hasRecvCallback;

    private:
        void OnResponseReceived(boost::shared_ptr<SbpRecvPack> data) // override
        {
            TUT_ASSERT(data.get() != 0);
            m_log << "Responce received"; // << data->ToString();

            // Stage 4:
            TUT_ASSERT(data->Curr().AsString() == CStage3Responce);

            m_log << "Finished";
            m_params.setServerFinished();

            TUT_ASSERT(m_infoCounter == CSendInfoCounter);

            AsyncSetReadyForDeleteFlag(m_done);
        }
        
        void OnInfoReceived(boost::shared_ptr<SbpRecvPack> data) // override
        {
            TUT_ASSERT(data.get() != 0);
            ++m_infoCounter;

            m_log << "Info received"; // << data->ToString();
            
            // read and check
            TUT_ASSERT(!data->ToString().empty());
            TUT_ASSERT(data->Count() == CStage1InfoFrameCount);
            TUT_ASSERT((*data)[0].AsByte() == CStage1InfoByte);
            TUT_ASSERT((*data)[1].AsBool() == CStage1InfoBool);
            TUT_ASSERT((*data)[2].AsWord() == CStage1InfoWord);
            TUT_ASSERT((*data)[3].AsDword() == CStage1InfoDword);
            TUT_ASSERT((*data)[4].AsInt32() == CStage1InfoInt32);
            TUT_ASSERT((*data)[5].AsInt64() == CStage1InfoInt64);
            TUT_ASSERT((*data)[6].AsFloat() == CStage1InfoFloat);
            TUT_ASSERT((*data)[7].AsDouble() == CStage1InfoDouble);
            TUT_ASSERT((*data)[8].AsString() == CStage1InfoString);
            TUT_ASSERT((*data)[9].AsWstring() == CStage1InfoWstring);
            TUT_ASSERT((*data)[10].AsBinary() == CStage1InfoVectorByte);

            std::string str;
            (*data)[8].AsString(str);
            TUT_ASSERT(str == CStage1InfoString);

            std::wstring wstr;
            (*data)[9].AsWstring(wstr);
            TUT_ASSERT(wstr == CStage1InfoWstring);

            std::vector<byte> vec;
            (*data)[10].AsBinary(vec);
            TUT_ASSERT(vec == CStage1InfoVectorByte);
            
            byte array[CStage1InfoVectorByteSize];
            (*data)[11].AsBinary(array, CStage1InfoVectorByteSize);
            for (size_t i = 0; i < CStage1InfoVectorByteSize; ++i)
            {
                TUT_ASSERT(array[i] == CStage1InfoVectorByte.at(i));
            }
            

            // next is empty
            TUT_ASSERT((*data)[12].AsString() == std::string());
            TUT_ASSERT((*data)[13].AsWstring() == std::wstring());
            TUT_ASSERT((*data)[14].AsBinary() == std::vector<byte>());
            TUT_ASSERT((*data)[15].AsBinary() == std::vector<byte>());
            TUT_ASSERT((*data)[16].AsBinary() == std::vector<byte>());
            TUT_ASSERT(!data->ToString().empty());
            
            m_log << data->ToString();
            
            if (m_infoCounter > 1) return;
            
            // Stage 2:
            {
                SbpSendPackCmd pack(Protocol());
                pack.WriteString(CStage2CmdName).WriteBool(CStage2CmdParam);
                
                m_log << "Sending cmd"; // << pack.ToString();
                
            } // send pack in ~SbpSendPackCmd()
        }

       
        void OnProtocolError(boost::shared_ptr<SbpError> err) // override
        {
            TUT_ASSERT(err.get() != 0);
            ESS_HALT(err->ToString());
        }
        
        void OnNewTransport() // override
        {
            // do nothing
        }
        
        
        void OnSentPacketInfo(const SbpPackInfo &data, bool isSentPack)
        {
            if ((!m_hasSendCallback) && (isSentPack))
            {
                m_hasSendCallback = true;
        
                // on Stage 2
                TUT_ASSERT(data.Count() == 2);
                TUT_ASSERT(data[0].AsString() == CStage2CmdName);
                TUT_ASSERT(data[1].AsBool() == CStage2CmdParam);
        
                // m_log << data.ConvertToString();
            }
        
            if ((!m_hasRecvCallback) && (!isSentPack))
            {
                m_hasRecvCallback = true;
        
                TUT_ASSERT(data.Count() == CStage1InfoFrameCount);
                // check few fields
                TUT_ASSERT(data[0].AsByte() == CStage1InfoByte);
                TUT_ASSERT(data[1].AsBool() == CStage1InfoBool);
            }
        }

    public:
        ServerSide(iCore::IThreadRunner &runner, SafeBiProtoTestParams &params)
        : ServerSideBase(runner, params),
            m_log("Server", params.getSilentMode()), m_done(runner.getCompletedFlag()),
            m_params(params), m_infoCounter(0), 
            m_hasSendCallback(false), m_hasRecvCallback(false)
        {
        }

        ~ServerSide()
        {
            TUT_ASSERT(m_hasSendCallback);
            TUT_ASSERT(m_hasRecvCallback);
        }
    };
    
    // -----------------------------------------------------------
    
    class SafeBiProtoTestClass : public iCore::MsgObject
    {
        ServerSide m_server;
        ClientSide m_client;
        
    public:
        SafeBiProtoTestClass(iCore::IThreadRunner &runner, SafeBiProtoTestParams &params)
        : iCore::MsgObject(runner.getThread()), 
            m_server(runner, params), m_client(runner, params)
        {
        }
    };
    
    
} // namespace

// -----------------------------------------------------------

namespace SBPTests
{
    using namespace SBProto; 
    
    void SafeBiProtoTestSimple(ITestTransportsCreator &creator, bool testWithMsgs, 
                               bool silentMode)
    {
        if (!silentMode) std::cout<< "\nSafeBiProtoTestSimple start\n";

        SafeBiProtoTestParams params(creator, testWithMsgs, silentMode);
        iCore::ThreadRunner test(Platform::Thread::LowPriority, CTestTimeoutMs);
        
        if (test.Run<SafeBiProtoTestClass>("SafeBiProtoTestClass", params))
        {
            params.CheckFinished();

            if (!silentMode) std::cout << "SafeBiProto simple test okay." << std::endl;
            return;
        }
        
        TUT_ASSERT(0 && "SafeBiProto simple test timeout");
    }
       
} // namespace SBPTests 
    
