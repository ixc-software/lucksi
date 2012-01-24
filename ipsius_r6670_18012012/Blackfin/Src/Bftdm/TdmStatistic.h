#ifndef __TDMSTATISTIC__
#define __TDMSTATISTIC__

#include "Platform/Platform.h"

#include "SafeBiProto/ISerialazable.h"

namespace BfTdm
{
    using namespace Platform;   
    
	// ���������� Tdm		
    struct TdmStatistic : public SBProto::ISerialazable
	{

		int usedBlocks;					// ������� ���������� ������ � ������������
		int maxUsedBlocks;				// ������������ ���������� ������ ������ � ������������		
		int collisionCount;				// ������� �������� �� UserBlock`��				
		
		TdmStatistic()
		{
			Reset();
		}
			
		void Reset()
		{				
			usedBlocks = 0;
			maxUsedBlocks = 0;
			collisionCount = 0;
		}
		
		void CheckMaxUsedBlocks()
		{
	    	if (usedBlocks > maxUsedBlocks) maxUsedBlocks = usedBlocks;			
		}
	
		std::string ToString(bool withLf = true) const
		{		
            std::string lf = withLf ? "\n" : "  ";

			std::ostringstream resStream;

			resStream << "usedBlocks = " << usedBlocks << lf;
			resStream << "maxUsedBlocks = " << maxUsedBlocks << lf;		
			resStream << "collisionCount = " << collisionCount << lf;

			return resStream.str();
		}

		template<class Data, class TStream>
			static void Serialize(Data &data, TStream &s)
        {
            s
                << data.usedBlocks
                << data.maxUsedBlocks
                << data.collisionCount;
        }

	};	
    
    
}  // namespace BfTdm

#endif
