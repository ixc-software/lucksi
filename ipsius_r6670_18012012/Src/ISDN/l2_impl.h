

#ifndef ISDN_L2_H
#define ISDN_L2_H

//#include "IsdnConfig.h"
#include "isdnl2.h"
#include "isdnpack.h"



//from hisax.h
/* Layer2 Flags */


namespace ISDN
{
	enum
    {
	    REQUEST	= 0,//	0
	    CONFIRM,//		1
	    INDICATION,//	2
	    RESPONSE,// 	3
	};

    enum
    {
	    DL_ESTABLISH = 0x0200,
	    DL_RELEASE =   0x0210,
	    DL_DATA =      0x0220,
	    DL_FLUSH =     0x0224,
	    DL_UNIT_DATA = 0x0230,
    };


    enum
    {
	    MDL_BC_RELEASE = 0x0278,  // Formula-n enter:now
	    MDL_BC_ASSIGN =  0x027C,  // Formula-n enter:now
	    MDL_ASSIGN =     0x0280,
	    MDL_REMOVE =     0x0284,
	    MDL_ERROR =      0x0288,
	    MDL_INFO_SETUP = 0x02E0,
	    MDL_INFO_CONN =  0x02E4,
	    MDL_INFO_REL =   0x02E8,
    };


    enum
    {
	    RR =   0x01,
	    RNR =  0x05,
	    REJ  = 0x09,
	    SABME =0x6f,
	    SABM = 0x2f,
	    DM  =  0x0f,
	    UI  =  0x03,
	    DISC = 0x43,
	    UA  =  0x63,
	    FRMR = 0x87,
	    XID  = 0xaf,
    };

    enum
    {  // C/R bit
	    CMD = 0,
	    RSP = 1,
    };

	const int LC_FLUSH_WAIT  = 1;



	static IsdnL2* GetSelf(FsmMachine *fi)
	{
		return fi->GetUserData();
	}

	static IsdnPacket* ArgToPack(void*arg)
	{
		return static_cast<IsdnPacket*>(arg);
	}
	
};

#endif
