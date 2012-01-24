#ifndef TEI_H
#define TEI_H

#include "fsm.h"
#include "Platform/PlatformTypes.h"

namespace ISDN
{
	
	//static Fsm teifsm(TEI_STATE_COUNT,TEI_EVENT_COUNT,strTeiEvent,strTeiState);
	enum 
    {
	    ID_REQUEST=1,
	    ID_ASSIGNED,//	2
	    ID_DENIED,//	3
	    ID_CHK_REQ,//	4
	    ID_CHK_RES,//	5
	    ID_REMOVE,//	6
	    ID_VERIFY,//	7
	};

	enum 
    {
	    ST_TEI_NOP,
	    ST_TEI_IDREQ,
	    ST_TEI_IDVERIFY,
	};

	enum {
		EV_IDREQ,
		EV_ASSIGN,
        EV_ASSIGN_REQ,
		EV_DENIED,
		EV_CHKREQ,
		EV_REMOVE,
		EV_VERIFY,
		EV_T202,
	};

	
	const byte TEI_ENTITY_ID =	0xf;

	const byte TEI_STATE_COUNT = (ST_TEI_IDVERIFY+1);

	const byte TEI_EVENT_COUNT = (EV_T202+1);


	class IsdnL2;
	struct TeiManage 
	{
        TeiManage(IsdnL2* owner, IIsdnClock* clock):
		    ri(0),
            tei_m(CreateFsm_tei(owner, clock)),
			t202(tei_m), T202(2000)
		{
			tei_m->ChangeState(ST_TEI_NOP);
			T202 = 2000;
			N202 = 0;
		}

		~TeiManage()
		{
			DeleteFsm_tei();
		}

		int	ri;
		FsmMachine *tei_m;
		FsmTimer t202;
		int T202, N202;
		
	private:

		static FsmMachine* CreateFsm_tei(IsdnL2* owner, IIsdnClock* clock);

		void DeleteFsm_tei();
	};
}//ISDN

#endif//TEI_H

