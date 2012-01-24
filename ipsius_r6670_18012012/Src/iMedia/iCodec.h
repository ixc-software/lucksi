#ifndef _SDP_CODEC_H_
#define _SDP_CODEC_H_

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"

namespace Utils {   class MetaConfig; };

namespace iMedia
{
    class Codec;
    
    typedef std::vector<Codec> CodecsList;

    class Codec 
    {
    public:
        ESS_TYPEDEF(ParseError);

        Codec(std::string name, 
            int payloadType, 
            int rate, 
            std::string parameters, 
            std::string encodingParameters);

        Codec(std::string name, int payloadType, int rate=8000);

        static std::string ToString(const CodecsList &list, const std::string &separator);
        static CodecsList Parse(const std::string &str, 
            const std::string &separator); // can throw

        bool IsEqual(const Codec &codec) const;
        std::string getName() const;
        int getRate() const;
        int getPayloadType() const;
        std::string getParameters() const;
        std::string getEncodingParameters() const;

        // static audio codecs
			static const Codec ULaw_8000;           // name = PCMU, payload = 0, rate = 8000
			static const Codec ALaw_8000;           // name = PCMA, payload = 8, rate = 8000
			static const Codec GSM_8000;            // name = GSM, payload = 3, rate = 8000
			static const Codec G723_8000;           // name = G723, payload = 4,8000
			static const Codec DVI4_8000;           // name = DVI4, payload = 5, rate = 8000
			static const Codec DVI4_16000;          // name = DVI4, payload = 6, rate = 16000
			static const Codec LPC_8000;            // name = LPC, payload = 7, rate = 8000
			static const Codec G722_8000;           // name = G722, payload = 9, rate = 8000
			static const Codec L16_2_44100;         // name = L16-2, payload = 10, rate = 44100
			static const Codec L16_1_44100;         // name = L16-1, payload = 11, rate = 44100
			static const Codec QCELP_8000;          // name = QCELP, payload = 12, rate = 8000
			static const Codec CN_8000;             // name = CN, payload = 13, rate = 8000
			static const Codec MPA_90000;           // name = MPA, payload = 14, rate = 90000
			static const Codec G728_8000;           // name = G728, payload = 15, rate = 8000
			static const Codec DVI4_11025;          // name = DVI4, payload = 16, rate = 11025
			static const Codec DVI4_22050;          // name = DVI4, payload = 17, rate = 22050
			static const Codec G729_8000;           // name = G729, payload = 18, rate = 8000
			static const Codec TelephoneEvent_8000; // name = telephone-event, payload = 101, rate = 8000
			static const Codec FrfDialedEvent_8000; // name = frf-dialed-event, payload = 102, rate = 8000
			
			// Video or audio/video codecs
			static const Codec CelB_90000;  // name = CelB, payload = 25, rate = 90000
			static const Codec JPEG_90000;  // name = JPEG, payload = 26, rate = 90000
			static const Codec NV_90000;    // name = nv, payload = 28, rate = 90000
			static const Codec H261_90000;  // name = H261, payload = 31, rate = 90000
			static const Codec MPV_90000;   // name = MPV, payload = 32, rate = 90000
			static const Codec MP2T_90000;  // name = MP2T, payload = 33, rate = 90000
			static const Codec H263__90000; // name = H263, payload = 34, rate = 90000

        static const Codec *getStaticCodec(const std::string &name);
        static const Codec *getStaticCodec(int payload);

	private:
        std::string m_name; 
        int m_payload;
        int  m_rate;
        std::string m_parameters;
        std::string m_encodingParameters;
    };

    bool operator==(const Codec &arg1, const Codec &arg2);
    bool operator!=(const Codec &arg1, const Codec &arg2);
};

#endif
