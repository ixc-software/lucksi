#ifndef __ITDMWRITEWRAPPER__
#define __ITDMWRITEWRAPPER__

#include "Platform/PlatformTypes.h"
#include "Utils/IBasicInterface.h"
#include "Utils/BidirBuffer.h"


namespace TdmMng
{
    using Platform::byte;
    using Platform::word;
	
	class ITdmWriteWrapper : public Utils::IBasicInterface
	{
	public:

        void Write(byte timeSlot, word dstOffset, Utils::BidirBuffer& srcBuff, int srcCount)
        {
            Write(timeSlot, dstOffset, srcBuff.Front(), srcCount);
        }

        virtual void Write(byte timeSlot, word dstOffset, Platform::byte *pSrc, int srcCount) = 0;
        virtual int BlockSize() const = 0;
        virtual Platform::word* GetFrame(int frameNum, int offset) = 0;

	};
	
	
}  // namespace TdmMng

#endif

