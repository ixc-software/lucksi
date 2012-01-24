#ifndef __HDLCSTATS__
#define __HDLCSTATS__

#include "SafeBiProto/ISerialazable.h"

namespace Ds2155
{
    
	// ������� HDLC �����������	
	struct ErrorFlags
	{

        // sync with iCmp::BfLiuInfo - !!
		bool rxBadPackFlag;				// ���� - ������ ����� �����(CRC)
		bool rxIgnoredPackFlag;			// ���� - ������ �������������� �����
		bool txOverRunedFlag;			// ���� - ����� ������������ �����
		bool txFiFoFool;				// ���� - ������� ������� �� �������� �����		
		bool rxFiFoFool;				// ���� - ������� ������� �� ����� �����				
		bool txAllocErrorFlag;			// ���� - ����� Memory Pool'� ��� �����������
		bool rxAllocErrorFlag;			// ���� - ����� Memory Pool'� ��� ���������

		// ---------------------------------------------------------------------    	    		
	
		ErrorFlags()
		{
			Clear();	
		}
				
		void Clear()
		{
			rxBadPackFlag = false;
			rxIgnoredPackFlag = false;
			txOverRunedFlag = false;
			txFiFoFool = false;
			rxFiFoFool = false;			
			txAllocErrorFlag = false;
			rxAllocErrorFlag = false;
		}	
		
		bool HasErrors() const
		{
			bool result = false;
		
			if (rxBadPackFlag) result = true;
			if (rxIgnoredPackFlag) result = true;
			if (txOverRunedFlag) result = true;
			if (txFiFoFool) result = true;			
			if (rxFiFoFool) result = true;						
			if (txAllocErrorFlag) result = true;
			if (rxAllocErrorFlag) result = true;
		
			return result;		
		}
	
		std::string ToString() const
		{
			std::string resStr="";
				
			if (rxBadPackFlag) resStr += "rxBadPackFlag ";
			if (rxIgnoredPackFlag) resStr += "rxIgnoredPackFlag ";
			if (txOverRunedFlag) resStr += "txOverRunedFlag ";
			if (txFiFoFool) resStr += "txFiFoFool ";			
			if (rxFiFoFool) resStr += "rxFiFoFool ";						
			if (txAllocErrorFlag) resStr += "txAllocErrorFlag ";		
			if (rxAllocErrorFlag) resStr += "rxAllocErrorFlag ";
			if (HasErrors()) resStr += "\n";
		
			return resStr;
		}
		
	};
	
	// ---------------------------------------------------------------------    	    		
	
	// ���������� HDLC �����������		
    struct HdlcStatistic : public SBProto::ISerialazable
	{

		int rxBadPackCount;				// ������� �������� ����� �������
		int rxIgnoredPackCount;			// ������� �������� ������� �������� ������ �����������
		int txOverRuned;				// ������� ����� ������������ �������
		int packSended;					// ������� ������� ������������ �������
		int packRecieved;				// ������� ������� �������� �������
		int txAllocErrorsCount;			// ������� ������� Memory Pool'�	��� �����������
		int rxAllocErrorsCount;			// ������� ������� Memory Pool'�	��� ���������
		int txFifoFullCount;			// ������� ������� SendPack, ������� �����
		int rxFifoFullCount;			// ������� ������� RecvPack, ������� �����		

		// ---------------------------------------------------------------------    	    				
		
		HdlcStatistic()
		{
			Reset();
		}
			
		void Reset()
		{				
			rxBadPackCount = 0;
			rxIgnoredPackCount = 0;
			txOverRuned = 0;
			packSended = 0;
			packRecieved = 0;
			txAllocErrorsCount = 0;
			rxAllocErrorsCount = 0;				
			rxFifoFullCount = 0;			
			txFifoFullCount = 0;
		}
	
		std::string ToString(bool withLf = true) const
		{		
            std::string lf = withLf ? "\n" : "  ";

			std::ostringstream resStream;

			resStream << "packRecieved = " << packRecieved << lf;
			resStream << "packSended = " << packSended << lf;		

			resStream << "rxBadPackCount = " << rxBadPackCount << lf;
			resStream << "rxIgnoredPackCount = " << rxIgnoredPackCount << lf;		
			resStream << "txOverRuned = " << txOverRuned << lf;				
			resStream << "txAllocErrorsCount = " << txAllocErrorsCount << lf;		
			resStream << "rxAllocErrorsCount = " << rxAllocErrorsCount << lf;		
			resStream << "txFiFoFoolCount = " << txFifoFullCount << lf;		
			resStream << "rxFiFoFoolCount = " << rxFifoFullCount << lf;											

			return resStream.str();
		}

		template<class Data, class TStream>
		static void Serialize(Data &data, TStream &s)
        {
            s 
                << data.rxBadPackCount
                << data.rxIgnoredPackCount
                << data.txOverRuned
                << data.packSended
                << data.packRecieved
                << data.txAllocErrorsCount
                << data.rxAllocErrorsCount
                << data.txFifoFullCount
                << data.rxFifoFullCount;
        }

	};	
			
        
}  // namespace Ds2155

#endif
