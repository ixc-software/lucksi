#ifndef __IMIXPOINT__
#define __IMIXPOINT__

#include "Utils/SafeRef.h"
#include "Utils/IBasicInterface.h"

#include "MixUtils.h"
#include "CyclicBuff.h"

namespace SndMix
{
    using Utils::SafeRef;
    using Platform::int16;
 
    class IMixPointOwner;


    struct MixPointState
    {
        std::string Name;  
        CyclicBuffState ToMixer, FromMixer;

        MixPointState(const std::string &name, 
            const CyclicBuffState &toMixer, const CyclicBuffState &fromMixer) : 
          Name(name), ToMixer(toMixer), FromMixer(fromMixer)
        {
        }

        std::string ToString() const
        {
            std::ostringstream oss;

            oss << Name   << " / ToMixer "   << ToMixer.ToString() 
                          << " / FromMixer " << FromMixer.ToString();

            return oss.str();
        }

        static std::string ToString(const std::vector<MixPointState> &info)
        {
            std::ostringstream oss;
            oss << "{";

            for(int i = 0; i < info.size(); ++i)
            {
                oss << info.at(i).ToString();
                if (i != info.size() - 1) oss << "; ";
            }

            oss << "}";

            return oss.str();
        }
    };

    // ------------------------------------------------------------

    class IMixPoint : public Utils::IBasicInterface
    {
    public:

        virtual int Handle() const = 0;
        virtual PointMode GetMode() const = 0;
        virtual MixPointState PointState() const = 0;

        virtual CyclicBuff& ReadBuff() = 0;
        virtual CyclicBuff& WriteBuff() = 0;
    };

    // ------------------------------------------------------------

    class IMixPointOwner : public Utils::IBasicInterface
    {
    public:
        virtual void Bind(SafeRef<IMixPoint> point) = 0;
		virtual int  CreateIndex() = 0;
        virtual void Unbind(const IMixPoint &point) = 0;
    };

    
}  // namespace SndMix

#endif
