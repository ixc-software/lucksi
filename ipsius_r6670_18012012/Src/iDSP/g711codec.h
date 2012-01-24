#ifndef __G711CODEC__
#define __G711CODEC__

#include "Platform/PlatformTypes.h"
#include "Utils/IBasicInterface.h"

namespace iDSP
{
    using Platform::byte;
    using Platform::int16;

    byte linear2alaw(int pcm_val);
    int alaw2linear(byte a_val);

    byte linear2ulaw(int pcm_val);
    int ulaw2linear(byte u_val);

    byte alaw2ulaw(byte aval);
    byte ulaw2alaw(byte uval);


    // -------------------------------------------

    enum
    {
        CALowMin = -32256,
        CALowMax = 32256,

        CULowMin = -32124,
        CULowMax = 32124,
    };

    // -------------------------------------------

    class ICodec : public Utils::IBasicInterface
    {
    public:
        virtual byte Encode(int sample) const = 0;
        virtual int Decode(byte val) const = 0;

        virtual void EncodeBlock(const int16 *pIn, byte *pOut, int count) const
        {
            while(count--)
            {
                *pOut++ = Encode(*pIn++);
            }
        }

        virtual void DecodeBlock(const byte *pIn, Platform::int16 *pOut, int count) const
        {
            while(count--)
            {
                *pOut++ = Decode(*pIn++);
            }
        }

    };

    class CodecAlow : public ICodec
    {
    // ICodec impl
    public:
        byte Encode(int sample) const { return linear2alaw(sample); }
        int Decode(byte val) const { return alaw2linear(val); }

    };

    class CodecUlow : public ICodec
    {
    // ICodec impl
    public:
        byte Encode(int sample) const { return linear2ulaw(sample); }
        int Decode(byte val) const { return ulaw2linear(val); }
    };

}  // namespace iMedia

#endif
