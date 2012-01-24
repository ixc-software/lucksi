#ifndef __ITDMSOURCE__
#define __ITDMSOURCE__

#include "Utils/IBasicInterface.h"
#include "Utils/BidirBuffer.h"

#include "TdmMng/ITdmWriteWrapper.h"

namespace TdmMng
{
	
    class ITdmSource : public Utils::IBasicInterface
    {
    public:
        virtual bool WriteToBlock(ITdmWriteWrapper &block) = 0;
    };

    class IRtpLikeTdmSource : public Utils::IBasicInterface
    {
    public:

        virtual Utils::BidirBuffer* DetachData() = 0;  // get buffer with ownership

    };


}  // namespace TdmMng


#endif

