#include "stdafx.h"
#include "Utils/StringParser.h"
#include "Utils/StringUtils.h"
#include "Utils/MetaConfig.h"
#include "Utils/AtomicTypes.h"
#include "iCodec.h"

namespace 
{
    bool isEqualNoCase(const std::string& left, const std::string& right)
    {
        /*
        return ( (left.size() == right.size()) &&
            (strnicmp(left.data(), right.data(), left.size()) == 0) );
            */

        // int tolower(int ch);

        if (left.size() != right.size()) return false;

        for (size_t i = 0; i < left.size(); ++i)
        {
            if (tolower(left.at(i)) != tolower(right.at(i))) return false;
        }

        return true;
    }

    // -------------------------------------------------------------------

    class StaticCodecs : boost::noncopyable
    {
		static boost::scoped_ptr<StaticCodecs> GImpl;
    public:
		struct Constructor
		{
			Constructor()
			{
				ESS_ASSERT(GImpl == 0);
				GImpl.reset(new StaticCodecs);
			}
		};

        static const StaticCodecs &instance()
		{
			return *GImpl;
		}

        const iMedia::Codec *getStaticCodec(const std::string &name) const
        {
            for (ListCodecsPtr::const_iterator i = audioCodecs().begin();
                i!= audioCodecs().end(); ++i)
            {
                if(isEqualNoCase(name, (*i)->getName()))
                    return *i;
            }
            for (ListCodecsPtr::const_iterator i = videoCodecs().begin();
                i!= videoCodecs().end(); ++i)
            {
                if(isEqualNoCase(name, (*i)->getName()))
                    return *i;
            }
            return 0;
        }

        const iMedia::Codec *getStaticCodec(int payload) const
        {
            for (ListCodecsPtr::const_iterator i = audioCodecs().begin();
                i!= audioCodecs().end(); ++i)
            {
                if(payload == (*i)->getPayloadType())
                    return *i;
            }
            for (ListCodecsPtr::const_iterator i = videoCodecs().begin();
                i!= videoCodecs().end(); ++i)
            {
                if(payload == (*i)->getPayloadType())
                    return *i;
            }
            return 0;
        }

        StaticCodecs()
        {
            using namespace iMedia;
            m_audioCodecs.push_back(&Codec::ULaw_8000);
            m_audioCodecs.push_back(&Codec::ALaw_8000);
            m_audioCodecs.push_back(&Codec::GSM_8000);
            m_audioCodecs.push_back(&Codec::G723_8000);
            m_audioCodecs.push_back(&Codec::DVI4_8000);
            m_audioCodecs.push_back(&Codec::DVI4_16000);
            m_audioCodecs.push_back(&Codec::LPC_8000);
            m_audioCodecs.push_back(&Codec::G722_8000);
            m_audioCodecs.push_back(&Codec::L16_2_44100);
            m_audioCodecs.push_back(&Codec::L16_1_44100);
            m_audioCodecs.push_back(&Codec::QCELP_8000);
            m_audioCodecs.push_back(&Codec::CN_8000);
            m_audioCodecs.push_back(&Codec::MPA_90000);
            m_audioCodecs.push_back(&Codec::G728_8000);
            m_audioCodecs.push_back(&Codec::DVI4_11025);
            m_audioCodecs.push_back(&Codec::DVI4_22050);
            m_audioCodecs.push_back(&Codec::G729_8000);
            m_audioCodecs.push_back(&Codec::TelephoneEvent_8000);
            m_audioCodecs.push_back(&Codec::FrfDialedEvent_8000);

            // Video or audio/video codecs
            m_videoCodecs.push_back(&Codec::CelB_90000);
            m_videoCodecs.push_back(&Codec::JPEG_90000);
            m_videoCodecs.push_back(&Codec::NV_90000);
            m_videoCodecs.push_back(&Codec::H261_90000);
            m_videoCodecs.push_back(&Codec::MPV_90000);
            m_videoCodecs.push_back(&Codec::MP2T_90000);
            m_videoCodecs.push_back(&Codec::H263__90000);
        }
       
    private:
        typedef std::vector<const iMedia::Codec*> ListCodecsPtr;

        const ListCodecsPtr &audioCodecs() const 
        {
            return m_audioCodecs; 
        }

        const ListCodecsPtr &videoCodecs() const 
        {
            return m_videoCodecs;
        }
    private:
        ListCodecsPtr m_audioCodecs;
        ListCodecsPtr m_videoCodecs;
    };
	boost::scoped_ptr<StaticCodecs> StaticCodecs::GImpl;
};

namespace iMedia
{
	// -------------------------------------------------------------------
	// static audio codecs

	const Codec Codec::ULaw_8000("PCMU", 0, 8000);
	const Codec Codec::ALaw_8000("PCMA", 8, 8000);
	const Codec Codec::GSM_8000("GSM", 3, 8000);
	const Codec Codec::G723_8000("G723", 4,8000);
	const Codec Codec::DVI4_8000("DVI4", 5, 8000);
	const Codec Codec::DVI4_16000("DVI4", 6, 16000);
	const Codec Codec::LPC_8000("LPC", 7, 8000);
	const Codec Codec::G722_8000("G722", 9, 8000);
	const Codec Codec::L16_2_44100("L16-2", 10, 44100);
	const Codec Codec::L16_1_44100("L16-1", 11, 44100);
	const Codec Codec::QCELP_8000("QCELP", 12, 8000);
	const Codec Codec::CN_8000("CN", 13, 8000);
	const Codec Codec::MPA_90000("MPA", 14, 90000);
	const Codec Codec::G728_8000("G728", 15, 8000);
	const Codec Codec::DVI4_11025("DVI4", 16, 11025);
	const Codec Codec::DVI4_22050("DVI4", 17, 22050);
	const Codec Codec::G729_8000("G729", 18, 8000);
	const Codec Codec::TelephoneEvent_8000("telephone-event", 101, 8000);
	const Codec Codec::FrfDialedEvent_8000("frf-dialed-event", 102, 8000);

	// Video or audio/video codecs
	const Codec Codec::CelB_90000("CelB", 25, 90000);
	const Codec Codec::JPEG_90000("JPEG", 26, 90000);
	const Codec Codec::NV_90000("nv", 28, 90000);
	const Codec Codec::H261_90000("H261", 31, 90000);
	const Codec Codec::MPV_90000("MPV", 32, 90000);
	const Codec Codec::MP2T_90000("MP2T", 33, 90000);
	const Codec Codec::H263__90000("H263", 34, 90000);
	StaticCodecs::Constructor staticCodecsInit;


    Codec::Codec(std::string name, 
        int payloadType, 
        int rate, 
        std::string parameters, 
        std::string encodingParameters) :
        m_name(name),
        m_payload(payloadType),
        m_rate(rate),
        m_parameters(parameters),
        m_encodingParameters(encodingParameters)
    {}

    // -------------------------------------------------------------------

    Codec::Codec(std::string name, int payloadType, int rate) :
        m_name(name),
        m_payload(payloadType),
        m_rate(rate)
    {}

    // -------------------------------------------------------------------

    std::string Codec::ToString(const CodecsList &list, const std::string &separator)
    {
        std::string result;
        for (CodecsList::const_iterator i = list.begin(); i!=list.end(); ++i)
        {
            if(result.size())
                result += separator;
            result += i->getName();
        }
        return result;
    }

    // -------------------------------------------------------------------

    CodecsList Codec::Parse(const std::string &str, 
        const std::string &separator)
    {
        Utils::ManagedList<std::string> separatedList;
        Utils::StringParser(str, separator, separatedList, true); 
        CodecsList codecs;

        for (size_t i = 0; i < separatedList.Size(); ++i)
        {
            std::string name(Utils::TrimBlanks(*separatedList[i]));
            const Codec *codec = getStaticCodec(name);
            if (!codec)
            {
                std::ostringstream out;
                out << "Unknown codec: " << name;
                ESS_THROW_MSG(ParseError, out.str());
            }
            codecs.push_back(*codec);
        }
        return codecs;
    }

    // -------------------------------------------------------------------

    bool Codec::IsEqual(const Codec &codec) const
    {
        return m_name == codec.getName() && 
            m_payload == codec.getPayloadType() &&
            m_rate == codec.getRate();
    }

    // -------------------------------------------------------------------

    std::string Codec::getName() const
    {
        return m_name;
    }

    // -------------------------------------------------------------------

    int Codec::getRate() const
    {
        return m_rate;
    }

    // -------------------------------------------------------------------

    int Codec::getPayloadType() const
    {
        return m_payload;
    }

    // -------------------------------------------------------------------

    std::string Codec::getParameters() const
    {
        return m_parameters;
    }

    // -------------------------------------------------------------------

    std::string Codec::getEncodingParameters() const
    {
        return m_encodingParameters;
    }

    // -------------------------------------------------------------------

    const Codec *Codec::getStaticCodec(const std::string &name)
    {
        return StaticCodecs::instance().getStaticCodec(name);
    }

    // -------------------------------------------------------------------

    const Codec *Codec::getStaticCodec(int payload)
    {
        return StaticCodecs::instance().getStaticCodec(payload);
    }

    // -------------------------------------------------------------------

    bool operator==(const Codec& lhs, const Codec& rhs)
    {
        std::string defaultEncodingParameters("1");  // Default for audio streams (1-Channel)
        return 
            (isEqualNoCase(lhs.getName(), rhs.getName()) && 
            lhs.getRate() == rhs.getRate() && 
            (lhs.getEncodingParameters() == rhs.getEncodingParameters() ||
            (lhs.getEncodingParameters().empty() && rhs.getEncodingParameters() == defaultEncodingParameters) ||
            (lhs.getEncodingParameters() == defaultEncodingParameters && rhs.getEncodingParameters().empty())));
    }

    // -------------------------------------------------------------------

    inline bool operator!=(const Codec &arg1, const Codec &arg2)
    {
        return !operator==(arg1, arg2);
    }
};
