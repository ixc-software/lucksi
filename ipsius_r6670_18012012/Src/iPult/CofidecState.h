#ifndef __COFIDECSTATE__
#define __COFIDECSTATE__

#include "Utils/FullEnumResolver.h"


namespace iPult
{

    class CsEnum : public QObject
    {
        Q_OBJECT;

    public:

        enum LoopMode
        {
            lmLoopOff,
            lmAConverter,
            lmA64K,
            lmAInterface,
            lmAACE,
        };

        enum CsMode
        {
            mOff,
            mTdmToHandset,
            mTdmToLoud,
            mTdmLoudRing,
            mTdmCrossToHandset,
            mTdmCrossToLoud,
        };

        Q_ENUMS(LoopMode CsMode);

    };

    // -----------------------------------------------

    // Warning! This state affected CMP version 
    struct CofidecState
    {

        CsEnum::CsMode Mode;
        std::string EchoMode;  // as specail ini file 

        // in percent 0 .. 100 
        int HandsetMicLevel;
        int HandsetSpeakerLevel;
        int HandsfreeMicLevel;
        int HandsfreeSpeakerLevel;

        // debug
        CsEnum::LoopMode LoopMode;
                
        CofidecState() 
        {
            Mode = CsEnum::mOff;
            EchoMode.clear();

            HandsetMicLevel = 0;
            HandsetSpeakerLevel = 0;
            HandsfreeMicLevel = 0;
            HandsfreeSpeakerLevel = 0;            

            LoopMode = CsEnum::lmLoopOff;
        }

        template<class Data, class TStream>
        static void Serialize(Data &data, TStream &s)
        {
            s 
                << data.Mode
                << data.EchoMode
                << data.HandsetMicLevel
                << data.HandsetSpeakerLevel
                << data.HandsfreeMicLevel
                << data.HandsfreeSpeakerLevel                
                << data.LoopMode;
        }

        std::string ToString(const std::string &sep = "; ") const 
        {
            std::ostringstream oss;

            oss << "Mode: "                     << Utils::EnumResolve(Mode) << sep
                << (EchoMode.size() ? "+EchoMode" + sep : "") 
                << "HandsetMicLevel: "          << HandsetMicLevel          << sep
                << "HandsetSpeakerLevel: "      << HandsetSpeakerLevel      << sep
                << "HandsfreeMicLevel: "        << HandsfreeMicLevel        << sep
                << "HandsfreeSpeakerLevel: "    << HandsfreeSpeakerLevel    << sep
                << "LoopMode: "                 << Utils::EnumResolve(LoopMode);

            return oss.str();
        }

        bool IsLoudOutput() const
        {
            return 
                Mode == CsEnum::mTdmToLoud ||
                Mode == CsEnum::mTdmCrossToLoud ||
                Mode == CsEnum::mTdmLoudRing;
        }

        bool IsHsOutput() const
        {
            return Mode != CsEnum::mOff && !IsLoudOutput();
        }

                        
    };    
    
}  // namespace iPult

#endif