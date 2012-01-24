#ifndef __TDMCODECS__
#define __TDMCODECS__

#include "iDSP/g711codec.h"
#include "EchoApp/LowTransformT.h"

namespace TdmMng
{
    
    using Platform::byte;
    using Platform::int16;

    class ACodec : public iDSP::ICodec
    {
        typedef EchoApp::LowTemplateTable<false, EchoApp::ALowBase> Encoder;
        typedef EchoApp::LowTemplateTable<true, EchoApp::ALowBase>  Decoder;

    // iDSP::ICodec impl
    private:

        byte Encode(int sample) const
        {
            return Encoder::Process(sample);
        }

        int Decode(byte val) const
        {
            return Decoder::Process(val);
        }

        void EncodeBlock(const int16 *pIn, byte *pOut, int count) const
        {
            Encoder::Process(pIn, pOut, count);
        }

        void DecodeBlock(const byte *pIn, Platform::int16 *pOut, int count) const
        {
            Decoder::Process(pIn, pOut, count);
        }

    public:

        ACodec()
        {
            Encoder::Init();
            Decoder::Init();
        }

    };

    // -------------------------------------------------------------

    class UCodec : public iDSP::ICodec
    {
        typedef EchoApp::LowTemplateTable<false, EchoApp::ULowBase> Encoder;
        typedef EchoApp::LowTemplateTable<true, EchoApp::ULowBase>  Decoder;

    // iDSP::ICodec impl
    private:

        byte Encode(int sample) const
        {
            return Encoder::Process(sample);
        }

        int Decode(byte val) const
        {
            return Decoder::Process(val);
        }

        void EncodeBlock(const int16 *pIn, byte *pOut, int count) const
        {
            Encoder::Process(pIn, pOut, count);
        }

        void DecodeBlock(const byte *pIn, Platform::int16 *pOut, int count) const
        {
            Decoder::Process(pIn, pOut, count);
        }

    public:

        UCodec()
        {
            Encoder::Init();
            Decoder::Init();
        }

    };

    // -------------------------------------------------------------

    struct TdmCodec
    {

        static iDSP::ICodec* CreateCodec(bool useAlaw)
        {        
            if (useAlaw) return new ACodec();
                         return new UCodec();
        }

    };
            
}  // namespace TdmMng


#endif
