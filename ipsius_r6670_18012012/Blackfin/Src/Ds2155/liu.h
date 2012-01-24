#ifndef __DS2155_LIU__
#define __DS2155_LIU__

#include "Platform\platformtypes.h"
#include "ds2155_reg.h"
#include "LiuProfile.h"
#include "LiuPriStatus.h"

namespace Ds2155
{
    using namespace Platform;

	class IMembersToBoard;
	
	// ---------------------------------------------------------------------    	    		
					
    /*
        Line interface unit
        .....
    */
        
	class Liu
	{
	public:
	    
		Liu(IMembersToBoard &board, const LiuProfile &src);
		    		
		void ChangeProfileType(const LiuProfile &src);
		const PriStatus &PeekStatus();		
        std::string &ToString(std::string &str) const;

        bool Init();
    	byte UpdateBitsState();    	   	
    	
        // биты в результате процедуры LIU_Get_Bits_State
        enum
        {
            L_STATE_RDI_MF = 0,
            L_STATE_LSS,
            L_STATE_RDI,
            L_STATE_SERR,
            L_STATE_LOF,
            L_STATE_LOMF,
            L_STATE_LOS,
            L_STATE_AIS
        };
    	
    	
	private:

    	//Функциия тестирования доступа и наличия чипа	        
        void SaInsTransmit(byte sa8, byte sa7, byte sa6, byte sa5, byte sa4);
        void LongLine(bool mode);
    	void LoadBitsState();        
        bool Cycle(Enums::CycleType mode);
        void SignalNoStruct(bool mode);
        bool CRC4(bool mode);
        void ResetSync();
	    void LoadLOS();
	    void LoadAIS();    
	    void LoadFas();
	    void LoadRDI();	    
	    void LoadBert();
	    void LoadYellow();
    	bool isDS2155Present();
        byte ReceiveLevel();
        
        bool InitE1();
        bool InitT1();
    	
        
		IMembersToBoard &m_board;
		LiuProfile  m_profile;	            //настройки объекта линейной части Е1				
		byte        m_lPcpr;
		byte        m_lTestInterfInit;
		byte        m_stateLeds;
		word        m_ledsCondition;        // 7 -ais; 6 -los; 5-lomf; 4-lof; 3 - serr; 2 - rdi; 1 -lss		
		bool        m_yellowAlarm;				
		PriStatus	m_status;				// структура хранения статуса

	};
};

#endif







