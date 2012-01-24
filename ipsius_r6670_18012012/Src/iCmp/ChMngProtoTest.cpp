#include "stdafx.h"
#include "ChMngProtoTest.h"
#include "ChMngProto.h"
#include "SafeBiProto/SafeBiProto.h"
#include "SafeBiProto/SbpPackInfo.h"

// -----------------------------------------------

using namespace SBProto;

namespace
{

    class SendIntf : public ISafeBiProtoForSendPack
    {
        boost::scoped_ptr<SbpPackInfo> m_info;

    // ISafeBiProtoForSendPack impl
    private:

        byte ProtoVersion() { return 1; }
        size_t PacketStreamCapacity() { return 1024; }

        void SendPacketToTransport(Utils::MemWriterDynStream &header,
            Utils::MemWriterDynStream &body, 
            PacketHeader::SbpPacketType packType)
        {
            m_info.reset( new SbpPackInfo(header, body) );
        }
        

    public:

        SendIntf() 
        {
            // ...
        }

        std::string GetInfo()
        {
            // TODO -- CMP pack dumper class
            // ...
        }

    };

}  // namespace

// -----------------------------------------------

namespace iCmp
{
	
	void CmpTest()
	{
        SendIntf send;

        // PcCmdInit::Send(send, "TDM", 16, "1..15, 17..31");

	}
	
}  // namespace Dss1ToSip
