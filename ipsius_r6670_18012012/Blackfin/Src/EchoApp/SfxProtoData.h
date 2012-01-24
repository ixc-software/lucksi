#ifndef __SFXPROTODATA__
#define __SFXPROTODATA__

#include "SafeBiProto/SbpSerialize.h"

namespace EchoApp
{
    using SBProto::SbpRecvPack;
    
    typedef SBProto::SbpSendPackInfo SendPack;

    struct CmdInitData
    {
        std::string DeviceName;  // non-serializable

        int ProtoVersion;
        bool UseAlaw;
        Platform::dword VoiceChMask;
        bool T1Mode;

        CmdInitData()
        {
            ProtoVersion = CSfxProtoVersion;
            UseAlaw = true;
            VoiceChMask = 0;
            T1Mode = false;
        }

        CmdInitData(const SbpRecvPack &pack)
        {
            SBProto::PacketRead s(pack, 1);
            Serialize(*this, s);
        }

        void ToPacket(SendPack &p) const
        {
            SBProto::PacketWrite s(p);
            Serialize(*this, s);
        }

    private:

        template<class Data, class TStream>
        static void Serialize(Data &data, TStream &s)
        {
            s 
                << data.ProtoVersion 
                << data.UseAlaw
                << data.VoiceChMask
                << data.T1Mode;
        }
    };
    
}  // namespace EchoApp


#endif
