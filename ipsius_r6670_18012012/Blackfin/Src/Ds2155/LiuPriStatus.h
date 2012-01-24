#ifndef __LIUPRISTATUS__
#define __LIUPRISTATUS__

#include "SafeBiProto/ISerialazable.h"

namespace Ds2155
{
    
	// статистика LIU		
    struct PriStatus : public SBProto::ISerialazable
	{
        bool OperationModeE1;		    // 1 - E1, 0 - T1		    
		
		int rxSignalLevel;				// уровень входного Е1 сигнала (0-15)
		bool rxLOS;						// авария "нет сигнала линии"
		bool rxAIS;						// авария "приходят все еденицы"		
		bool rxLOF;						// авария "нет синхронизации Fas"
		bool rxLOMF;					// авария "нет сверхцикловой синхронизации MFas"		
		bool rxLSS;						// авария "нет синхронизации по ПСП"
        bool rxYellowAlarm;				// авария Yellow Alarm (только для T1)	
        
		
		PriStatus() : OperationModeE1(true)
		{		    
            Reset();   
		}

        void Reset()
        {
            rxSignalLevel = -1;
            rxLOS = false;
            rxAIS = false;
            rxLOF = false;
            rxLOMF = false;
			rxLSS = false;
			rxYellowAlarm = false; 
        }
						
		std::string ToString(bool withLf = true) const
		{				
            std::string lf = withLf ? "\n" : "  ";

			std::ostringstream resStream;

			if(OperationModeE1)
			{
						
    			resStream << "RxLOS = " << BoolToString(rxLOS) << lf;
    			resStream << "RxAIS = " << BoolToString(rxAIS) << lf;
    			resStream << "RxLOF = " << BoolToString(rxLOF) << lf;			
    			resStream << "RxLOMF = " << BoolToString(rxLOMF) << lf;						

			}
			else
			{
    			resStream << "RxRed Alarm = " << BoolToString(rxLOS) << lf;
    			resStream << "RxYellow Alarm = " << BoolToString(rxYellowAlarm) << lf;
    			resStream << "RxBlue Alarm = " << BoolToString(rxAIS) << lf;    						    
    			resStream << "RxLostSync = " << BoolToString(rxLOF) << lf;
			}

    		resStream << "RxLSS = " << BoolToString(rxLSS) << lf;															
    		resStream << "PriRecieveLevel = -" << (rxSignalLevel * 2.5) << " dB" << lf;						
	
			return resStream.str();
		}

		template<class Data, class TStream>
			static void Serialize(Data &data, TStream &s)
        {
            s
                << data.rxSignalLevel
                << data.rxLOS
                << data.rxAIS
                << data.rxLOF
                << data.rxLOMF
				<< data.rxLSS 
				<< data.rxYellowAlarm;
        }
		
	private:
		
		std::string BoolToString(bool val) const
		{
			return val ? "1" : "0";
		}
		
	};	
    
    
}  // namespace Ds2155


#endif
