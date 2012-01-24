#ifndef __TDMWRITEWRAPPEREMUL__
#define __TDMWRITEWRAPPEREMUL__

#include "ITdmWriteWrapper.h"

namespace TdmMng
{
		
    // emulation for TdmWriteWrapper
    class TdmWriteWrapperEmul : public ITdmWriteWrapper
    {        
        int m_capacity;
        std::vector<byte>& m_dataStore;        
        	
    // ITdmWriteWrapper impl
	public:        	
	
        // читает count байт из буфера
        void Write(byte timeSlot, word dstOffset, Platform::byte *pSrc, int srcCount)        
        {
            for(int i = 0; i < srcCount; ++i)
            {
                m_dataStore.push_back(pSrc[i]);
            }
        }

        int BlockSize() const
        {
            return m_capacity;
        }

        Platform::word* GetFrame(int frameNum, int offset)
        {
            ESS_UNIMPLEMENTED;
            return 0;
        }

		
    public:

        TdmWriteWrapperEmul(int capacity, std::vector<byte>& dataStore)
            : m_capacity(capacity),
            m_dataStore(dataStore)            
        {}

    };
	
	
}  // namespace TdmMng

#endif
