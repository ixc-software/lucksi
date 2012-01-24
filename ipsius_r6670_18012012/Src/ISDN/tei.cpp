
#include "stdafx.h"
#include "l2_impl.h"
#include "tei.h"
#include "l2_const.h"
#include "Utils/ContainerConverter.h"


namespace ISDN
{
	static Fsm::StateArr strTeiState[] =
	{
		"ST_TEI_NOP",
		"ST_TEI_IDREQ",
		"ST_TEI_IDVERIFY",
	};

	static Fsm::EventArr strTeiEvent[] =
	{
		"EV_IDREQ",
		"EV_ASSIGN",
        "EV_ASSIGN_REQ",
		"EV_DENIED",
		"EV_CHKREQ",
		"EV_REMOVE",
		"EV_VERIFY",
		"EV_T202",
	};


//------------------
///void tei_handler(struct PStack *st, byte pr, struct sk_buff *skb);

	static bool fiTraced(FsmMachine* fi)
	{
		return GetSelf(fi)->getTraceOn();
	}
   
    IsdnL2::Layer2* findtei(IsdnL2::Layer2* l2, int tei){return 0;}

    static void put_tei_msg(IsdnL2::Layer2 *l2, byte m_id, unsigned int ri, byte tei)
    {
	    //struct sk_buff *skb;
	    std::vector<byte> bp(8);

	    //bp = skb_put(skb, 3);
	    bp.at(0) = (TEI_SAPI << 2);
	    bp.at(1) = (GROUP_TEI << 1) | 0x1;
	    bp.at(2) = UI;
	    //bp = skb_put(skb, 5);
	    bp.at(3) = TEI_ENTITY_ID;
	    bp.at(4) = ri >> 8;
	    bp.at(5) = ri & 0xff;
	    bp.at(6) = m_id;
	    bp.at(7) = (tei << 1) | 1;

	    //st->l2.l2l1(st, PH_DATA | REQUEST, skb);
	    //IPacket* pIPack=new(l2->owner->GetInfra()) IPacket;
        
	    
        // костыль, однако. TODO
        if ( l2->owner->getTraceOn() )
        {
            IsdnPacket* pPack = l2->owner->CreatePacket<IsdnPacket>();
	        pPack->AddBack(bp);
            l2->LogPacketSend( pPack );
            pPack->Delete();
        }
        QVector<Platform::byte> qvPacket;
        Utils::Converter<>::DoConvert(bp, qvPacket);

	    l2->owner->GetDownIntf()->DataRequest( qvPacket );
    }

    static void tei_id_request(FsmMachine *fi, int event, void *arg)
    {
	    //struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
	    IsdnL2 *pSelf = GetSelf(fi);	  		
	    IsdnL2::Layer2 *l2 = pSelf->getL2();

	    if ( l2->tei != -1) {		
		    //st->ma.tei_m.printdebug
    		
		    if(fiTraced(fi)) 
		    {
			    LogWriter( GetSelf(fi) ).Write()<<"-Assign request for allready asigned tei "<<l2->tei;
		    }
		    return;
	    }
	    //st->ma.ri = random_ri();
	    l2->m_TeiManage.ri = l2->Random_ri();
	    if(fiTraced(fi)) 
	    {
		    LogWriter( GetSelf(fi) ).Write()<<"-Assign request ri "<<l2->m_TeiManage.ri;
	    }
    	
	    put_tei_msg(l2, ID_REQUEST, l2->m_TeiManage.ri, 127); 
        fi->ChangeState(ST_TEI_IDREQ);
        
	    // FsmChangeState(&l2->m_TeiManage.tei_m, ST_TEI_IDREQ);
	    l2->m_TeiManage.t202.Set(l2->m_TeiManage.T202, EV_T202, NULL);
	    //FsmAddTimer(&l2->m_TeiManage.t202, l2->m_TeiManage.T202, EV_T202, NULL, 1);
	    l2->m_TeiManage.N202 = 3;
       
    }
    
    
    //Add tei_assign_req (mISDN)
    static void  tei_assign_req(FsmMachine *fi, int event, void *arg)
    {   
        IsdnL2 *pSelf = GetSelf(fi);	  		
	    IsdnL2::Layer2 *l2 = pSelf->getL2();
	    IsdnPacket *pPack = ArgToPack(arg);//static_cast<IsdnPacket*>(arg);
	    
        //teimgr_t *tm = fi->userdata;
	    //u_char *dp = arg;

        if (l2->tei == -1 && fiTraced(fi)) {
		     LogWriter( GetSelf(fi) ).Write()<<"-net tei assign request without tei";
		    return;
	    }
        l2->m_TeiManage.ri = (dword)pPack->GetByIndex(1)<<8;//((unsigned int) *dp++ << 8);
	    l2->m_TeiManage.ri += pPack->GetByIndex(2);//tm->ri += *dp++;
        //l2->m_TeiManage.ri = (pPack->GetByIndex(1) << 8) | (pPack->GetByIndex(2));
	    // назначаетс€ фикс значение ==100, смотри ToDo.txt
        if(fiTraced(fi)) 
	    {
		    LogWriter( GetSelf(fi) ).Write() << "-net assign request ri "
                << l2->m_TeiManage.ri << " tei = " << 100;//(int)( (byte)pPack->GetByIndex(4)<<1 );
	    }

        put_tei_msg(l2, ID_ASSIGNED, l2->m_TeiManage.ri, 100);//(tm, ID_ASSIGNED, tm->ri, tm->l2->tei);
	    l2->m_TeiManage.tei_m->ChangeState(ST_TEI_NOP);//mISDN_FsmChangeState(fi, ST_TEI_NOP);
    }
    
    static void tei_id_assign(FsmMachine *fi, int event, void *arg)
    {
	    //struct PStack *ost, *st = reinterpret_cast<PStack*>(fi->userdata);
	    IsdnL2 *pSelf = GetSelf(fi);	  		
	    IsdnL2::Layer2 *ost,*l2 = pSelf->getL2();
	    IsdnPacket *pPack = ArgToPack(arg);//static_cast<IsdnPacket*>(arg);
	    //struct IsdnCardState *cs;
	    int ri, tei;

	    ri = ((dword) pPack->GetByIndex(1) << 8) + pPack->GetByIndex(2);
	    tei = pPack->GetByIndex(4) >> 1;
    	
	    if(fiTraced(fi)) 
	    {
		    LogWriter( GetSelf(fi) ).Write()<<"-Identity assign ri = "<<ri<<" tei = "<<tei;
	    }
	    if ((ost = findtei(l2, tei))) {	/* same tei is in use */
		    if (ri != ost->m_TeiManage.ri) {
			    if(fiTraced(fi)) 
			    {
				    LogWriter( GetSelf(fi) ).Write()<<"-Possible duplicate assignment tei"<<tei;
			    }
			    ost->tei_l2tei(MDL_ERROR | RESPONSE, NULL);//call tei_l2tei
		    }
	    } else if (ri == l2->m_TeiManage.ri) {  // если дублирует то перезапрос
		    //FsmDelTimer(&l2->m_TeiManage.t202, 1);
		    l2->m_TeiManage.t202.Deactivate();
		    //FsmChangeState(&l2->m_TeiManage.tei_m, ST_TEI_NOP);
		    l2->m_TeiManage.tei_m->ChangeState(ST_TEI_NOP);
		    //st->l3.l3l2(l2, MDL_ASSIGN | REQUEST, (void *) (long) tei);
		    l2->owner->MAssignReq(/*(void *) (long) */tei); // здесь должно назначитс€ tei
		    //cs = (struct IsdnCardState *) st->l1.hardware;//!¬озможно эти сообщени€ нужно преобразовать в вызовы
		    //cs->cardmsg(cs, MDL_ASSIGN | REQUEST, NULL);//     процедур драйвера, но вр€дли.
	    }
    }

    static void tei_id_test_dup(FsmMachine *fi, int event, void *arg)
    {
    	
	    IsdnPacket *pPack = ArgToPack(arg);
	    IsdnL2 *pSelf = GetSelf(fi);	  		
	    IsdnL2::Layer2 *ost,*l2 = pSelf->getL2();
	    int tei, ri;

	    ri = ((unsigned int) pPack->GetByIndex(1) << 8) + pPack->GetByIndex(2);
	    tei = pPack->GetByIndex(4) >> 1;
	    if(fiTraced(fi)) 
	    {
		    LogWriter( GetSelf(fi) ).Write()<<"-Foreign identity assign ri "<<ri<<" tei "<<tei;
	    }
	    if ((ost = findtei(l2, tei))) {	/* same tei is in use */
		    if (ri != ost->m_TeiManage.ri) {	/* and it wasn't our request */
    			
			    if(fiTraced(fi)) 
			    {
				    LogWriter( GetSelf(fi) ).Write()<<"-Possible duplicate assignment tei  "<<tei;
			    }
		    }
	    } 
    }

    static void tei_id_denied(FsmMachine *fi, int event, void *arg)
    {
	    //struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
	    IsdnL2 *pSelf = GetSelf(fi);	  		
	    IsdnL2::Layer2 *l2 = pSelf->getL2();
	    //struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
	    IsdnPacket *pPack=ArgToPack(arg);
	    int ri, tei;

	    ri = ((unsigned int) pPack->GetByIndex(1) << 8) + pPack->GetByIndex(2);
	    tei = pPack->GetByIndex(4) >> 1;
	    if(fiTraced(fi)) 
	    {
		    LogWriter( GetSelf(fi) ).Write()<<"-Identity denied ri  "<<ri<<" tei "<<tei;
	    }
    }

    static void tei_id_chk_req(FsmMachine *fi, int event, void *arg)
    {
	    //struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
	    IsdnL2 *pSelf = GetSelf(fi);	  		
	    IsdnL2::Layer2 *l2 = pSelf->getL2();
	    IsdnPacket* pPack=ArgToPack(arg);
	    int tei;

	    tei = pPack->GetByIndex(4) >> 1;
        IsdnL2* pL2 = GetSelf(fi);
	    if(pL2->getTraceOn()) 
	    {
		    LogWriter( pL2, pL2->getLogTags().tei ).Write()<<"Identity check req tei  "<<tei;
	    }
	    if ((l2->tei != -1) && ((tei == GROUP_TEI) || (tei == l2->tei))) {
		    //FsmDelTimer(&l2->m_TeiManage.t202, 4);
		    l2->m_TeiManage.t202.Deactivate();
		    //FsmChangeState(&l2->m_TeiManage.tei_m, ST_TEI_NOP);
		    l2->m_TeiManage.tei_m->ChangeState(ST_TEI_NOP);
            put_tei_msg(l2, ID_CHK_RES, l2->Random_ri(), l2->tei);
	    }
    }

    static void tei_id_remove(FsmMachine *fi, int event, void *arg)
    {
	    //struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
	    IsdnL2 *pSelf = GetSelf(fi);	  		
	    IsdnL2::Layer2 *l2 = pSelf->getL2();
	    IsdnPacket *pPack=ArgToPack(arg);
	    int tei;

	    tei = pPack->GetByIndex(4) >> 1;
        IsdnL2* pL2 = GetSelf(fi);
	    if(pL2->getTraceOn()) 
	    {
		    LogWriter( pL2, pL2->getLogTags().tei ).Write()<<"Identity remove tei  "<<tei;
	    }

	    if ((l2->tei != -1) && ((tei == GROUP_TEI) || (tei == l2->tei))) {
		    //FsmDelTimer(&l2->m_TeiManage.t202, 5);
		    l2->m_TeiManage.t202.Deactivate();
		    //FsmChangeState(&l2->m_TeiManage.tei_m, ST_TEI_NOP);
		    l2->m_TeiManage.tei_m->ChangeState(ST_TEI_NOP);
		    //st->l3.l3l2(st, MDL_REMOVE | REQUEST, NULL);
		    l2->owner->MRemoveReq();
		    //cs = (struct IsdnCardState *) st->l1.hardware;//!¬озможно эти сообщени€ нужно преобразовать в вызовы
		    //cs->cardmsg(cs, MDL_REMOVE | REQUEST, NULL);
	    }
    }

    static void tei_id_verify(FsmMachine *fi, int event, void *arg)
    {
	    //struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
	    IsdnL2 *pSelf = GetSelf(fi);	  		
	    IsdnL2::Layer2 *l2 = pSelf->getL2();
	    if(pSelf->getTraceOn()) 
	    {
		    LogWriter( pSelf, pSelf->getLogTags().tei ).Write()<<"-Id verify request for tei  "<<l2->tei;
	    }

	    put_tei_msg(l2, ID_VERIFY, 0, l2->tei);
	    //FsmChangeState(&l2->m_TeiManage.tei_m, ST_TEI_IDVERIFY);
	    l2->m_TeiManage.tei_m->ChangeState(ST_TEI_IDVERIFY);
	    //FsmAddTimer(&l2->m_TeiManage.t202, l2->m_TeiManage.T202, EV_T202, NULL, 2);
	    l2->m_TeiManage.t202.Set(l2->m_TeiManage.T202, EV_T202, NULL);
	    l2->m_TeiManage.N202 = 2;
    }

    static void tei_id_req_tout(FsmMachine *fi, int event, void *arg)
    {
	    //struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
	    IsdnL2 *pSelf = GetSelf(fi);	  		
	    IsdnL2::Layer2 *l2 = pSelf->getL2();
	    //struct IsdnCardState *cs;

	    if (--l2->m_TeiManage.N202) {//!проверь
		    l2->m_TeiManage.ri = l2->Random_ri();
    	
	    if(pSelf->getTraceOn()) 
	    {
		    LogWriter( pSelf, pSelf->getLogTags().tei ).Write()<<"Assign req("
			    << 4 - l2->m_TeiManage.N202<<") ri "<<l2->m_TeiManage.ri;
	    }
		    put_tei_msg(l2, ID_REQUEST, l2->m_TeiManage.ri, 127);
		    //FsmAddTimer(&l2->m_TeiManage.t202, l2->m_TeiManage.T202, EV_T202, NULL, 3);
		    l2->m_TeiManage.t202.Set(l2->m_TeiManage.T202, EV_T202, NULL);
	    } else {
		    if(pSelf->getTraceOn())
			   pSelf->DoLog("Assign req failed");
		    //l2->m_TeiManage.tei_m.printdebug(&l2->m_TeiManage.tei_m, "assign req failed");
		    //st->l3.l3l2(st, MDL_ERROR | RESPONSE, NULL);
		    l2->owner->MErrorResponse();
		    //cs = (struct IsdnCardState *) st->l1.hardware;!¬озможно эти сообщени€ нужно преобразовать в вызовы
		    //cs->cardmsg(cs, MDL_REMOVE | REQUEST, NULL);
		    //FsmChangeState(fi, ST_TEI_NOP);
		    fi->ChangeState(ST_TEI_NOP);
	    }
    }

    static void tei_id_ver_tout(FsmMachine *fi, int event, void *arg)
    {
	    //struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
	    IsdnL2 *pSelf = GetSelf(fi);	  		
	    IsdnL2::Layer2 *l2 = pSelf->getL2();
	    //struct IsdnCardState *cs;

	    if (--l2->m_TeiManage.N202) {
			    if(pSelf->getTraceOn()) 
			    {
				    LogWriter( pSelf, pSelf->getLogTags().tei ).Write()<<"-id verify req("
					    <<3 - l2->m_TeiManage.N202<<") for tei "<<l2->tei;
			    }
		    put_tei_msg(l2, ID_VERIFY, 0, l2->tei);
		    //FsmAddTimer(&l2->m_TeiManage.t202, l2->m_TeiManage.T202, EV_T202, NULL, 4);
		    l2->m_TeiManage.t202.Set(l2->m_TeiManage.T202, EV_T202,0);
	    } else {
		    if(pSelf->getTraceOn()) 
		    {
			    LogWriter( pSelf, pSelf->getLogTags().tei ).Write()<<"-id verify req for tei failed tei="<<l2->tei;
		    }
    				
		    //st->l3.l3l2(st, MDL_REMOVE | REQUEST, NULL);
		    l2->owner->MRemoveReq();
		    //cs = (struct IsdnCardState *) st->l1.hardware;!¬озможно эти сообщени€ нужно преобразовать в вызовы
		    //cs->cardmsg(cs, MDL_REMOVE | REQUEST, NULL);
		    //FsmChangeState(fi, ST_TEI_NOP);
		    fi->ChangeState(ST_TEI_NOP);
	    }
    }



    //
    //static void //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //tei_debug(FsmMachine *fi, char *fmt, ...)
    //{
    //	/*va_list args;
    //	struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
    //
    //	va_start(args, fmt);
    //	VHiSax_putstatus(st->l1.hardware, "tei ", fmt, args);
    //	va_end(args);*/
    //}
    //
    //void
    //SetTeiManage(IsdnL2::Layer2* l2)
    //{
    //	l2->l2tei = tei_l2tei;
    //	l2->m_TeiManage.T202 = 2000;	/* T202  2000 milliseconds */
    //	//st->l1.l1tei = tei_l1l2;!!!!????
    //	//l2->m_TeiManage.debug = 1;
    //	l2->m_TeiManage.tei_m.fsm = &teifsm;
    //	l2->m_TeiManage.tei_m.state = ST_TEI_NOP;
    //	l2->m_TeiManage.tei_m.IsLogged = true;
    //	l2->m_TeiManage.tei_m.userdata = l2;
    //	l2->m_TeiManage.tei_m.userint = 0;
    //	//l2->m_TeiManage.tei_m.printdebug = tei_debug;
    //	FsmInitTimer(&l2->m_TeiManage.tei_m, &l2->m_TeiManage.t202);
    //}


    static struct FsmNode TeiFnList[]  =
    {
	    {ST_TEI_NOP, EV_IDREQ, tei_id_request},
	    {ST_TEI_NOP, EV_ASSIGN, tei_id_test_dup},
        {ST_TEI_NOP, EV_ASSIGN_REQ, tei_assign_req},//add!!
	    {ST_TEI_NOP, EV_VERIFY, tei_id_verify},
	    {ST_TEI_NOP, EV_REMOVE, tei_id_remove},
	    {ST_TEI_NOP, EV_CHKREQ, tei_id_chk_req},
	    {ST_TEI_IDREQ, EV_T202, tei_id_req_tout},
	    {ST_TEI_IDREQ, EV_ASSIGN, tei_id_assign},
	    {ST_TEI_IDREQ, EV_DENIED, tei_id_denied},
	    {ST_TEI_IDVERIFY, EV_T202, tei_id_ver_tout},
	    {ST_TEI_IDVERIFY, EV_REMOVE, tei_id_remove},
	    {ST_TEI_IDVERIFY, EV_CHKREQ, tei_id_chk_req},
    };

    #define TEI_FN_COUNT (sizeof(TeiFnList)/sizeof(struct FsmNode))

    //int __init
    //TeiNew(/*IIsdnAlloc* allocator*/)
    //{
    //	/*teifsm.state_count = TEI_STATE_COUNT;
    //	teifsm.event_count = TEI_EVENT_COUNT;
    //	teifsm.strEvent = strTeiEvent;
    //	teifsm.strState = strTeiState;*/
    //	//return FsmNew(/*allocator,*/&teifsm, TeiFnList, TEI_FN_COUNT);
    //}



    FsmMachine* TeiManage::CreateFsm_tei(IsdnL2* owner, IIsdnClock* clock)
    {
	    FsmMachine* pFsm = new FsmMachine(TeiFnList, TEI_FN_COUNT, 
		    TEI_STATE_COUNT,TEI_EVENT_COUNT,strTeiEvent,strTeiState,
		    clock,  owner ,
		    owner, 0, "TeiMachine");
	    return pFsm;
    }
    void TeiManage::DeleteFsm_tei()
    {
	    delete tei_m;
    }

};//ISDN


