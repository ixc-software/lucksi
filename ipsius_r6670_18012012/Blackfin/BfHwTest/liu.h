#ifndef __DS2155_LIU__
#define __DS2155_LIU__

#include "macros.h"

	struct PriStatus
	{

		int  rxSignalLevel;				// уровень входного Е1 сигнала (0-15)
		bool rxLOS;						// авария "нет сигнала линии"
		bool rxAIS;						// авария "приходят все еденицы"
		bool rxLOF;						// авария "нет синхронизации Fas"
		bool rxLOMF;					// авария "нет сверхцикловой синхронизации MFas"
		bool rxLSS;						// авария "нет синхронизации по ПСП"
	};	
        
	extern PriStatus m_status;	

    bool LiuInitT1();		
    bool LiuInit();
	byte UpdateBitsState();    	   	
	void LoadStatus();	
	void PriStatShow();	
	void T1FrameFill();

#endif







