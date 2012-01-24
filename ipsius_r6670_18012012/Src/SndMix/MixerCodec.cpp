#include "stdafx.h"

#include "Utils/IntToString.h"
#include "Utils/ManagedList.h"
#include "Utils/GlobalMutex.h"
#include "TdmMng/TdmCodecs.h"

#include "MixerCodec.h"

// -------------------------------------------------------

namespace
{
    using namespace SndMix;

    // --------------------------------------------------

    struct G711a
    {
        static const iRtp::RtpPayload Payload = iRtp::PCMA;
        static const char* Name() { return "G711a"; }
        typedef TdmMng::ACodec Codec;
    };

    struct G711u
    {
        static const iRtp::RtpPayload Payload = iRtp::PCMU;
        static const char* Name() { return "G711u"; }
        typedef TdmMng::UCodec Codec;
    };

    template<class T>
    class G711 : public SndMix::ICodecInfoImpl
    {
        const std::string m_name;
        boost::scoped_ptr<const iDSP::ICodec> m_codec;

        class CodecWrapper : public ICodec
        {
            const ICodecInfo &m_info;
            const iDSP::ICodec &m_codec;

        // ICodec impl
        private:

            int Encode(const int16 *pData, int size, byte *pOutput, int maxOutputSize)
            {
                ESS_ASSERT(size > 0);
                ESS_ASSERT(maxOutputSize >= size);

                m_codec.EncodeBlock(pData, pOutput, size);

                return size;
            }

            int TypicalBlockSizeForEncode() const
            {
                return 160;
            }

            const ICodecInfo& EncoderInfo() const
            {
                return m_info;
            }


            int Decode(const byte *pIn, int size, int16 *pOutput, int maxOutputSize)
            {
                ESS_ASSERT(size > 0);
                ESS_ASSERT(maxOutputSize >= size);

                m_codec.DecodeBlock(pIn, pOutput, size);

                return size;
            }

            const ICodecInfo& DecoderInfo() const
            {
                return m_info;
            }


        public:

            CodecWrapper(const ICodecInfo &info, const iDSP::ICodec &codec) : 
              m_info(info),
              m_codec(codec)
            {
            }
        };

    // SndMix::ICodecInfo impl
    private:

        const std::string& Name() const { return m_name; }
        iRtp::RtpPayload PayloadType() const { return T::Payload; }

        IEncoder* CreateEncoder(const std::string &params) const
        {
            return CreateCodec(params);
        }

        IDecoder* CreateDecoder(const std::string &params) const
        {
            return CreateCodec(params);
        }

        ICodec*   CreateCodec(const std::string &params) const
        {
            if (!params.empty()) ESS_THROW(Error::G711DontHaveParams);
            return new CodecWrapper(*this, *m_codec);
        }

    public:

        G711() : m_name( T::Name() )
        {            
            typedef typename T::Codec Codec;
            m_codec.reset( new Codec() );
        }

    };

    // --------------------------------------------------

    class CodecList
    {
        Utils::ManagedList<const ICodecInfoImpl> m_list;

    public:

        CodecList()
        {
            m_list.Add( new G711<G711a>() ); 
            m_list.Add( new G711<G711u>() ); 
        }

		iRtp::RtpPayloadList GetPayloadList() const
		{
			iRtp::RtpPayloadList res;

			for(int i = 0; i < m_list.Size(); ++i)
			{
				res.push_back(m_list[i]->PayloadType());
			}

			return res;
		}


        const ICodecInfo* Find(const std::string &name) const 
        {
            for(int i = 0; i < m_list.Size(); ++i)
            {
                if (m_list[i]->Name() == name) 
                {
                    return m_list[i];
                }
            }

            return 0;
        }

        const ICodecInfo* Find(iRtp::RtpPayload payload) const 
        {
            for(int i = 0; i < m_list.Size(); ++i)
            {
                if (m_list[i]->PayloadType() == payload) 
                {
                    return m_list[i];
                }
            }

            return 0;
        }

    };

    // --------------------------------------------------

    CodecList& List()
    {
        Utils::GlobalMutexLocker locker;
        static CodecList list;
        return list;
    }


}  // namespace


// -------------------------------------------------------

namespace SndMix
{

    static const ICodecInfo* ReturnPtr(const ICodecInfo *p, bool throwOnNull,
        const std::string &info)
    {
        if ((p == 0) && throwOnNull)
        {
            ESS_THROW_MSG(Error::CantFindCodec, info);            
        }

        return p;
    }
           
    const ICodecInfo* CodecMng::Find(const std::string &name, bool throwOnNull)
    {
        return ReturnPtr(List().Find(name), throwOnNull, "Name " + name);
    }

    const ICodecInfo* CodecMng::Find(iRtp::RtpPayload payload, bool throwOnNull)
    {
        std::string info = "Payload " + Utils::IntToString(payload);
        return ReturnPtr(List().Find(payload), throwOnNull, info);
    }

	iRtp::RtpPayloadList  CodecMng::GetPayloadList()
	{
        return List().GetPayloadList();
	}

}  // namespace SndMix
