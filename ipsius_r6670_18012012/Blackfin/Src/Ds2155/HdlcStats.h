#ifndef __HDLCSTATS__
#define __HDLCSTATS__

#include "SafeBiProto/ISerialazable.h"

namespace Ds2155
{
    
	// ошибоки HDLC контроллера	
	struct ErrorFlags
	{

        // sync with iCmp::BfLiuInfo - !!
		bool rxBadPackFlag;				// флаг - принят битый пакет(CRC)
		bool rxIgnoredPackFlag;			// флаг - принят игнорированный пакет
		bool txOverRunedFlag;			// флаг - битый отправленный пакет
		bool txFiFoFool;				// флаг - очередь пакетов на передачу полна		
		bool rxFiFoFool;				// флаг - очередь пакетов на прием полна				
		bool txAllocErrorFlag;			// флаг - отказ Memory Pool'а для передатчика
		bool rxAllocErrorFlag;			// флаг - отказ Memory Pool'а для приемника

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
	
	// статистика HDLC контроллера		
    struct HdlcStatistic : public SBProto::ISerialazable
	{

		int rxBadPackCount;				// счетчик принятых битых пакетов
		int rxIgnoredPackCount;			// счетчик принятых пакетов размером больще допустимого
		int txOverRuned;				// счетчик битых отправленных пакетов
		int packSended;					// счетчик успешно отправленных пакетов
		int packRecieved;				// счетчик успешно принятых пакетов
		int txAllocErrorsCount;			// счетчик отказов Memory Pool'а	для передатчика
		int rxAllocErrorsCount;			// счетчик отказов Memory Pool'а	для приемника
		int txFifoFullCount;			// счетчик отказов SendPack, очередь полна
		int rxFifoFullCount;			// счетчик отказов RecvPack, очередь полна		

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
