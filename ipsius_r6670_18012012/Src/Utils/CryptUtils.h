#ifndef __CRYPTUTILS__
#define __CRYPTUTILS__

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include "cryptopp/base64.h"
#include "cryptopp/md5.h"

namespace Utils
{
    
    template<class T>
    struct VerifyByteContainer
    {
        enum
        {
            CTIsString     = boost::is_base_of<std::string, T>::value,
            CTIsVectorByte = boost::is_base_of<std::vector<Platform::byte>, T>::value,
            CTIsVectorChar = boost::is_base_of<std::vector<char>, T>::value,
        };

        BOOST_STATIC_ASSERT(CTIsString || CTIsVectorByte || CTIsVectorChar);
    };

    // -----------------------------------------------------------

    class Base64
    {

    public:

        template<class TOut>
        static void Encode(const void *pIn, int inputSize, TOut &out)
        {
            { VerifyByteContainer<TOut> b; }

            ESS_ASSERT(inputSize > 0);

            CryptoPP::Base64Encoder e(0, false);

            e.Put((const byte*)pIn, inputSize);
            e.MessageEnd();

            int len = e.MaxRetrievable();
            out.resize(len);
            ESS_ASSERT( e.Get( (byte*)(&out[0]), out.size() ) == len );
        }

        template<class TIn, class TOut>
        static void Encode(const TIn &in, TOut &out)
        {
            { VerifyByteContainer<TIn> a; VerifyByteContainer<TOut> b; }

            ESS_ASSERT(in.size() > 0);
            Encode(&in[0], in.size(), out);
        }

        static void EncodeTest()
        {
            std::string in = "qwerty";
            std::string out;

            Encode(in, out);
            TUT_ASSERT(out == "cXdlcnR5");

            std::vector<Platform::byte> outBytes;
            Encode(in, out);
        }
    };

    // ------------------------------------------------------------

    class MD5
    {
    public:

        template<class TOut>
        static void Digest(const void *pIn, int inputSize, TOut &out)
        {
            { VerifyByteContainer<TOut> b; }

            ESS_ASSERT(inputSize > 0);

            out.resize( CryptoPP::Weak::MD5::DIGESTSIZE );

            CryptoPP::Weak::MD5 md5;
            md5.CalculateDigest((byte*)(&out[0]), (const byte*)pIn, inputSize);
        }

        template<class TIn, class TOut>
        static void Digest(const TIn &in, TOut &out)
        {
            { VerifyByteContainer<TIn> a; VerifyByteContainer<TOut> b; }

            ESS_ASSERT(in.size() > 0);

            Digest(&in[0], in.size(), out);
        }

        static void DigestTest()
        {
            std::string in = "Nobody inspects the spammish repetition";
            std::string out;

            Digest(in, out);

            TUT_ASSERT(out == "\xbb\x64\x9c\x83\xdd\x1e\xa5\xc9\xd9\xde\xc9\xa1\x8d\xf0\xff\xe9");
        }

    };

    
}  // namespace Utils

#endif
