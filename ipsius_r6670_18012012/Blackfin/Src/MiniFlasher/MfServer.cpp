#include "stdafx.h"

#include "Utils/BinaryWriter.h"
#include "Utils/MemWriterStream.h"
#include "Utils/ZlibPipe.h"

#include "MfBasicSendRecv.h"
#include "MfServer.h"
#include "MfProtocol.h"

// ---------------------------------------

namespace
{
    using Platform::byte;
    using namespace MiniFlasher;

    enum
    {
        CDummyFlashWrite = false,    // debug, set true for test raw UART speed
    };

    class MfServer : MfBasicSendRecv, IFlashTestCallback
    {

        enum State
        {
            StDisconnected,
            StConnected,
            StErased,
        };

        class TestState
        {
            int m_prevPercent;
            int m_errors;

        public:

            TestState()
            {
                Clear();
            }

            void Clear()
            {
                m_prevPercent = 0;
                m_errors = 0;
            }

            int TotalErrors() const { return m_errors; }

            bool Update(int percent, int readErrors, int writeErrors)
            {
                const int CPercentUpdateStep = 2;

                m_errors = readErrors + writeErrors;

                if ((percent - m_prevPercent >= CPercentUpdateStep) || (percent == 100))
                {
                    m_prevPercent = percent;
                    return true;
                }

                return false;
            }
        };

        ServerProfile m_profile;

        State m_state;
        dword m_writedBytes;
        dword m_writeCrc;
        dword m_writeStartPos;

        TestState m_test;

        void EraseFlash(dword offs, dword size)
        {
            if (size == 0 || size > m_profile.Flash.FlashSize())
            {
                SendError(Protocol::ErrBadArgValue);
                return;
            }

            if (!m_profile.Flash.EraseBegin(offs))
            {
                SendError(Protocol::ErrFlashErase);
                return;
            }

            dword erased = 0;

            while(erased < size)
            {
                if (!m_profile.Flash.EraseNext(erased))
                {
                    SendError(Protocol::ErrFlashErase);
                    return;
                }

                SendPack resp(this, Protocol::RespEraseProgress);
                resp.WriteDword(erased);
            }

            // done
            m_state = StErased;

            m_writeStartPos = offs;
            m_writedBytes = 0;
            m_writeCrc = Utils::RawCRC32::InitialValue;

            m_profile.Flash.BeginWrite(m_writeStartPos);

            // send OK
            SendPack resp(this, Protocol::RespErased);
        }

        void FlashWriteOK()
        {
            SendPack resp(this, Protocol::RespWrited);
        }

        bool FlashWrite(const Utils::BinaryWriteBuff &pack, dword blockSize, bool compressed)
        {
            const int CDataOffs = 1 + (1 + 4);  // byte cmd + bool compressed + dword size

            // state verify
            if (m_state != StErased)
            {
                SendError(Protocol::ErrBadState);
                return false;
            }

            // size verify
            if (blockSize + CDataOffs != pack.Size()) 
            {
                SendError(Protocol::ErrPacketArg);
                return false;
            }

            // write position align
            if (m_writedBytes % m_profile.Flash.FlashWriteGranularity() != 0 ||
                blockSize == 0)
            {
                SendError(Protocol::ErrBadArgValue);
                return false;
            }

            // send OK in async mode
            bool async = m_profile.AsyncMode;

            if (async) FlashWriteOK();  // yes, it is ready before done

            // extract
            const void *pData = pack.Begin(CDataOffs);
            int dataSize = blockSize;

            BinaryWriteBuff dataExtracted(Protocol::CMaxDataPayload);
            if (compressed)
            {
                Utils::ZlibInflate inf(dataExtracted);

                bool finalized;
                int size = inf.Add(pData, dataSize, finalized);
                if (!finalized || size != dataSize) 
                {
                    SendError(Protocol::ErrExtractError);
                    return false;
                }

                pData = dataExtracted.Begin();
                dataSize = dataExtracted.Size();
            }

            // write 
            if (!CDummyFlashWrite)
            {
                if ( !m_profile.Flash.Write(pData, dataSize) )
                {
                    SendError(Protocol::ErrFlashWrite);
                    return false; 
                }
            }

            if (!async) FlashWriteOK();

            m_writedBytes += dataSize;
            m_writeCrc = Utils::UpdateCRC32(pData, dataSize, m_writeCrc);

            return true;
        }

        void FlashRead(dword offs, dword size)
        {
            if (size > Protocol::CMaxDataPayload) 
            {
                SendError(Protocol::ErrPacketArg);
                return;
            }

            BinaryWriteBuff buff(size);
            bool res = m_profile.Flash.Read(buff.End(), offs, size);
            if (!res)
            {
                SendError(Protocol::ErrFlashRead);
                return;                
            }
            buff.AddToSize(size);

            SendPack pack(this, Protocol::RespReaded);
            pack.WriteDword(size);
            pack.WriteData(buff.Begin(), size);
        }


        bool FlashVerify(dword &crc32)
        {
            const int CReadBuffSize = 8 * 1024;
            const int CCallbackSize = CReadBuffSize * 16;

            std::vector<byte> buff(CReadBuffSize, 0);

            dword offs = 0;
            crc32 = Utils::RawCRC32::InitialValue;

            while(offs < m_writedBytes)
            {
                int bytesToRead = m_writedBytes - offs;
                if (bytesToRead > CReadBuffSize) bytesToRead = CReadBuffSize;

                if (!m_profile.Flash.Read(&buff[0], m_writeStartPos + offs, bytesToRead)) return false;

                bool finalize = (offs + bytesToRead == m_writedBytes);
                crc32 = Utils::UpdateCRC32(&buff[0], bytesToRead, crc32, finalize);

                offs += bytesToRead;

                if (offs % CCallbackSize == 0)
                {
                    SendPack resp(this, Protocol::RespVerifyProgress);
                    resp.WriteDword(offs);
                }
            }

            if (crc32 != m_writeCrc)
            {
                // std::cout << "CRC flash bug!" << std::endl;  // debug
            }

            return true;
        }

        void RunFlashTest()
        {
            m_test.Clear();

            m_profile.Flash.RunFlashTest(*this);

            SendPack pack(this, Protocol::RespFlashTestCompleted);
            pack.WriteDword(m_test.TotalErrors());
        }


        // override
        void OnProtocolLevelError(MfTranspPacketRecv::ResultCode code)
        {
            SendError(Protocol::ErrPacketLevelError);
        }

        // override
        void OnPacketLevelError()
        {
            SendError(Protocol::ErrPacketArg);
        }

        // parse server side packets (commands)
        // override
        void ProcessPacket(const Utils::BinaryWriteBuff &pack, 
            Utils::IBinaryReader &reader,            
            byte cmd)
        {
            if (cmd == Protocol::CmdConnect)
            {
                m_state = StConnected;

                SendPack resp(this, Protocol::RespConnected);
                resp.WriteDword(Protocol::CVersion);
                return;
            }

            // other commands allowed only in connected state
            if (m_state == StDisconnected)
            {
                SendError(Protocol::ErrBadState);
                return;
            }

            if (cmd == Protocol::CmdErase)
            {            	
                dword offs = reader.ReadDword();
                dword size = reader.ReadDword();
                EraseFlash(offs, size);
                return;
            }

            if (cmd == Protocol::CmdWrite)
            {
                bool compressed = reader.ReadBool();
                dword size = reader.ReadDword();

                FlashWrite(pack, size, compressed);

                return;
            }

            if (cmd == Protocol::CmdVerify)
            {
                dword crc32;

                if (FlashVerify(crc32))
                {
                    SendPack resp(this, Protocol::RespVerifyDone);
                    resp.WriteDword(crc32);
                }
                else
                {
                    SendError(Protocol::ErrFlashVerify);
                }

                return;
            }

            if (cmd == Protocol::CmdRun)
            {
                dword offs = reader.ReadDword();

                // send OK
                {
                    SendPack resp(this, Protocol::RespRunned);
                }

                // wait and run
                Platform::ThreadSleep(200);  // wait for send pack transmited
                m_profile.Sys.Run(offs);

                // never goes here
                return;
            }

            if (cmd == Protocol::CmdRead)
            {
                dword offs = reader.ReadDword();
                dword size = reader.ReadDword();
                FlashRead(offs, size);
                return; 
            }

            // flash test
            if (cmd == Protocol::CmdFlashTest)
            {
                RunFlashTest();
                return;
            }

            // unknown cmd 
            SendError(Protocol::ErrUnknownCmd);
        }

    // IFlashTestCallback impl
    private:

        void FlashTestState(int percent, int readErrors, int writeErrors)
        {
            if (m_test.Update(percent, readErrors, writeErrors))
            {
                SendPack pack(this, Protocol::RespFlashTestProgress);
                pack.WriteDword(percent);
                pack.WriteDword(m_test.TotalErrors());
            }
        }

    public:

        MfServer(const ServerProfile &profile) : 
          MfBasicSendRecv(profile.Transport, profile.RecvTimeoutMs),
          m_profile(profile)
        {
            m_state = StDisconnected;
            m_writedBytes = 0;
            m_writeCrc = 0;
            m_writeStartPos = 0;
        }

        void Run()
        {
            m_state = StDisconnected;

            m_profile.Sys.Leds(true);

            int loopTimer = Platform::GetSystemTickCount();

            while(true)
            {
                // leds
                if (Platform::GetSystemTickCount() - loopTimer > 1000)
                {
                    m_profile.Sys.Leds(false);
                }

                // do recv 
                if (!ProcessRecv()) continue;

                // sleep
                m_profile.Sys.Sleep();
            }
        }

    };    

}  // namespace


// ---------------------------------------

namespace MiniFlasher
{

    void RunMfServer(const ServerProfile &profile)
    {
        MfServer srv(profile);
        srv.Run();
    }

    
}  // namespace MiniFlasher
