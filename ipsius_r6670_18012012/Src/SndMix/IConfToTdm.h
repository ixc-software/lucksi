#ifndef __ICONFTOTDM__
#define __ICONFTOTDM__

#include "Utils/IBasicInterface.h"

#include "TdmMng/ISendDirection.h"
#include "TdmMng/ITdmSource.h"

namespace SndMix
{
    
    class IConfToTdm
    {
    public:

        virtual void GetDeviceInfo(const std::string &streamName, 
            /* out */ int &tdmBlockSize, bool &aLow) = 0;

        virtual void BindToConference(const std::string &streamName, int chNum,
            const Utils::SafeRef<TdmMng::ISendSimple> &send,
            const Utils::SafeRef<TdmMng::IRtpLikeTdmSource> &recv) = 0;

        virtual void UnbindFromConference(const std::string &streamName, int chNum,
            const Utils::SafeRef<TdmMng::ISendSimple> &send,
            const Utils::SafeRef<TdmMng::IRtpLikeTdmSource> &recv) = 0;

    };
    
}  // namespace SndMix

#endif
