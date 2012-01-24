#ifndef __SBPSENDPACKBENCH__
#define __SBPSENDPACKBENCH__

#include "Platform/Platform.h"
#include "iCmp/ChMngProto.h"
#include "SafeBiProto/SbpSendPack.h"
#include "SafeBiProto/SbpPacketHeader.h"
#include "SafeBiProto/SbpRecvPack.h"

#include "Utils/MemWriterStream.h"
#include "Utils/MemReaderStream.h"
#include "Utils/BinaryReader.h"
#include "Utils/BinaryWriter.h"

namespace SBPTests
{
    using namespace SBProto;
    using Platform::byte;
    using Utils::MemWriterDynStream;
    
    class SbpSendPackBench : ISafeBiProtoForSendPack
    {
        int m_totalTime;
        int m_totalOps;
        Platform::int64 m_totalBytes;
        
        boost::scoped_ptr<MemWriterDynStream> m_sendPack;

        static std::string PacketToString(const SbpPackInfo &data)
        {
            std::ostringstream ss;

            // print cmd
            ss << data[0].AsString();

            for(int i = 1; i < data.Count(); ++i)
            {
                SBProto::TypeInfoDesc typeInfo = data[i].Type().TypeID;

                if (typeInfo == SBProto::TypeString) 
                {
                    ss << " \"" << data[i].Value() << "\""; 
                    continue;
                }

                ss << " " << data[i].Value();  
            }

            return ss.str();
        }

        static bool ReadPacket(MemWriterDynStream &pack)
        {
            byte *pData = pack.GetBuffer();
            int size = pack.BufferSize();

            // read header
            PacketHeaderParser headRecv;
            while(true)
            {
                if (size <= 0) return false;  // not enough data for header
                size--;
                if (headRecv.Add( *pData++ )) break;
            }

            PacketHeader header = headRecv.GetHeader();

            // read body
            if (size != header.getDataLength()) return false;  // not enough data for body

            SbpRecvPack recvPack(header);
            ISbpRecvPack& recvIntf = recvPack;

            size_t bytesUsed;
            if (!recvIntf.AddToPacketBody(pData, size, bytesUsed)) return false;
            if (bytesUsed != size) return false;

            // OK
            std::cout << PacketToString(recvPack.PackInfo()) << std::endl;
           
            return true;
        }

        /*
        static void ReadPacket(MemWriterDynStream &pack)
        {
            using namespace Utils;
            using namespace SBProto;

            boost::scoped_ptr<IBinaryReader> r( pack.CreateReader() );

            try
            {
                int size = pack.BufferSize();

                // read header
                PacketHeaderParser headRecv;
                while(true)
                {
                    size--;
                    if (headRecv.Add( r->ReadByte() )) break;
                }

                // body
                SbpRecvPack recvPack( headRecv.GetHeader() );
                ISbpRecvPack& recvIntf = recvPack;


            }
            catch(...)  // catch IO exceptions
            {
                throw;
            }

        } */

        SbpSendPackBench(int durationMs) : m_totalBytes(0)
        {            
            int start = Platform::GetSystemTickCount();
            int ops = 0;

            while(Platform::GetSystemTickCount() - start < durationMs)
            {
                ops += Task();
            }

            m_totalTime = Platform::GetSystemTickCount() - start;
            m_totalOps = ops;

            // verify m_sendPack
            ReadPacket(*m_sendPack);
        }

        int Task()
        {
            enum { CCount = 256 };

            iCmp::BfTdmInfo info;

            for(int i = 0; i < CCount; ++i)
            {
                iCmp::BfRespGetTdmInfo::Send(*this, info);
            }

            return CCount;
        }

        std::string Result()
        {
            const int KbDiv = 1024;

            int opsPerSec = (m_totalOps * 1000) / m_totalTime;
            int mbPerSec = (m_totalBytes * 1000) / m_totalTime / KbDiv;

            std::ostringstream ss;

            ss << "Packs per sec " << opsPerSec << "; Kb per sec " << mbPerSec;

            return ss.str();
        }

    // ISafeBiProtoForSendPack impl
    private:

        enum { CMaxSize = 1024 };

        byte ProtoVersion() const { return 1; }
        size_t PacketStreamCapacity() const { return CMaxSize; }

        void SendPacketToTransport(const Utils::MemWriterDynStream &header,
                const Utils::MemWriterDynStream &body, 
                PacketHeader::SbpPacketType packType)
        {
            if (m_sendPack == 0)
            {
                m_sendPack.reset( new MemWriterDynStream(CMaxSize) );
                m_sendPack->Write(header);
                m_sendPack->Write(body);
            }

            m_totalBytes += header.BufferSize();
            m_totalBytes += body.BufferSize();
        }

    public:
                 
        static void RunTest()
        {
            using namespace std;

            SbpSendPackBench bench(1000);

            // PC debug             ~3k / 700K
            // Bf debug + cache     1.9k / 440K

            cout << bench.Result() << std::endl;
        }

    };
    
}  // namespace SBPTests

#endif
