#ifndef __MFBASICSENDRECV__
#define __MFBASICSENDRECV__

#include "Utils/IntToString.h"

#include "MfTrace.h"
#include "MfTranspPacket.h"

namespace MiniFlasher
{
    using Platform::byte;
    
    class MfBasicSendRecv : boost::noncopyable
    {
        ITransport &m_transp;

        MfTranspPacketSender m_sender;
        MfTranspPacketRecv m_recv;

        std::vector<byte> m_sendBuff;
        Utils::MemWriterStream m_sendStream;
        bool m_sendStreamUsed;

        void DoPacketProcess(const Utils::BinaryWriteBuff &pack)
        {
            Utils::MemReaderStream rStream(pack.Begin(), pack.Size());
            Utils::BinaryReader<Utils::MemReaderStream> reader(rStream);

            byte cmd = reader.ReadByte();

            ProcessPacket(pack, reader, cmd);
        }

        class RecvRaiiClear : boost::noncopyable
        {
            MfTranspPacketRecv &m_recv;
        public:
            RecvRaiiClear( MfTranspPacketRecv &recv) : m_recv(recv) {}
            ~RecvRaiiClear() { m_recv.Clear(); }
        };

        // RAII for class SendPack
        Utils::MemWriterStream& AllocSendStream()
        {
            ESS_ASSERT(!m_sendStreamUsed);

            m_sendStreamUsed = true;
            m_sendStream.ClearBufferSize();

            return m_sendStream;
        }

        // free and send 
        void FreeSendStream()
        {
            ESS_ASSERT(m_sendStreamUsed);
            ESS_ASSERT(m_sendStream.BufferSize() > 0);

            m_sender.Send(m_sendStream.GetBuffer(), m_sendStream.BufferSize());
            m_sendStreamUsed = false;
        }

    protected:

        class SendPack : public Utils::BinaryWriter<Utils::MemWriterStream>
        {
            MfBasicSendRecv &m_owner;

            static MfBasicSendRecv& FromPtr(MfBasicSendRecv *pOwner)
            {
                ESS_ASSERT(pOwner != 0);
                return *pOwner;
            }

        public:

            // code is Protocol::RespCode or ::CmdCode
            SendPack(MfBasicSendRecv *pOwner, int code) : 
              Utils::BinaryWriter<Utils::MemWriterStream>( FromPtr(pOwner).AllocSendStream() ),
              m_owner(FromPtr(pOwner))
            {
                WriteByte(code);
            }

            ~SendPack()
            {
                m_owner.FreeSendStream();
            }
        };

    protected:

        // debug
        int SendPackets() const { return m_sender.SendPackets(); }

        virtual void ProcessPacket(const Utils::BinaryWriteBuff &pack, 
            Utils::IBinaryReader &reader,
            byte cmd) = 0;

        virtual void OnProtocolLevelError(MfTranspPacketRecv::ResultCode code) = 0;
        virtual void OnPacketLevelError() = 0;

        void SendError(Protocol::Error errorCode)
        {
            SendPack pack(this, Protocol::RespError);
            pack.WriteDword(errorCode);
        }

        // return true on MfTranspPacketRecv::ecOK
        bool ProcessRecv()
        {
            MfTranspPacketRecv::ResultCode code = m_recv.Process();

            // packet
            if (code == MfTranspPacketRecv::ecPackRecv)
            {
                RecvRaiiClear recvClear(m_recv);

                try
                {
                    DoPacketProcess( m_recv.Packet() );
                }
                catch(/* const */ Utils::BRStreamException &e)
                {
                    OnPacketLevelError();
                }

                return false;
            }

            // error
            if (code != MfTranspPacketRecv::ecOK)
            {
                OnProtocolLevelError(code);
                return false;
            }

            // OK
            ESS_ASSERT(code == MfTranspPacketRecv::ecOK);
            return true;
        }

    public:

        MfBasicSendRecv(ITransport &transp, int recvTimeout) :
          m_transp(transp),
          m_sender(transp),
          m_recv(transp, recvTimeout),
          m_sendBuff(Protocol::CMaxPacketBodySize),
          m_sendStream(&m_sendBuff[0], m_sendBuff.size())
        {
            m_sendStreamUsed = false;
        }

        virtual ~MfBasicSendRecv()
        {
        }
    };
    
}  // namespace MiniFlasher


#endif
