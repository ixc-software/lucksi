#ifndef __MFTRANSPPACKET__
#define __MFTRANSPPACKET__

#include "Platform/Platform.h"
#include "Utils/BinaryWriteBuff.h"
#include "Utils/IBasicInterface.h"
#include "Utils/MemWriterStream.h"
#include "Utils/BinaryReader.h"
#include "Utils/BinaryWriter.h"
#include "Utils/MemReaderStream.h"
#include "Utils/RawCRC32.h"
#include "Utils/IntToString.h"

#include "MiniFlasher/MfDebugConfig.h"
#include "MfTrace.h"
#include "MfProtocol.h"

namespace MiniFlasher
{
    using Platform::byte;
    using Platform::word;
    using Platform::dword;
    using Utils::BinaryWriteBuff;

    class ITransport : public Utils::IBasicInterface
    {
    public:
        virtual void Send(const void *pData, int dataSize) = 0;
        virtual int Recv(void *pData, int buffSize) = 0;
    };


    // -------------------------------------------

    struct PackHeader
    {
        enum 
        {
            CSize       = 7,
        };

        byte      ID;
        word      DataSize;
        dword     DataCRC32;

        // data[]
    };

    BOOST_STATIC_ASSERT(PackHeader::CSize <= sizeof(PackHeader));

    // -------------------------------------------

    class MfTranspPacketSender : boost::noncopyable
    {
        ITransport &m_transport;
        MfTrace m_trace;
        int m_sendPackets;
        int m_sendBytes;

    public:

        MfTranspPacketSender(ITransport &transport) : 
          m_transport(transport),
          m_trace(false, CTraceMaxOutputBytes, CTraceOutput)
        {
            m_sendPackets = 0;
            m_sendBytes = 0;
        }

        void Send(const void *pData, int dataSize)
        {
            ++m_sendPackets;
            m_sendBytes += PackHeader::CSize + dataSize;

            // trace
            m_trace.Trace(pData, dataSize);

            // header
            {
                byte buff[PackHeader::CSize];
                Utils::MemWriterStream stream(buff, sizeof(buff));
                Utils::BinaryWriter<Utils::MemWriterStream> writer(stream);

                writer.WriteByte(Protocol::CHeaderID);
                writer.WriteWord(dataSize);
                writer.WriteDword( Utils::UpdateCRC32(pData, dataSize) );

                m_transport.Send(buff, sizeof(buff));
            }

            // data
            m_transport.Send(pData, dataSize);
        }

        int SendPackets() const { return m_sendPackets; }

    };

    // -------------------------------------------

    class MfTranspPacketRecv : boost::noncopyable
    {
    public:

        enum ResultCode  // sync it with ResolveResultCode() - !
        {
            ecOK,
            ecPackRecv,

            // recv errors
            ecBadID,
            ecBadDataSize,
            ecBadCRC,
            ecRecvTimeout,

            // upper level errors
            ecUnknownCommand,
            ecPacketProcessException,
        };

        static std::string ResolveResultCode(ResultCode code)
        {
            #define VAL_DECODE(m) if (code == m) return #m;

            VAL_DECODE(ecOK);
            VAL_DECODE(ecPackRecv);

            VAL_DECODE(ecBadID);
            VAL_DECODE(ecBadDataSize);
            VAL_DECODE(ecBadCRC);
            VAL_DECODE(ecRecvTimeout);

            VAL_DECODE(ecUnknownCommand);
            VAL_DECODE(ecPacketProcessException);

            #undef VAL_DECODE

            return "Unknown code!";
        }

    public:

        MfTranspPacketRecv(ITransport &transport, int recvTimeout) :
          m_transport(transport),
          m_recvTimeout(recvTimeout),
          m_trace(true, CTraceMaxInputBytes, CTraceInput),
          m_headerBuff(PackHeader::CSize),
          m_dataBuff(Protocol::CMaxPacketBodySize)
        {
            Clear();
        }

        void Clear()
        {
            m_headerBuff.Clear();
            m_dataBuff.Clear();
            m_lastDataRecvTime = 0;
        }

        // user must call Clear() if result was ecPackRecv
        ResultCode Process()
        {
            // assert -- no packet ready
            ESS_ASSERT(!DataCompleted());

            ResultCode code = ProcessRecv();

            // clear recvied data on error
            if (code != ecOK && code != ecPackRecv)
            {
                Clear();
            }

            return code;
        }

        // get packet
        const BinaryWriteBuff& Packet() const
        {
            ESS_ASSERT( DataCompleted() );

            return m_dataBuff;
        }


    private:


        ITransport &m_transport; 
        int m_recvTimeout;
        MfTrace m_trace;

        dword m_lastDataRecvTime;

        // header
        BinaryWriteBuff m_headerBuff;
        PackHeader m_header;

        // data
        BinaryWriteBuff m_dataBuff;

        ResultCode ProcessRecv()
        {
            // timeout check
            if (m_lastDataRecvTime != 0)
            {
                if (Platform::GetSystemTickCount() - m_lastDataRecvTime > m_recvTimeout)
                {
                    return ecRecvTimeout;
                }
            }

            // header not ready
            if (m_headerBuff.Size() < PackHeader::CSize)
            {
                ResultCode code = ReadHeader();
                if (code != ecOK) return code;  // error
            }

            // read data if header ready
            if (HeaderCompleted())
            {
                return ReadData();
            }

            return ecOK;
        }

        bool HeaderCompleted() const
        {
            return (m_headerBuff.Full());
        }

        bool DataCompleted() const
        {
            if(!HeaderCompleted()) return false;
            
            return (m_dataBuff.Size() == m_header.DataSize);
        }


        int Recv(void *pData, int size)
        {
            int res = m_transport.Recv(pData, size);

            if (res > 0) m_lastDataRecvTime = Platform::GetSystemTickCount();
            
            return res;
        }

        ResultCode ReadHeader() 
        {
            ESS_ASSERT(!m_headerBuff.Full());  // header not completed

            int bytesToRead = m_headerBuff.FreeBytes();                
            int bytes = Recv(m_headerBuff.End(), bytesToRead);
            m_headerBuff.AddToSize(bytes);

            if (bytes <= 0) return ecOK;

            // check ID
            if (m_headerBuff.ReadByte(0) != Protocol::CHeaderID) 
            {
                return ecBadID;
            }

            // is header completed?
            if (!m_headerBuff.Full()) return ecOK;
            
            // m_headerBuff -> m_header
            Utils::MemReaderStream s(m_headerBuff.Begin(), m_headerBuff.Size());
            Utils::BinaryReader<Utils::MemReaderStream> br(s);

            m_header.ID = br.ReadByte();
            m_header.DataSize = br.ReadWord();
            m_header.DataCRC32 = br.ReadDword();

            // verify
            if (m_header.DataSize > Protocol::CMaxPacketBodySize || 
                m_header.DataSize == 0)
            {
                return ecBadDataSize;
            }

            ESS_ASSERT(m_header.ID == Protocol::CHeaderID);
            ESS_ASSERT(m_dataBuff.Empty());
            ESS_ASSERT(m_dataBuff.Capacity() >= m_header.DataSize);
           
            return ecOK;
        }

        ResultCode ReadData()
        {
            ESS_ASSERT( HeaderCompleted() );
            ESS_ASSERT( !DataCompleted() ); 
            
            // read
            int bytesToRead = m_header.DataSize - m_dataBuff.Size();
            ESS_ASSERT(bytesToRead <= m_dataBuff.FreeBytes());
            int bytes = Recv(m_dataBuff.End(), bytesToRead); 
            m_dataBuff.AddToSize(bytes);

            // is packet ready?
            if (!DataCompleted()) return ecOK;

            // CRC
            dword crc = Utils::UpdateCRC32(m_dataBuff.Begin(), m_dataBuff.Size());
            if (crc != m_header.DataCRC32)
            {
                return ecBadCRC;
            }

            // trace
            m_trace.Trace(m_dataBuff.Begin(), m_dataBuff.Size());

            return ecPackRecv; 
        }

    };


}  // MiniFlasher


#endif
