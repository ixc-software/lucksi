#ifndef __ITDMGENERATOR__
#define __ITDMGENERATOR__

#include "Utils/IBasicInterface.h"
#include "Utils/BidirBuffer.h"

#include "ITdmWriteWrapper.h"

namespace TdmMng
{
	
	class ITdmGenerator : public Utils::IBasicInterface
    {
	public:
		
		virtual void WriteTo(Utils::BidirBuffer &buff, int size) = 0;
		virtual void WriteTo(ITdmWriteWrapper &buff, int chNum) = 0;
        virtual bool Completed() = 0;
    };

	
}  // namespace TdmMng

#endif
