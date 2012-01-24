#ifndef __MFCLIENT__
#define __MFCLIENT__

#include "Utils/BinaryWriteBuff.h"

#include "MfBasicSendRecv.h"
#include "MfTranspPacket.h"
#include "MfPcProtocol.h"
#include "MfProtocol.h"
#include "MfPacket.h"
#include "MfPacketFactory.h"

namespace MiniFlasher
{
    using Platform::dword;

    class MfClient : MfBasicSendRecv, public IMfProtocol
    {
        ITransport &m_transp; 
        MfPacketFactory m_factory;
        Utils::ManagedList<MfPacket> m_packs;
        boost::scoped_ptr<MfPacket> m_currPack;

        void ThrowErr(MfTranspPacketRecv::ResultCode code)
        {
            ESS_THROW_MSG(ProtocolError, 
                          MfTranspPacketRecv::ResolveResultCode(code));
        }

        void ProcessPacket(const Utils::BinaryWriteBuff &pack, 
                           Utils::IBinaryReader &reader, byte cmd); // override 

        void OnProtocolLevelError(MfTranspPacketRecv::ResultCode code) // override
        {
            ThrowErr(code);
        }

        void OnPacketLevelError() // override
        {
            ThrowErr(MfTranspPacketRecv::ecPacketProcessException);
        }

    // IMfProtocol impl
    public:

        void SendConnect()
        {
            SendPack cmd(this, Protocol::CmdConnect);
        }

        void SendErase(dword offs, dword size)
        {
            SendPack cmd(this, Protocol::CmdErase);
            cmd.WriteDword(offs);
            cmd.WriteDword(size);
        }

        void SendWrite(bool compressed, const void *pData, dword dataSize)
        {
            SendPack cmd(this, Protocol::CmdWrite);
            cmd.WriteBool(compressed);
            cmd.WriteDword(dataSize);
            cmd.WriteData(pData, dataSize);
        }

        void SendVerify()
        {
            SendPack cmd(this, Protocol::CmdVerify);            
        }

        void SendRun(dword offs)
        {
            SendPack cmd(this, Protocol::CmdRun);
            cmd.WriteDword(offs);            
        }

        void SendRead(dword offset, dword size)
        {
            SendPack cmd(this, Protocol::CmdRead);
            cmd.WriteDword(offset);
            cmd.WriteDword(size);
        }

        void StartFlashTest()
        {
            SendPack cmd(this, Protocol::CmdFlashTest);
        }

        const MfPacket* Receive();

        dword MaxDataSize() const
        {
            return Protocol::CMaxDataPayload;
        }

    public:
        ESS_TYPEDEF(ProtocolError);

    public:
        MfClient(ITransport &transp, int recvTimeout = 5 * 1000);
        ~MfClient();
    };
    
}  // namespace MiniFlasher

#endif
