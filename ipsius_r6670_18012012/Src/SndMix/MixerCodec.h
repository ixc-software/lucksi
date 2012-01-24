#ifndef __MIXERCODEC__
#define __MIXERCODEC__

#include "Platform/PlatformTypes.h"
#include "Utils/IBasicInterface.h"
#include "iRtp/RtpPayload.h"

#include "MixerError.h"

namespace SndMix
{
    using Platform::int16;
    using Platform::byte;

    class ICodecInfo;
    
    class IEncoder : public Utils::IBasicInterface
    {
    public:
        virtual int Encode(const int16 *pData, int size, byte *pOutput, int maxOutputSize) = 0;
        virtual int TypicalBlockSizeForEncode() const = 0;

        virtual const ICodecInfo& EncoderInfo() const = 0;

        // helpers for vector and BidirBuff
        // ... 
    };

    class IDecoder : public Utils::IBasicInterface
    {
    public:
        virtual int Decode(const byte *pIn, int size, int16 *pOutput, int maxOutputSize) = 0;

        virtual const ICodecInfo& DecoderInfo() const = 0;

        // helpers for vector and BidirBuff
        // ... 
    };

    class ICodec : public IEncoder, public IDecoder
    {
    public:
        // nothing 
    };

    // --------------------------------------------------------

    class ICodecInfo : public Utils::IBasicInterface
    {

    protected:

        // ICodecInfo can't be destoyed outside
        virtual ~ICodecInfo()
        {
        }

    public:

        virtual const std::string& Name() const = 0;
        virtual iRtp::RtpPayload PayloadType() const = 0;

        virtual IEncoder* CreateEncoder(const std::string &params) const = 0;
        virtual IDecoder* CreateDecoder(const std::string &params) const = 0;
        virtual ICodec*   CreateCodec(const std::string &params) const = 0;
    };

    // ICodecInfoImpl destoyable ICodecInfo version 
    class ICodecInfoImpl : public ICodecInfo
    {
    };

    // --------------------------------------------------------

    class CodecMng
    {
    public:

        static const ICodecInfo* Find(const std::string &name, bool throwOnNull);
        static const ICodecInfo* Find(iRtp::RtpPayload payload, bool throwOnNull);

		static iRtp::RtpPayloadList GetPayloadList();

        // list codecs, const for codec names
        // ... 
    };


    
}  // namespace SndMix

#endif
