#ifndef __DS2155_LIU__
#define __DS2155_LIU__

#include "macros.h"

	struct PriStatus
	{

		int  rxSignalLevel;				// ������� �������� �1 ������� (0-15)
		bool rxLOS;						// ������ "��� ������� �����"
		bool rxAIS;						// ������ "�������� ��� �������"
		bool rxLOF;						// ������ "��� ������������� Fas"
		bool rxLOMF;					// ������ "��� ������������� ������������� MFas"
		bool rxLSS;						// ������ "��� ������������� �� ���"
	};	
        
	extern PriStatus m_status;	

    bool LiuInitT1();		
    bool LiuInit();
	byte UpdateBitsState();    	   	
	void LoadStatus();	
	void PriStatShow();	
	void T1FrameFill();

#endif







