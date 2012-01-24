#include "stdafx.h"

#include "MfClient.h"

// -----------------------------------------------------

namespace
{
    /*
    using namespace MiniFlasher;

    class UartTransport : public ITransport
    {

    private:

        void Send(const void *pData, int dataSize)
        {

        }

        int Recv(void *pData, int buffSize)
        {
            return 0;
        }


    public:

        UartTransport()
        {
            // uart
            // ...
        }
    };

    // ----------------------------------------------------------

    class ClientOwner : IMfProtocolCallback
    {
        UartTransport m_uart;
        MfClient m_client;

    // IMfProtocolCallback impl
    private:

        void Connected(dword ver) {}
        void EraseProgress(dword size) {}
        void Erased() {}
        void Writed() {}
        void VerifyProgress(dword size) {}
        void Verified(dword crc) {}
        void Runned() {}
        void Readed(const void *pData, dword dataSize) {}
        void FlashTestProgress(dword progress, dword errors) {};
        void FlashTestCompleted(dword errors) {};
        void RespError(Protocol::Error error) {}
        void ProtocolError(MfTranspPacketRecv::ResultCode code) {}

    public:

        ClientOwner() : m_client(m_uart, *this)
        {
            // ... 
        }
    };
    
    // ----------------------------------------------------------

    void Run()
    {
        ClientOwner owner;
    }
    */
}

// -----------------------------------------------------

namespace MiniFlasher
{
    MfClient::MfClient(ITransport &transp, int recvTimeout) :
      MfBasicSendRecv(transp, recvTimeout),
      m_transp(transp), m_packs(true)
    {
    }

    // -----------------------------------------------------

    MfClient::~MfClient()
    {
        m_currPack.reset();
        m_packs.Clear();
    }

    // -----------------------------------------------------
    
    const MfPacket* MfClient::Receive()
    {
        m_currPack.reset();
        
        // process
        if (m_packs.IsEmpty()) ProcessRecv();

        // timeout
        if (m_packs.IsEmpty()) return 0;

        m_currPack.reset(m_packs.Detach(0));

        return m_currPack.get();
    }

    // -----------------------------------------------------
       
    void MfClient::ProcessPacket( const Utils::BinaryWriteBuff &pack, 
                                  Utils::IBinaryReader &reader, 
                                  byte cmd )
    {
        if (!Protocol::IsValidRespCode(cmd))
        {
            ThrowErr(MfTranspPacketRecv::ecUnknownCommand);
        }
        
        Protocol::RespCode type = static_cast<Protocol::RespCode>(cmd);

        try
        {
            m_packs.Add(m_factory.Create(type, reader));
        }
        catch (MfPacketFactory::BadDataSize &e)
        {
            OnProtocolLevelError(MfTranspPacketRecv::ecBadDataSize);
        }
        catch (Utils::NotEnoughData &e)
        {
            OnProtocolLevelError(MfTranspPacketRecv::ecBadDataSize);
        }
        catch (MfPacketField::UnknownProtocolError &e)
        {
            QString msg = QString("Unknown Protocol::Error: %1")
                                  .arg(e.getTextMessage().c_str());
            ESS_HALT(msg.toStdString());
        }

        // check that read all data from packet
        try
        {
            byte b = reader.ReadByte(); 
        }
        catch (Utils::NotEnoughData &e)
        {
            return;
        }

        OnProtocolLevelError(MfTranspPacketRecv::ecBadDataSize);
    }

    // -----------------------------------------------------
    
    /*
    void MfClient::ProcessPacket( const Utils::BinaryWriteBuff &pack, 
                                  Utils::IBinaryReader &reader, 
                                  byte cmd )
    {
        if (cmd == Protocol::RespConnected)
        {
            dword ver = reader.ReadDword();
            m_callback.Connected(ver);
            return;
        }

        if (cmd == Protocol::RespErased)
        {
            m_callback.Erased();
            return;
        }

        if (cmd == Protocol::RespEraseProgress)
        {
            dword erasedSize = reader.ReadDword();
            m_callback.EraseProgress(erasedSize);
            return;
        }

        if (cmd == Protocol::RespError)
        {
            Protocol::Error code = static_cast<Protocol::Error>(reader.ReadDword());
            m_callback.RespError(code);
            return;
        }

        if (cmd == Protocol::RespVerifyProgress)
        {
            dword size = reader.ReadDword();
            m_callback.VerifyProgress(size);
            return;
        }

        if (cmd == Protocol::RespVerifyDone)
        {
            dword crc32 = reader.ReadDword();
            m_callback.Verified(crc32);
            return;
        }

        if (cmd == Protocol::RespWrited)
        {
            m_callback.Writed();
            return;
        }

        if (cmd == Protocol::RespRunned)
        {
            m_callback.Runned();
            return;
        }

        if (cmd == Protocol::RespReaded)
        {
            const int CDataOffs = 1 + (4);  // byte cmd + dword size

            dword size = reader.ReadDword();

            if (size + CDataOffs != pack.Size()) 
            {
                m_callback.ProtocolError(MfTranspPacketRecv::ecBadDataSize);
                return;
            }

            const void *pData = pack.Begin(CDataOffs);
            m_callback.Readed(pData, size);
            return;
        }

        if (cmd == Protocol::RespFlashTestProgress)
        {
            dword percent = reader.ReadDword();
            dword errors = reader.ReadDword();
            m_callback.FlashTestProgress(percent, errors);
            return;
        }

        if (cmd == Protocol::RespFlashTestCompleted)
        {
            dword errors = reader.ReadDword();
            m_callback.FlashTestCompleted(errors);
            return;
        }

        m_callback.ProtocolError(MfTranspPacketRecv::ecUnknownCommand);
    }*/

}  // namespace MiniFlasher

