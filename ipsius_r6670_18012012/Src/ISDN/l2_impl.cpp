
#include "stdafx.h"
#include "IsdnLayersInterfaces.h"
#include "isdnpack.h"
#include "l2_impl.h"
#include "fsm.h"
#include "l2_const.h"
#include "Utils/ErrorsSubsystem.h"


namespace ISDN
{

	enum
	{
		ST_L2_1,
		ST_L2_2,
		ST_L2_3,
		ST_L2_4,
		ST_L2_5,
		ST_L2_6,
		ST_L2_7,
		ST_L2_8,
	};

#define L2_STATE_COUNT (ST_L2_8+1)

	static Fsm::StateArr strL2State[] =
	{
		"ST_L2_1",
		"ST_L2_2",
		"ST_L2_3",
		"ST_L2_4",
		"ST_L2_5",
		"ST_L2_6",
		"ST_L2_7",
		"ST_L2_8",
	};

	enum
	{
		EV_L2_UI,
		EV_L2_SABME,
		EV_L2_DISC,
		EV_L2_DM,
		EV_L2_UA,
		EV_L2_FRMR,
		EV_L2_SUPER,
		EV_L2_I,
		EV_L2_DL_DATA,
		EV_L2_ACK_PULL,
		EV_L2_DL_UNIT_DATA,
		EV_L2_DL_ESTABLISH_REQ,
		EV_L2_DL_RELEASE_REQ,
		EV_L2_MDL_ASSIGN,
		EV_L2_MDL_REMOVE,
		EV_L2_MDL_ERROR,
		EV_L1_DEACTIVATE,
		EV_L2_T200,
		EV_L2_T203,
		EV_L2_SET_OWN_BUSY,
		EV_L2_CLEAR_OWN_BUSY,
		EV_L2_FRAME_ERROR,
	};

#define L2_EVENT_COUNT (EV_L2_FRAME_ERROR+1)

	static Fsm::EventArr strL2Event[] =
	{
		"EV_L2_UI",
		"EV_L2_SABME",
		"EV_L2_DISC",
		"EV_L2_DM",
		"EV_L2_UA",
		"EV_L2_FRMR",
		"EV_L2_SUPER",
		"EV_L2_I",
		"EV_L2_DL_DATA",
		"EV_L2_ACK_PULL",
		"EV_L2_DL_UNIT_DATA",
		"EV_L2_DL_ESTABLISH_REQ",
		"EV_L2_DL_RELEASE_REQ",
		"EV_L2_MDL_ASSIGN",
		"EV_L2_MDL_REMOVE",
		"EV_L2_MDL_ERROR",
		"EV_L1_DEACTIVATE",
		"EV_L2_T200",
		"EV_L2_T203",
		"EV_L2_SET_OWN_BUSY",
		"EV_L2_CLEAR_OWN_BUSY",
		"EV_L2_FRAME_ERROR",
	};
	//---------------------------------------------------------------------------------------------


    ObjLink::ObjectLink<IL2ToDriver>& L2Down(IsdnL2::Layer2 *l2) 
	{
		return l2->owner->GetDownIntf();
	}

	/*inline bool IsLogged(IsdnL2::Layer2 *l2)
	{
		return l2->owner->getTraceOn();
	}*/

	inline IsdnL2* Owner(IsdnL2::Layer2 *l2)
	{
		return l2->owner;
	}

	void sendToMErrorInd(IsdnL2 *pL2,char errCode)
	{
        L2Error err(errCode);
		pL2->MErrorIndication(err);
	}

	bool test_and_set_flag(bool &flg)
	{
		bool ret = flg;
		flg = true;
		return ret;
	}

	bool test_flag(bool &flg)
	{
		return flg;
	}

	bool test_and_clear_flag(bool &flg)
	{
		bool ret = flg;
		flg = false;
		return ret;
	}

	static void set_peer_busy(IsdnL2::Layer2 *l2) 
	{
		test_and_set_flag(l2->m_flag.PEER_BUSY);
		if ( !l2->m_iQueue.IsEmpty() ||
			 !l2->m_uiQueue.IsEmpty() )
			test_and_set_flag(l2->m_flag.L2BLOCK);
	}

	static void clear_peer_busy(IsdnL2::Layer2 *l2) 
	{
		if ( test_and_clear_flag(l2->m_flag.PEER_BUSY) )
			test_and_clear_flag(l2->m_flag.L2BLOCK);
	}

	/* int WinSize(IsdnL2::Layer2 *l2)
	 {
		 l2->GetMaxWindow()
	 }*/

	static int//!replace it in constructor
	freewin1(IsdnL2::Layer2 *l2)
	{
		int i, cnt = 0;

		for ( i = 0; i < l2->GetMaxWindow(); i++ )
		{
			if ( l2->m_windowAr.at(i) )
			{
				cnt++;
				//dev_kfree_skb(l2->windowar[i]);
				//delete (l2->m_windowar[i],l2->owner->GetInfra());
				l2->m_windowAr.at(i)->Delete();
				l2->m_windowAr.at(i) = 0;
			}
		}
		return cnt;
	}

	static inline void  freewin(IsdnL2::Layer2* l2)
	{
		freewin1(l2);
	}

	static void ReleaseWin(IsdnL2::Layer2 *l2)
	{
		int cnt;

		if ( (cnt = freewin1(l2)) )
        {
			//printk(KERN_WARNING "isdl2 freed %d skbuffs in release\n", cnt);
            ILoggable& log = l2->getLogSession();
            log.getTraceOn();
            if (log.getTraceOn())
            {
                LogWriter(&log, l2->getLogTags().general).Write() 
                    << "Warning: Feed " << cnt << " packages in release.";
            }
			
        }
	}

	static inline bool
	cansend(IsdnL2::Layer2* l2)
	{
		unsigned int p1;

		if ( test_flag(l2->m_flag.MOD128) )
			p1 = (l2->vs - l2->va) % 128;
		else
			p1 = (l2->vs - l2->va) % 8;
		return((p1 < l2->GetMaxWindow()) && !test_flag(l2->m_flag.PEER_BUSY));
	}

	static inline void clear_exception(IsdnL2::Layer2 *l2)
	{
		test_and_clear_flag(l2->m_flag.ACK_PEND);
		test_and_clear_flag(l2->m_flag.REJEXC);
		test_and_clear_flag(l2->m_flag.OWN_BUSY);
		clear_peer_busy(l2);
	}

	static inline int l2headersize(IsdnL2::Layer2 *l2, int ui)
	{
		return(((test_flag(l2->m_flag.MOD128) && (!ui)) ? 2 : 1) +
			   (test_flag(l2->m_flag.LAPD) ? 2 : 1));
	}

	inline unsigned int
	l2addrsize(IsdnL2::Layer2 *l2)
	{
		return(test_flag(l2->m_flag.LAPD) ? 2 : 1);
	}

	//формирует некоторое адресное поле в переданном масиве header и возвращает занятую им(полем) в масиве длину
	static int sethdraddr(IsdnL2::Layer2 *l2, byte* header, int rsp)
	{

		byte *ptr = header;
		int crbit = rsp;
		if ( !l2->m_flag.ORIG )	rsp = !rsp;	// !!!!Добавленно. Отличается от исходного!

		if ( test_flag(l2->m_flag.LAPD) )
		{

			*ptr++ = (l2->sap << 2) | (rsp ? 2 : 0);
			*ptr++ = (l2->tei << 1) | 1;
			return(2);
		}
		else
		{
			if ( test_flag(l2->m_flag.ORIG) )
				crbit = !crbit;
			if ( crbit )
				*ptr++ = 1;
			else
				*ptr++ = 3;
			return(1);
		}
	}

	static inline void enqueue_super(IsdnL2::Layer2 *l2,IsdnPacket * pPack)
	{
		if ( test_flag(l2->m_flag.LAPB) )
			l2->BCStx_cntInc(pPack->GetLenData());
		//l2->l2l1(PH_DATA | REQUEST, skb);
        
        l2->LogPacketSend(pPack);
		L2Down(l2)->DataRequest(pPack->getAsQVector());
        pPack->Delete();
        l2->getStat().AllTxInc();
	}

#define enqueue_ui(a, b) enqueue_super(a, b)

	int IsUI(byte data0)
	{
		return((data0 & 0xef) == UI);
	}

	int IsUA(byte data0)
	{
		return((data0 & 0xef) == UA);
	}

	int IsDM(byte data0)
	{
		return((data0 & 0xef) == DM);
	}

	int IsDISC(byte data0)
	{
		return((data0 & 0xef) == DISC);
	}

	int IsSFrame(byte data0, IsdnL2::Layer2* l2)
	{
		register byte d = data0;

		if ( !test_flag(l2->m_flag.MOD128) )
			d &= 0xf;
		return(((d & 0xf3) == 1) && ((d & 0x0c) != 0x0c));
	}

	int IsSABME(byte data0, IsdnL2::Layer2* l2)
	{
		byte d = data0 & ~0x10;
		return(test_flag(l2->m_flag.MOD128) ? d == SABME : d == SABM);
	}

	int IsREJ(byte data0, IsdnL2::Layer2* l2)
	{
		return(test_flag(l2->m_flag.MOD128) ? data0 == REJ : (data0 & 0xf) == REJ);
	}

	int IsFRMR(byte data0)
	{
		return((data0 & 0xef) == FRMR);
	}

	int IsRNR(byte  data0, IsdnL2::Layer2* l2)
	{
		//return (test_flag(FLG_MOD128, &l2->flag) ? data[0] == RNR : (data[0] & 0xf) == RNR);
		return(test_flag(l2->m_flag.MOD128) ? data0 == RNR : (data0 & 0xf) == RNR);
	}

	int iframe_error(IsdnL2::Layer2* l2, IsdnPacket* pPack)
	{
		unsigned int i = l2addrsize(l2) + (test_flag(l2->m_flag.MOD128) ? 2 : 1);

		int rsp = pPack->GetByIndex(0) & 0x2;

		if ( test_flag(l2->m_flag.ORIG) )
			rsp = !rsp;

		if ( rsp )
			return 'L';


		if ( pPack->GetLenData() < i )
			return 'N';

		if ( (pPack->GetLenData() - i) > l2->maxlen )
			return 'O';


		return 0;
	}

	int super_error(IsdnL2::Layer2* l2, IsdnPacket *pPack)
	{
		if ( pPack->GetLenData() != l2addrsize(l2) +
			 (test_flag(l2->m_flag.MOD128) ? 2 : 1) )
			return 'N';

		return 0;
	}

	int unnum_error(IsdnL2::Layer2* l2, IsdnPacket *pPack, int wantrsp)
	{
		//int rsp = (*skb->data & 0x2) >> 1;
		int rsp = (pPack->GetByIndex(0) & 0x2) >> 1;
		if ( test_flag(l2->m_flag.ORIG) ) // if (test_flag(l2->m_flag.ORIG))
			rsp = !rsp;

		if ( rsp != wantrsp )
			return 'L';

		if ( pPack->GetLenData() != l2addrsize(l2) + 1 )
			return 'N';

		return 0;
	}

	int UI_error(IsdnL2::Layer2* l2, IsdnPacket *pPack)
	{
		int rsp = pPack->GetByIndex(0) & 0x2;
		if ( test_flag(l2->m_flag.ORIG) ) // if (test_flag(l2->m_flag.ORIG))
			rsp = !rsp;

		if ( rsp )
			return 'L';

		if ( pPack->GetLenData() > l2->maxlen + l2addrsize(l2) + 1 )
			return 'O';

		return 0;
	}

    void TraceFRMR_error(IsdnL2::Layer2* l2, IsdnPacket *pPack, int headers, int count)
    {
        if (!l2->getLogSession().getTraceOn()) return;
        
        LogWriter logWriter(&l2->getLogSession(), l2->getLogTags().frameErrorInfo);
        for (int i = 0; i < count; ++i)
        {
            logWriter.Write() << pPack->GetByIndex(i+headers);
        }
    }

	int FRMR_error(IsdnL2::Layer2* l2,IsdnPacket *pPack)
	{
		unsigned int headers = l2addrsize(l2) + 1;		

		int rsp = pPack->GetByIndex(headers) & 0x2;

		if ( test_flag(l2->m_flag.ORIG) ) rsp = !rsp;

		if ( !rsp ) return 'L';

        int count = test_flag(l2->m_flag.MOD128) ? 3 : 5;
        if ( pPack->GetLenData() < headers + count ) return 'N';            
        TraceFRMR_error(l2, pPack, headers, count);	        

		return 0;
	}

	static bool legalnr(IsdnL2::Layer2 *l2, unsigned int nr)
	{

		if ( test_flag(l2->m_flag.MOD128) )
			return((nr - l2->va) % 128) <= ((l2->vs - l2->va) % 128);
		else
			return((nr - l2->va) % 8) <= ((l2->vs - l2->va) % 8);
	}

    // все отправленные подтверждены
    static bool WinIsEmpty(IsdnL2::Layer2 *l2)
    {        
        return l2->vs == l2->va;
    }

    // установка счетчика подтвержденных пакетов, зачистка благополучно доставленных
	static void setva(IsdnL2::Layer2 *l2, unsigned int nr)
	{
		int len;
		//u_long flags;

		//spin_lock_irqsave(&l2->lock, flags);
		while ( l2->va != nr )
		{
			(l2->va)++;
			if ( test_flag(l2->m_flag.MOD128) )
				l2->va %= 128;
			else
				l2->va %= 8;
			//len = l2->windowar[l2->sow]->len;
		//len= l2->m_windowAr.at(l2->sow)->GetLenData();
//			if (PACKET_NOACK == l2->m_windowar[l2->sow]->GetPktType())
//				len = -1;

			//delete (l2->m_windowar[l2->sow],l2->owner->GetInfra());//dev_kfree_skb(l2->windowar[l2->sow]);
			l2->m_windowAr.at(l2->sow)->Delete();
			l2->m_windowAr.at(l2->sow) = 0;
			l2->sow = (l2->sow + 1) % l2->GetMaxWindow();
			//spin_unlock_irqrestore(&l2->lock, flags);
//			if (test_flag(FLG_LLI_L2WAKEUP, &st->lli.flag) && (len >=0))//st!;
//				lli_writewakeup(st, len);
			//spin_lock_irqsave(&l2->lock, flags);
		}
		
        //spin_unlock_irqrestore(&l2->lock, flags);
        if ( WinIsEmpty(l2) )
            l2->owner->GetUpIntf()->FreeWinInd();
	}


	//sk_buff* alloc_skb(int, int);//убрать

	static void send_uframe(IsdnL2::Layer2 *l2, byte cmd, byte cr)
	{		
		IsdnPacket*pPack=l2->owner->CreatePacket<IsdnPacket>();


		byte tmp[MAX_HEADER_LEN];
		ESS_ASSERT(   (MAX_HEADER_LEN >= 3) && "Invalid MAX_HEADER_LEN value!!!"   );
		int i = sethdraddr(l2, tmp, cr);
		ESS_ASSERT( i < MAX_HEADER_LEN );
		tmp[i++] = cmd;
		
		//memcpy(skb_put(skb, i), tmp, i);
		pPack->AddBack ( std::vector<byte>(tmp,tmp+i) );
		enqueue_super(l2, pPack);
	}

	static inline byte get_PollFlag(IsdnL2::Layer2 *l2, IsdnPacket *pPack)
	{
		//return (skb->data[l2addrsize(l2)] & 0x10);
		return(   pPack->GetByIndex( l2addrsize(l2) ) & 0x10   );
	}

	static inline byte  get_PollFlagFree(IsdnL2::Layer2 *l2, IsdnPacket *pPack)
	{
		byte PF;

		PF = get_PollFlag(l2, pPack);
		//dev_kfree_skb(skb);
		//delete (pPack,l2->owner->GetInfra());
		pPack->Delete();
		return(PF);
	}

	static inline void start_t200(IsdnL2::Layer2 *l2, int i)
	{
		//FsmAddTimer(&l2->t200, l2->T200, EV_L2_T200, NULL, i);
		l2->t200.Set(l2->T200, EV_L2_T200, 0);
		test_and_set_flag(l2->m_flag.T200_RUN);
	}

	static inline void restart_t200(IsdnL2::Layer2 *l2, int i)
	{
		//FsmRestartTimer(&l2->t200, l2->T200, EV_L2_T200, NULL, i);
		l2->t200.Restart(l2->T200, EV_L2_T200, 0);
		test_and_set_flag(l2->m_flag.T200_RUN);
	}

	static inline void stop_t200(IsdnL2::Layer2 *l2/*,where int i*/)
	{
		if ( test_and_clear_flag(l2->m_flag.T200_RUN) )
			//FsmDelTimer(&l2->t200, i);
			l2->t200.Deactivate();
	}

	static inline void st5_dl_release_l2l3(IsdnL2::Layer2 *l2)
	{		
        //todo!
        // нет критерия по которому можно судить был ли до этого
        // активен L2. Если не был, то не надо GetUpIntf()->ReleaseInd(); и getStat().DeactivateMoment()

		if ( test_and_clear_flag(l2->m_flag.PEND_REL) )
        {         
			l2->owner->GetUpIntf()->ReleaseConf();            
        }
        else
        {			
			l2->owner->GetUpIntf()->ReleaseInd();            
        }

        
        l2->getStat().DeactivateMoment();
	}

	static inline void lapb_dl_release_l2l3(IsdnL2::Layer2 *l2, int f)
	{
		if ( test_flag(l2->m_flag.LAPB) )
			//l2->l2l1(PH_DEACTIVATE | REQUEST, NULL); 
			L2Down(l2)->DeactivateRequest();
		//l2->l2l3(DL_RELEASE | f, NULL);  
		if ( f == CONFIRM )
		{
			l2->owner->GetUpIntf()->ReleaseConf();
            l2->getStat().DeactivateMoment();
            return;
		}
		if ( f == INDICATION )
		{
			l2->owner->GetUpIntf()->ReleaseInd();
            l2->getStat().DeactivateMoment();
            return;
		}
	}

	static void establishlink(FsmMachine *fi)
	{		
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
		byte cmd;

		clear_exception(l2);
		l2->rc = 0;
		cmd = (test_flag(l2->m_flag.MOD128) ? SABME : SABM) | 0x10;
		send_uframe(l2, cmd, CMD);
		//FsmDelTimer(&l2->t203, 1);
		l2->t203.Deactivate();
		restart_t200(l2, 1);
		test_and_clear_flag(l2->m_flag.PEND_REL);
		freewin(l2);
		//FsmChangeState(fi, ST_L2_5);
		l2->pl2m->ChangeState(ST_L2_5);
	}
//--------------------------------------------------------------------------------------
	//методы Layer2:
	static void l2_mdl_error_ua(FsmMachine *fi, int event, void *arg)
	{
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IsdnPacket* pPack=ArgToPack(arg);
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		if ( get_PollFlagFree(l2, pPack) )
			sendToMErrorInd(pSelf,'C');
		else
			sendToMErrorInd(pSelf,'D');
	}

	static void l2_mdl_error_dm(FsmMachine *fi, int event, void *arg)
	{
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IsdnPacket* pPack=ArgToPack(arg);
		//struct PStack *st =reinterpret_cast<PStack*>( fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		if ( get_PollFlagFree(l2, pPack) )
			sendToMErrorInd(pSelf,'B');
		else
		{
			sendToMErrorInd(pSelf,'E');
			establishlink(fi);
			test_and_clear_flag( l2->m_flag.L3_INIT);
		}
	}

	static void l2_st8_mdl_error_dm(FsmMachine *fi, int event, void *arg)
	{
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IsdnPacket* pPack=ArgToPack(arg);
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		if ( get_PollFlagFree(l2, pPack) )
			sendToMErrorInd(pSelf,'B');
		else
		{
			sendToMErrorInd(pSelf,'E');
		}
		establishlink(fi);
		test_and_clear_flag( l2->m_flag.L3_INIT);
	}

	static void l2_go_st3(FsmMachine *fi, int event, void *arg)
	{
		//FsmChangeState(fi, ST_L2_3); 
		fi->ChangeState(ST_L2_3);
	}

	static void l2_mdl_assign(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st =reinterpret_cast<PStack*>( fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		//FsmChangeState(fi, ST_L2_3); 
		fi->ChangeState(ST_L2_3);
		//l2->l2tei(l2,MDL_ASSIGN | INDICATION, NULL);
		l2->tei_l2tei(MDL_ASSIGN | INDICATION, 0);
	}

	static void l2_queue_ui_assign(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IsdnPacket* pPack=ArgToPack(arg);

		//skb_queue_tail(&l2->ui_queue, pPack);
		l2->m_uiQueue.AddBack(pPack);
		//FsmChangeState(fi, ST_L2_2);
		fi->ChangeState(ST_L2_2);
		l2->tei_l2tei(MDL_ASSIGN | INDICATION, 0);
	}

	static void l2_queue_ui(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IsdnPacket* pPack=ArgToPack(arg);

		//skb_queue_tail(&l2->ui_queue, ...);
		l2->m_uiQueue.AddBack(pPack);
	}

	static void tx_ui(IsdnL2::Layer2 *l2)
	{
		//struct sk_buff *skb;
		IsdnPacket* pPack;
		byte header[MAX_HEADER_LEN];
		int i = sethdraddr( l2, header, CMD);
		header[i++] = UI;
		/*while ((skb = skb_dequeue(&l2->ui_queue))) {
			memcpy(skb_push(skb, i), header, i);
			enqueue_ui(l2, skb);
		}*/
        
		while ( !l2->m_uiQueue.IsEmpty() )
		{
			pPack=l2->m_uiQueue.Dequeue();
			pPack->AddFront( std::vector<byte>(header,header+i) );
			enqueue_ui(l2, pPack);
            l2->getStat().UPackTxInc();
		}

	}

	static void l2_send_ui(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IsdnPacket *pPack=ArgToPack(arg);

		//skb_queue_tail(&l2->ui_queue, skb);
		l2->m_uiQueue.AddBack(pPack);
		tx_ui(l2);
	}

	static void l2_got_ui(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
        l2->getStat().UPackRxInc();
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IsdnPacket *pPack=ArgToPack(arg);
		//skb_pull(skb, l2headersize(l2, 1));
		pPack->ClearFront(l2headersize(l2, 1));
		//l2->l2l3(DL_UNIT_DATA | INDICATION, skb);
		
        //l2->owner->GetUpIntf()->UDataInd( static_cast<UPacket *>(pPack) );
        l2->owner->GetUpIntf()->UDataInd( pPack->getAsQVector() );
        pPack->Delete();
	}

    // обработчик запроса активации
	static void l2_establish(FsmMachine *fi, int event, void *arg)
	{		
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		establishlink(fi); // send SUBM/SUBME
		test_and_set_flag( l2->m_flag.L3_INIT);
	}

	static void l2_discard_i_setl3(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		//skb_queue_purge(&l2->i_queue);
		l2->m_iQueue.Clear();
		test_and_set_flag( l2->m_flag.L3_INIT);
		test_and_clear_flag( l2->m_flag.PEND_REL);
	}

	static void l2_l3_reestablish(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		//skb_queue_purge(&l2->i_queue);
		l2->m_iQueue.Clear();
		establishlink(fi);
		test_and_set_flag( l2->m_flag.L3_INIT);
	}

	static void l2_release(FsmMachine *fi, int event, void *arg)
	{		
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		l2->owner->GetUpIntf()->ReleaseConf();
        l2->getStat().DeactivateMoment();
    }

	static void l2_pend_rel(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		test_and_set_flag( l2->m_flag.PEND_REL);
	}

	static void l2_disconnect(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		//skb_queue_purge(&l2->i_queue);
		l2->m_iQueue.Clear();
		freewin(l2);
		//FsmChangeState(fi, ST_L2_6);
		fi->ChangeState(ST_L2_6);
		l2->rc = 0;
		send_uframe(l2, DISC | 0x10, CMD);
		//FsmDelTimer(&l2->t203, 1);
		l2->t203.Deactivate();
		restart_t200(l2, 2);
	}

	static void l2_start_multi(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IsdnPacket *pPack=ArgToPack(arg);

		send_uframe(l2, UA | get_PollFlagFree(l2, pPack), RSP);

		clear_exception(l2);
		l2->vs = 0;
		l2->va = 0;
		l2->vr = 0;
		l2->sow = 0;
		//FsmChangeState(fi, ST_L2_7);
		fi->ChangeState(ST_L2_7);
		//FsmAddTimer(&l2->t203, l2->T203, EV_L2_T203, NULL, 3);
		l2->t203.Set(l2->T203, EV_L2_T203, 0);

		//l2->l2l3(DL_ESTABLISH | INDICATION, NULL);
		l2->owner->GetUpIntf()->EstablishInd();
        l2->getStat().ActivateMoment();
	}

	static void l2_send_UA(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IsdnPacket *pPack=ArgToPack(arg);

		send_uframe(l2, UA | get_PollFlagFree(l2, pPack), RSP);
	}

	static void l2_send_DM(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IsdnPacket *pPack=ArgToPack(arg);

		send_uframe(l2, DM | get_PollFlagFree(l2, pPack), RSP);
	}

	static void l2_restart_multi(FsmMachine *fi, int event, void *arg) //no call?
	{
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
		IsdnPacket *pPack=ArgToPack(arg);

		int est = 0, state;

		state = fi->GetState();

		send_uframe(l2, UA | get_PollFlagFree(l2, pPack), RSP);

		sendToMErrorInd(pSelf,'F');

		if ( l2->vs != l2->va )
		{
			//skb_queue_purge(&l2->i_queue);
			l2->m_iQueue.Clear();
			est = 1;
		}

		clear_exception(l2);
		l2->vs = 0;
		l2->va = 0;
		l2->vr = 0;
		l2->sow = 0;
		//FsmChangeState(fi, ST_L2_7);
		fi->ChangeState(ST_L2_7);
		stop_t200(l2);
		//FsmRestartTimer(&l2->t203, l2->T203, EV_L2_T203, NULL, 3);
		l2->t203.Restart(l2->T203, EV_L2_T203, 0);

		if ( est )
        {
			//l2->l2l3(DL_ESTABLISH | INDICATION, NULL);
			Owner(l2)->GetUpIntf()->EstablishInd();
            l2->getStat().DeactivateMoment();
            l2->getStat().ActivateMoment();
        }

		if ( (ST_L2_7==state) || (ST_L2_8 == state) )
			if ( !l2->m_iQueue.IsEmpty() && cansend(l2) )
				//l2->l2l1(PH_PULL | REQUEST, NULL);
				L2Down(l2)->PullRequest();
	}

	static void l2_stop_multi(FsmMachine *fi, int event, void *arg)
	{
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
		IsdnPacket *pPack=ArgToPack(arg);

		//FsmChangeState(fi, ST_L2_4);
		fi->ChangeState(ST_L2_4);
		//FsmDelTimer(&l2->t203, 3);
		l2->t203.Deactivate();
		stop_t200(l2);

		send_uframe(l2, UA | get_PollFlagFree(l2, pPack), RSP);

		//skb_queue_purge(&l2->i_queue);
		l2->m_iQueue.Clear();
		freewin(l2);
		lapb_dl_release_l2l3(l2, INDICATION);
	}

	static void l2_connected(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IsdnPacket *pPack=ArgToPack(arg);
		int pr=-1;

		if ( !get_PollFlag(l2, pPack) )
		{
			l2_mdl_error_ua(fi, event, arg);
			return;
		}
		//delete (pPack,l2->owner->GetInfra());//dev_kfree_skb(skb);
		pPack->Delete();

		if ( test_and_clear_flag( l2->m_flag.PEND_REL) )
        {
			l2_disconnect(fi, event, arg);
            return;
        }

		if ( test_and_clear_flag( l2->m_flag.L3_INIT) )
		{
			pr = DL_ESTABLISH | CONFIRM;
		}
		else if ( l2->vs != l2->va )
		{
			//skb_queue_purge(&l2->i_queue);
			l2->m_iQueue.Clear();
			pr = DL_ESTABLISH | INDICATION;
		}

		stop_t200(l2);

		l2->vr = 0;
		l2->vs = 0;
		l2->va = 0;
		l2->sow = 0;
		//FsmChangeState(fi, ST_L2_7);
		fi->ChangeState(ST_L2_7);
		//FsmAddTimer(&l2->t203, l2->T203, EV_L2_T203, NULL, 4);
		l2->t203.Set(l2->T203, EV_L2_T203, 0);

		//if (pr != -1)
		//l2->l2l3(pr, NULL);
		if ( pr == (DL_ESTABLISH | CONFIRM) )
        {
			l2->owner->GetUpIntf()->EstablishConf();
            l2->getStat().ActivateMoment();
        }
		if ( pr == (DL_ESTABLISH | INDICATION) )
        {
			l2->owner->GetUpIntf()->EstablishInd();
            l2->getStat().ActivateMoment();
        }

		if ( !l2->m_iQueue.IsEmpty() && cansend(l2) )
			//l2->l2l1(PH_PULL | REQUEST, NULL);
			L2Down(l2)->PullRequest();
	}

	static void l2_released(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IsdnPacket *pPack=ArgToPack(arg);

		if ( !get_PollFlag(l2, pPack) )
		{
			l2_mdl_error_ua(fi, event, arg);
			return;
		}
		//dev_kfree_skb(skb);
		//delete (pPack,l2->owner->GetInfra());
		pPack->Delete();

		stop_t200(l2);
		lapb_dl_release_l2l3(l2, CONFIRM);
		//FsmChangeState(fi, ST_L2_4);
		fi->ChangeState(ST_L2_4);
	}

	static void l2_reestablish(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IsdnPacket *pPack=ArgToPack(arg);

		if ( !get_PollFlagFree(l2, pPack) )
		{
			establishlink(fi);
			test_and_set_flag( l2->m_flag.L3_INIT);
		}
	}

	static void l2_st5_dm_release(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IsdnPacket *pPack = ArgToPack(arg);

		if ( get_PollFlagFree(l2, pPack) )
		{
			stop_t200(l2);
			if ( !test_flag( l2->m_flag.L3_INIT) )
				//skb_queue_purge(&l2->i_queue);
				l2->m_iQueue.Clear();
			if ( test_flag( l2->m_flag.LAPB) )
				//l2->l2l1(PH_DEACTIVATE | REQUEST, NULL);
				L2Down(l2)->DeactivateRequest();
			st5_dl_release_l2l3(l2);
			//FsmChangeState(fi, ST_L2_4);
			fi->ChangeState(ST_L2_4);
		}
	}

	static void l2_st6_dm_release(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IsdnPacket *pPack = ArgToPack(arg);

		if ( get_PollFlagFree(l2, pPack) )
		{
			stop_t200(l2);
			lapb_dl_release_l2l3(l2, CONFIRM);
			//FsmChangeState(fi, ST_L2_4);
			fi->ChangeState(ST_L2_4);
		}
	}

	static inline void enquiry_cr(/*struct PStack *st*/IsdnL2::Layer2 *l2, byte typ, byte cr, byte pf)
	{
		//struct sk_buff *skb;
		byte tmp[MAX_HEADER_LEN];//tmp - header пакета
		int i;

		i = sethdraddr(l2, tmp, cr);// записать в tmp i-byte инфы
		if ( test_flag( l2->m_flag.MOD128) )
		{
			tmp[i++] = typ;
			tmp[i++] = (l2->vr << 1) | (pf ? 1 : 0);
		}
		else
			tmp[i++] = (l2->vr << 5) | typ | (pf ? 0x10 : 0);
		/*if (!(skb = alloc_skb(i, GFP_ATOMIC))) {
			printk(KERN_WARNING "isdl2 can't alloc sbbuff for enquiry_cr\n");
			return;
		}
		memcpy(skb_put(skb, i), tmp, i);*/
		IsdnPacket *pPack = l2->owner->CreatePacket<IsdnPacket>();
		pPack->AddBack( std::vector<byte> (tmp,tmp+i) );
		enqueue_super(l2, pPack);
	}

	static inline void  enquiry_response(IsdnL2::Layer2 *l2)
	{
		if ( test_flag( l2->m_flag.OWN_BUSY) )
			enquiry_cr(l2, RNR, RSP, 1);
		else
			enquiry_cr(l2, RR, RSP, 1);
		test_and_clear_flag( l2->m_flag.ACK_PEND);
	}

	static inline void transmit_enquiry(IsdnL2::Layer2* l2)
	{
		if ( test_flag( l2->m_flag.OWN_BUSY) )
			enquiry_cr(l2, RNR, CMD, 1);
		else
			enquiry_cr(l2, RR, CMD, 1);
		test_and_clear_flag( l2->m_flag.ACK_PEND);
		start_t200(l2, 9);
	}


	static void nrerrorrecovery(FsmMachine *fi)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		sendToMErrorInd(pSelf,'J');

		establishlink(fi);
		test_and_clear_flag( l2->m_flag.L3_INIT);
	}

	static void invoke_retransmission(IsdnL2::Layer2 *l2, unsigned int nr)
	{

		unsigned int p1;
		//u_long flags;

		//spin_lock_irqsave(&l2->lock, flags);
		if ( l2->vs != nr )
		{
			while ( l2->vs != nr )
			{
				(l2->vs)--;
				if ( test_flag( l2->m_flag.MOD128) )
				{
					l2->vs %= 128;
					p1 = (l2->vs - l2->va) % 128;
				}
				else
				{
					l2->vs %= 8;
					p1 = (l2->vs - l2->va) % 8;
				}
				p1 = (p1 + l2->sow) % l2->GetMaxWindow();
				if ( test_flag( l2->m_flag.LAPB) )
					//st->l1.bcs->tx_cnt += l2->windowar[p1]->len + l2headersize(l2, 0);
					l2->BCStx_cntInc(l2->m_windowAr.at(p1)->GetLenData() + l2headersize(l2, 0));
				//skb_queue_head(&l2->i_queue, l2->windowar[p1]);
				l2->m_iQueue.AddFront( l2->m_windowAr.at(p1) );
				l2->m_windowAr.at(p1) = 0;
			}
			//spin_unlock_irqrestore(&l2->lock, flags);
			//l2->l2l1(PH_PULL | REQUEST, NULL);
			L2Down(l2)->PullRequest();
			return;
		}
		//spin_unlock_irqrestore(&l2->lock, flags);
	}    

	static void l2_st7_got_super(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IsdnPacket *pPack = ArgToPack(arg);
		int PollFlag, rsp, typ = RR;
		unsigned int nr;

		rsp = pPack->GetByIndex(0) & 0x2;
		if ( test_flag( l2->m_flag.ORIG) ) // if (test_flag( l2->m_flag.ORIG))
			rsp = !rsp;

		//skb_pull(skb, l2addrsize(l2));
		pPack->ClearFront(l2addrsize(l2));

		//u_char *data=new u_char[pPack->GetLenData()];
		//pPack->GetRawData(data);
		byte data0=pPack->GetByIndex(0);
		byte data1=pPack->GetByIndex(1);

        // определяется тип команды
		if ( IsRNR(data0, l2) )
		{
			set_peer_busy(l2);
			typ = RNR;
		}
		else
			clear_peer_busy(l2);

		if ( IsREJ(data0, l2) )
			typ = REJ;
        //----

        // номер последнего принятого той стороной?
		if ( test_flag( l2->m_flag.MOD128) )
		{
			PollFlag = (data1 & 0x1) == 0x1;
			nr = data1 >> 1;
		}
		else
		{
			PollFlag = (data0 & 0x10);
			nr = (data0 >> 5) & 0x7;
		}
        //-----
		
		pPack->Delete();


        // если требуется подтверждение
		if ( PollFlag )
		{
            // если это запрос
			if ( rsp )
				//pSelf->MErrorIndication('A');
				sendToMErrorInd(pSelf,'A');
			else
				enquiry_response(l2);
		}
		

		if ( legalnr(l2, nr) )
		{
			if ( typ == REJ )
			{
				setva(l2, nr);
				invoke_retransmission(l2, nr);
				stop_t200(l2);
				/*if (FsmAddTimer(&l2->t203, l2->T203,
						EV_L2_T203, NULL, 6))*/
				if ( l2->t203.Set(l2->T203,EV_L2_T203, 0) )
                {                    
                    ILoggable& log = l2->getLogSession();
                    log.getTraceOn();
                    if (log.getTraceOn())
                    {
                        log.DoLog("Restart T203 ST7 REJ", l2->getLogTags().timerEvents);
                    }
                }
				

			}
			else if ( (nr == l2->vs) && (typ == RR) )
			{
				setva(l2, nr);
				stop_t200(l2);
				/*FsmRestartTimer(&l2->t203, l2->T203,
						EV_L2_T203, NULL, 7);*/
				l2->t203.Restart(l2->T203, EV_L2_T203, 0);                
			}
			else if ( (l2->va != nr) || (typ == RNR) )
			{
				setva(l2, nr);
				if ( typ != RR ) /*FsmDelTimer(&l2->t203, 9);*/
					l2->t203.Deactivate();
				restart_t200(l2, 12);
			}
			if ( !l2->m_iQueue.IsEmpty() && (typ == RR) )
				//l2->l2l1(PH_PULL | REQUEST, NULL);
				L2Down(l2)->PullRequest();
		}
		else
			nrerrorrecovery(fi);
	}

	static void l2_feed_i_if_reest(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IsdnPacket *pPack=ArgToPack(arg);

		if ( test_flag( l2->m_flag.LAPB) )
			//st->l1.bcs->tx_cnt += skb->len + l2headersize(&st->l2, 0);//!!!bcs
			l2->BCStx_cntInc(pPack->GetLenData() + l2headersize(l2, 0));
		if ( !test_flag( l2->m_flag.L3_INIT) )
			//skb_queue_tail(&l2->i_queue, skb);
			l2->m_iQueue.AddBack(pPack);
		else
			//dev_kfree_skb(skb);
			//delete (pPack,l2->owner->GetInfra());
			pPack->Delete();
	}

	static void l2_feed_i_pull(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IsdnPacket *pPack=ArgToPack(arg);

		if ( test_flag( l2->m_flag.LAPB) )
			//st->l1.bcs->tx_cnt += skb->len + l2headersize(&st->l2, 0);//!!bcs
			l2->BCStx_cntInc(pPack->GetLenData() + l2headersize(l2, 0));
		//skb_queue_tail(&l2->i_queue, skb);
		l2->m_iQueue.AddBack(pPack);
		//l2->l2l1(PH_PULL | REQUEST, NULL);
		L2Down(l2)->PullRequest();
	}

	static void l2_feed_iqueue(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IsdnPacket *pPack=ArgToPack(arg);

		if ( test_flag( l2->m_flag.LAPB) )
			//st->l1.bcs->tx_cnt += skb->len + l2headersize(&st->l2, 0);//!!bcs
			l2->BCStx_cntInc(pPack->GetLenData() + l2headersize(l2, 0));
		//skb_queue_tail(&l2->i_queue, skb);
		l2->m_iQueue.AddBack(pPack);
	}

	static void l2_got_iframe(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IPacket *pIPack = static_cast<IPacket*>(arg);        
        l2->getStat().IPackRxInc();

		int PollFlag, ns, i;
		unsigned int nr;

		// extract ns/nr
		i = l2addrsize(l2);
        ESS_ASSERT(pIPack->GetLenData() > (i + 1) ); // guarantied by iframe_error
		if ( test_flag( l2->m_flag.MOD128) )
		{
			PollFlag = ((pIPack->GetByIndex(i+1) & 0x1) == 0x1);
			ns = pIPack->GetByIndex(i) >> 1;
			nr = (pIPack->GetByIndex(i+1) >> 1) & 0x7f;
		}
		else
		{
			PollFlag = (pIPack->GetByIndex(i) & 0x10);
			ns = (pIPack->GetByIndex(i) >> 1) & 0x7;
			nr = (pIPack->GetByIndex(i) >> 5) & 0x7;
		}

		if ( test_flag( l2->m_flag.OWN_BUSY) )
		{
			//dev_kfree_skb(skb);
			//delete (pIPack,l2->owner->GetInfra());
			pIPack->Delete();
			if ( PollFlag )	enquiry_response(l2);
		}
		else
		{

			if ( l2->vr == ns )
			{
				(l2->vr)++;

				if ( test_flag( l2->m_flag.MOD128) ) l2->vr %= 128;
				else l2->vr	%= 8;

				test_and_clear_flag( l2->m_flag.REJEXC);

				if ( PollFlag )	enquiry_response(l2);
				else test_and_set_flag( l2->m_flag.ACK_PEND);

				//skb_pull(skb, l2headersize(l2, 0));
				pIPack->ClearFront(l2headersize(l2, 0));
				//l2->l2l3(DL_DATA | INDICATION, skb);
				
                //l2->owner->GetUpIntf()->DataInd( /*static_cast<IPacket *>*/pIPack );
                l2->owner->GetUpIntf()->DataInd( /*static_cast<IPacket *>*/pIPack->getAsQVector() );
                pIPack->Delete();
			}
			else
			{
				/* n(s)!=v(r) */
				//dev_kfree_skb(skb);
				//delete (pIPack,l2->owner->GetInfra());
				pIPack->Delete();

				if ( test_and_set_flag( l2->m_flag.REJEXC) )
				{
					if ( PollFlag )	enquiry_response(l2);
				}
				else
				{
					enquiry_cr(l2, REJ, RSP, PollFlag);
					test_and_clear_flag( l2->m_flag.ACK_PEND);
				}
			}

		}


		if ( legalnr(l2, nr) )
		{
			if ( !test_flag( l2->m_flag.PEER_BUSY) && (fi->GetState() == ST_L2_7) )
			{
				if ( nr == l2->vs )
				{
					stop_t200(l2);
					/*FsmRestartTimer(&l2->t203, l2->T203,
							EV_L2_T203, NULL, 7);*/
					l2->t203.Restart(l2->T203, EV_L2_T203, 0);
				}
				else
					if ( nr != l2->va )	restart_t200(l2, 14);
			}

			setva(l2, nr);

		}
		else
		{
			nrerrorrecovery(fi);
			return;
		}

		if ( !l2->m_iQueue.IsEmpty() && (fi->GetState() == ST_L2_7) )
			L2Down(l2)->PullRequest();

		if ( test_and_clear_flag( l2->m_flag.ACK_PEND) )
			enquiry_cr(l2, RR, RSP, 0);
	}

	static void l2_got_tei(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		l2->tei = reinterpret_cast<long>(arg);

		if ( fi->GetState() == ST_L2_3 )
		{
			establishlink(fi);
			test_and_set_flag( l2->m_flag.L3_INIT);
		}
		else
			//FsmChangeState(fi, ST_L2_4);
			fi->ChangeState(ST_L2_4);
		if ( !l2->m_uiQueue.IsEmpty() )
			tx_ui(l2);
	}

	static void l2_st5_tout_200(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		if ( test_flag( l2->m_flag.LAPD) &&
			 test_flag( l2->m_flag.DCHAN_BUSY) )
		{
			//FsmAddTimer(&l2->t200, l2->T200, EV_L2_T200, NULL, 9);
			l2->t200.Set(l2->T200, EV_L2_T200, 0);
		}
		else if ( l2->rc == l2->N200 )
		{
			//FsmChangeState(fi, ST_L2_4);
			fi->ChangeState(ST_L2_4);
			test_and_clear_flag( l2->m_flag.T200_RUN);
			//skb_queue_purge(&l2->i_queue);
			l2->m_iQueue.Clear();
			sendToMErrorInd(pSelf,'G');
			if ( test_flag( l2->m_flag.LAPB) )
				//l2->l2l1(PH_DEACTIVATE | REQUEST, NULL);
				L2Down(l2)->DeactivateRequest();
			st5_dl_release_l2l3(l2);
		}
		else
		{
			l2->rc++;
			//FsmAddTimer(&l2->t200, l2->T200, EV_L2_T200, NULL, 9);
			l2->t200.Set(l2->T200, EV_L2_T200, 0);
			send_uframe(l2, (test_flag( l2->m_flag.MOD128) ? SABME : SABM)
						| 0x10, CMD);
		}
	}

	static void l2_st6_tout_200(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		if ( test_flag( l2->m_flag.LAPD) &&
			 test_flag( l2->m_flag.DCHAN_BUSY) )
		{
			//FsmAddTimer(&l2->t200, l2->T200, EV_L2_T200, NULL, 9);
			l2->t200.Set(l2->T200, EV_L2_T200, 0);
		}
		else if ( l2->rc == l2->N200 )
		{
			//FsmChangeState(fi, ST_L2_4);
			fi->ChangeState(ST_L2_4);
			test_and_clear_flag( l2->m_flag.T200_RUN);
			sendToMErrorInd(pSelf,'H');
			lapb_dl_release_l2l3(l2, CONFIRM);
		}
		else
		{
			l2->rc++;
			//FsmAddTimer(&l2->t200, l2->T200, EV_L2_T200,
			l2->t200.Set(l2->T200, EV_L2_T200,0);
			//NULL, 9);
			send_uframe(l2, DISC | 0x10, CMD);
		}
	}

	static void l2_st7_tout_200(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		if ( test_flag( l2->m_flag.LAPD) &&
			 test_flag( l2->m_flag.DCHAN_BUSY) )
		{
			//FsmAddTimer(&l2->t200, l2->T200, EV_L2_T200, NULL, 9);
			l2->t200.Set(l2->T200, EV_L2_T200, 0);
			return;
		}
		test_and_clear_flag( l2->m_flag.T200_RUN);
		l2->rc = 0;
		//FsmChangeState(fi, ST_L2_8);
		fi->ChangeState(ST_L2_8);

		transmit_enquiry(l2);
		l2->rc++;
	}

	static void l2_st8_tout_200(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		if ( test_flag( l2->m_flag.LAPD) &&
			 test_flag( l2->m_flag.DCHAN_BUSY) )
		{
			//FsmAddTimer(&l2->t200, l2->T200, EV_L2_T200, NULL, 9);
			l2->t200.Set(l2->T200, EV_L2_T200, 0);
			return;
		}
		test_and_clear_flag( l2->m_flag.T200_RUN);
		if ( l2->rc == l2->N200 )
		{
			sendToMErrorInd(pSelf,'I');
			establishlink(fi);
			test_and_clear_flag( l2->m_flag.L3_INIT);
		}
		else
		{
			transmit_enquiry(l2);
			l2->rc++;
		}
	}

	static void l2_st7_tout_203(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		if ( test_flag( l2->m_flag.LAPD) &&
			 test_flag( l2->m_flag.DCHAN_BUSY) )
		{
			//FsmAddTimer(&l2->t203, l2->T203, EV_L2_T203, NULL, 9);
			l2->t203.Set(l2->T203, EV_L2_T203, 0);
			return;
		}
		//FsmChangeState(fi, ST_L2_8);
		fi->ChangeState(ST_L2_8);
		transmit_enquiry(l2);
		l2->rc = 0;
	}

	static void l2_pull_iqueue(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		//struct sk_buff *skb, *oskb;
		IsdnPacket *pPack;

		dword p1;
		//u_long flags;

		if ( !cansend(l2) )
            return;        
            //l2->owner->GetUpIntf()->FreeWinInd();
		        
		//skb = skb_dequeue(&l2->i_queue);//извлечь из очереди
		//if (!skb)
		//	return;
		if ( l2->m_iQueue.IsEmpty() )
            return;
		pPack=l2->m_iQueue.Dequeue();

		//spin_lock_irqsave(&l2->lock, flags);
		if ( test_flag( l2->m_flag.MOD128) )
			p1 = (l2->vs - l2->va) % 128;
		else
			p1 = (l2->vs - l2->va) % 8;
		p1 = (p1 + l2->sow) % l2->GetMaxWindow();
		if ( l2->m_windowAr.at(p1) )
		{
			//printk(KERN_WARNING "isdnl2 try overwrite ack queue entry %d\n",p1);
            ILoggable& log = l2->getLogSession();
            log.getTraceOn();
            if (log.getTraceOn())
            {
                LogWriter(&log, l2->getLogTags().warning).Write() << "isdn_l2 try overwrite ack queue entry " << p1;
            }
			
			//delete(l2->m_windowar[p1],l2->owner->GetInfra());
			l2->m_windowAr.at(p1)->Delete();
            l2->m_windowAr.at(p1) = 0;
		}
		l2->m_windowAr.at(p1) = pPack->Clone(/*l2->owner->GetInfra()*/);//skb_clone(skb, GFP_ATOMIC);

		byte header[MAX_HEADER_LEN];
		int i = sethdraddr(l2, header, CMD );

		if ( test_flag( l2->m_flag.MOD128) )
		{
			header[i++] = l2->vs << 1;
			header[i++] = l2->vr << 1;
			l2->vs = (l2->vs + 1) % 128;
		}
		else
		{
			header[i++] = (l2->vr << 5) | (l2->vs << 1);
			l2->vs = (l2->vs + 1) % 8;
		}
		//spin_unlock_irqrestore(&l2->lock, flags);

		//p1 = skb->data - skb->head;//проверка влезет ли header в сегмент head
		//if (p1 >= i)
		//	memcpy(skb_push(skb, i), header, i);//дописать header перед блоком данных за счет емкости head!
		//else {							//а если не хватило места (аллок новый скб, запись хидера,слияние)==довыделение памяти
		//	printk(KERN_WARNING
		//	"isdl2 pull_iqueue skb header(%d/%d) too short\n", i, p1);
		//	oskb = skb;
		//	skb = alloc_skb(oskb->len + i, GFP_ATOMIC);
		//	memcpy(skb_put(skb, i), header, i);
		//	skb_copy_from_linear_data(oskb,
		//				  skb_put(skb, oskb->len), oskb->len);
		//	dev_kfree_skb(oskb);
		//}
		pPack->AddFront( std::vector<byte>(header,header+i) );

		//l2->l2l1(PH_PULL | INDICATION, skb);
        if ( l2->owner->getTraceOn() ) l2->LogPacketSend( pPack );
		L2Down(l2)->PullInd(pPack->getAsQVector());
        pPack->Delete();
        l2->getStat().IPackTxInc();
        l2->getStat().AllTxInc();

        /*if ( IsLogged(l2) )
        {
            LogWriter(Owner(l2)).Write() << "Req ";
        }*/

		test_and_clear_flag( l2->m_flag.ACK_PEND);
		if ( !test_and_set_flag( l2->m_flag.T200_RUN) )
		{
			//FsmDelTimer(&l2->t203, 13);
			l2->t203.Deactivate();
			//FsmAddTimer(&l2->t200, l2->T200, EV_L2_T200, NULL, 11);
			l2->t200.Set(l2->T200, EV_L2_T200, 0);
		}
		if ( !l2->m_iQueue.IsEmpty() && cansend(l2) )
			L2Down(l2)->PullRequest();
	}

	static void l2_st8_got_super(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IsdnPacket *pPack = ArgToPack(arg);
		int PollFlag, rsp, rnr = 0;
		unsigned int nr;


		rsp = pPack->GetByIndex(0) & 0x2;
		if ( test_flag( l2->m_flag.ORIG) )// if (test_flag( l2->m_flag.ORIG))
			rsp = !rsp;

		//skb_pull(skb, l2addrsize(l2));
		pPack->ClearFront(l2addrsize(l2));

		if ( IsRNR(pPack->GetByIndex(0), l2) )
		{
			set_peer_busy(l2);
			rnr = 1;
		}
		else
			clear_peer_busy(l2);

		if ( test_flag( l2->m_flag.MOD128) )
		{
			PollFlag = (pPack->GetByIndex(1) & 0x1) == 0x1;
			nr = pPack->GetByIndex(1) >> 1;
		}
		else
		{
			PollFlag = (pPack->GetByIndex(0) & 0x10);
			nr = (pPack->GetByIndex(0) >> 5) & 0x7;
		}
		//dev_kfree_skb(skb);
		//delete (pPack,l2->owner->GetInfra());
		pPack->Delete();

		if ( rsp && PollFlag )
		{
			if ( legalnr(l2, nr) )
			{
				if ( rnr )
				{
					restart_t200(l2, 15);
				}
				else
				{
					stop_t200(l2);
					/*FsmAddTimer(&l2->t203, l2->T203,
							EV_L2_T203, NULL, 5);*/
					l2->t203.Set(l2->T203, EV_L2_T203, 0);
					setva(l2, nr);
				}
				invoke_retransmission(l2, nr);
				//FsmChangeState(fi, ST_L2_7);
				fi->ChangeState(ST_L2_7);
				if ( !l2->m_iQueue.IsEmpty() && cansend(l2) )
					L2Down(l2)->PullRequest();
			}
			else
				nrerrorrecovery(fi);
		}
		else
		{
			if ( !rsp && PollFlag )
				enquiry_response(l2);
			if ( legalnr(l2, nr) )
			{
				setva(l2, nr);
			}
			else
				nrerrorrecovery(fi);
		}
	}

	static void l2_got_FRMR(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();
		//struct sk_buff *skb = reinterpret_cast<sk_buff*>(arg);
		//sk_buff *skb=ArgToBuf(arg);
		IsdnPacket *pPack=ArgToPack(arg);

		//skb_pull(skb, l2addrsize(l2) + 1);
		pPack->ClearFront(l2addrsize(l2) + 1);

		int data0=pPack->GetByIndex(0);
		if ( !(data0 & 1) || ((data0 & 3) == 1) ||		 /* I or S */
			 (IsUA(data0) && (fi->GetState() == ST_L2_7)) )
		{
			sendToMErrorInd(pSelf,'K');
			establishlink(fi);
			test_and_clear_flag( l2->m_flag.L3_INIT);
		}
		//dev_kfree_skb(skb);
		//delete (pPack,l2->owner->GetInfra());
		pPack->Delete();

	}

	static void l2_st24_tei_remove(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		//skb_queue_purge(&l2->ui_queue);
		l2->m_uiQueue.Clear();
		l2->tei = -1;
		//FsmChangeState(fi, ST_L2_1);
		fi->ChangeState(ST_L2_1);
	}

	static void l2_st3_tei_remove(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		//skb_queue_purge(&l2->ui_queue);
		l2->m_uiQueue.Clear();
		l2->tei = -1;
		//l2->l2l3(DL_RELEASE | INDICATION, NULL);
		l2->owner->GetUpIntf()->ReleaseInd();
        l2->getStat().DeactivateMoment();
		//FsmChangeState(fi, ST_L2_1);
		fi->ChangeState(ST_L2_1);
	}

	static void l2_st5_tei_remove(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		//skb_queue_purge(&l2->i_queue);
		l2->m_iQueue.Clear();
		//skb_queue_purge(&l2->ui_queue);
		l2->m_uiQueue.Clear();
		freewin(l2);
		l2->tei = -1;
		stop_t200(l2);
		st5_dl_release_l2l3(l2);
		//FsmChangeState(fi, ST_L2_1);
		fi->ChangeState(ST_L2_1);
	}

	static void l2_st6_tei_remove(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		//skb_queue_purge(&l2->ui_queue);
		l2->m_uiQueue.Clear();
		l2->tei = -1;
		stop_t200(l2);
		//l2->l2l3(DL_RELEASE | CONFIRM, NULL);
		l2->owner->GetUpIntf()->ReleaseConf();
        l2->getStat().DeactivateMoment();
		//FsmChangeState(fi, ST_L2_1);
		fi->ChangeState(ST_L2_1);
	}

	static void l2_tei_remove(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		//skb_queue_purge(&l2->i_queue);
		l2->m_iQueue.Clear();
		//skb_queue_purge(&l2->ui_queue);
		l2->m_uiQueue.Clear();
		freewin(l2);
		l2->tei = -1;
		stop_t200(l2);
		//FsmDelTimer(&l2->t203, 19);
		l2->t203.Deactivate();
		//l2->l2l3(DL_RELEASE | INDICATION, NULL);
		Owner(l2)->GetUpIntf()->EstablishInd();
        l2->getStat().ActivateMoment();
		//FsmChangeState(fi, ST_L2_1);
		fi->ChangeState(ST_L2_1);
	}

	static void l2_st14_persistent_da(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		//skb_queue_purge(&l2->i_queue);
		l2->m_iQueue.Clear();
		//skb_queue_purge(&l2->ui_queue);
		l2->m_uiQueue.Clear();

		if ( test_and_clear_flag( l2->m_flag.ESTAB_PEND) )
        {
			//l2->l2l3( DL_RELEASE | INDICATION, NULL);
			l2->owner->GetUpIntf()->ReleaseInd();
            l2->getStat().DeactivateMoment();
        }
	}

	static void l2_st5_persistent_da(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		//skb_queue_purge(&l2->i_queue);
		l2->m_iQueue.Clear();
		//skb_queue_purge(&l2->ui_queue);
		l2->m_uiQueue.Clear();
		freewin(l2);
		stop_t200(l2);
		st5_dl_release_l2l3(l2);
		//FsmChangeState(fi, ST_L2_4);
		fi->ChangeState(ST_L2_4);
	}

	static void l2_st6_persistent_da(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		//skb_queue_purge(&l2->ui_queue);
		l2->m_uiQueue.Clear();
		stop_t200(l2);
		//l2->l2l3( DL_RELEASE | INDICATION, NULL);
		l2->owner->GetUpIntf()->ReleaseInd();
        l2->getStat().DeactivateMoment();
		//FsmChangeState(fi, ST_L2_4);
		fi->ChangeState(ST_L2_4);
	}

	static void l2_persistent_da(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		//skb_queue_purge(&l2->i_queue);
		l2->m_iQueue.Clear();
		//skb_queue_purge(&l2->ui_queue);
		l2->m_uiQueue.Clear();
		freewin(l2);
		stop_t200(l2);
		//FsmDelTimer(&l2->t203, 19);
		l2->t203.Deactivate();
		//l2->l2l3( DL_RELEASE | INDICATION, NULL);
		l2->owner->GetUpIntf()->ReleaseInd();
        l2->getStat().DeactivateMoment();
		//FsmChangeState(fi, ST_L2_4);
		fi->ChangeState(ST_L2_4);
	}

	static void l2_set_own_busy(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		if ( !test_and_set_flag( l2->m_flag.OWN_BUSY) )
		{
			enquiry_cr(l2, RNR, RSP, 0);
			test_and_clear_flag( l2->m_flag.ACK_PEND);
		}
	}

	static void l2_clear_own_busy(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		if ( !test_and_clear_flag( l2->m_flag.OWN_BUSY) )
		{
			enquiry_cr(l2, RR, RSP, 0);
			test_and_clear_flag( l2->m_flag.ACK_PEND);
		}
	}

	static void l2_frame_error(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            

		//pSelf->MErrorIndication(*(reinterpret_cast<char*>(arg)));
        int err = reinterpret_cast<int>(arg);
		sendToMErrorInd(pSelf,(static_cast<char>(err)));
	}

	static void l2_frame_error_reest(FsmMachine *fi, int event, void *arg)
	{
		//struct PStack *st = reinterpret_cast<PStack*>(fi->userdata);
		IsdnL2 *pSelf = GetSelf(fi);            
		IsdnL2::Layer2 *l2 = pSelf->getL2();

		//pSelf->MErrorIndication(*(reinterpret_cast<char*>(arg)));
        int err = reinterpret_cast<int>(arg);
		sendToMErrorInd(pSelf,(static_cast<char>(err)));
		establishlink(fi);
		test_and_clear_flag( l2->m_flag.L3_INIT);
	}

	static struct FsmNode L2FnList[]  =
	{
		{ST_L2_1, EV_L2_DL_ESTABLISH_REQ, l2_mdl_assign},
		{ST_L2_2, EV_L2_DL_ESTABLISH_REQ, l2_go_st3},
		{ST_L2_4, EV_L2_DL_ESTABLISH_REQ, l2_establish},
		{ST_L2_5, EV_L2_DL_ESTABLISH_REQ, l2_discard_i_setl3},
		{ST_L2_7, EV_L2_DL_ESTABLISH_REQ, l2_l3_reestablish},
		{ST_L2_8, EV_L2_DL_ESTABLISH_REQ, l2_l3_reestablish},
		{ST_L2_4, EV_L2_DL_RELEASE_REQ, l2_release},
		{ST_L2_5, EV_L2_DL_RELEASE_REQ, l2_pend_rel},
		{ST_L2_7, EV_L2_DL_RELEASE_REQ, l2_disconnect},
		{ST_L2_8, EV_L2_DL_RELEASE_REQ, l2_disconnect},
		{ST_L2_5, EV_L2_DL_DATA, l2_feed_i_if_reest},
		{ST_L2_7, EV_L2_DL_DATA, l2_feed_i_pull},
		{ST_L2_8, EV_L2_DL_DATA, l2_feed_iqueue},
		{ST_L2_1, EV_L2_DL_UNIT_DATA, l2_queue_ui_assign},
		{ST_L2_2, EV_L2_DL_UNIT_DATA, l2_queue_ui},
		{ST_L2_3, EV_L2_DL_UNIT_DATA, l2_queue_ui},
		{ST_L2_4, EV_L2_DL_UNIT_DATA, l2_send_ui},
		{ST_L2_5, EV_L2_DL_UNIT_DATA, l2_send_ui},
		{ST_L2_6, EV_L2_DL_UNIT_DATA, l2_send_ui},
		{ST_L2_7, EV_L2_DL_UNIT_DATA, l2_send_ui},
		{ST_L2_8, EV_L2_DL_UNIT_DATA, l2_send_ui},
		{ST_L2_1, EV_L2_MDL_ASSIGN, l2_got_tei},
		{ST_L2_2, EV_L2_MDL_ASSIGN, l2_got_tei},
		{ST_L2_3, EV_L2_MDL_ASSIGN, l2_got_tei},
		{ST_L2_2, EV_L2_MDL_ERROR, l2_st24_tei_remove},
		{ST_L2_3, EV_L2_MDL_ERROR, l2_st3_tei_remove},
		{ST_L2_4, EV_L2_MDL_REMOVE, l2_st24_tei_remove},
		{ST_L2_5, EV_L2_MDL_REMOVE, l2_st5_tei_remove},
		{ST_L2_6, EV_L2_MDL_REMOVE, l2_st6_tei_remove},
		{ST_L2_7, EV_L2_MDL_REMOVE, l2_tei_remove},
		{ST_L2_8, EV_L2_MDL_REMOVE, l2_tei_remove},
		{ST_L2_4, EV_L2_SABME, l2_start_multi},
		{ST_L2_5, EV_L2_SABME, l2_send_UA},
		{ST_L2_6, EV_L2_SABME, l2_send_DM},
		{ST_L2_7, EV_L2_SABME, l2_restart_multi},
		{ST_L2_8, EV_L2_SABME, l2_restart_multi},
		{ST_L2_4, EV_L2_DISC, l2_send_DM},
		{ST_L2_5, EV_L2_DISC, l2_send_DM},
		{ST_L2_6, EV_L2_DISC, l2_send_UA},
		{ST_L2_7, EV_L2_DISC, l2_stop_multi},
		{ST_L2_8, EV_L2_DISC, l2_stop_multi},
		{ST_L2_4, EV_L2_UA, l2_mdl_error_ua},
		{ST_L2_5, EV_L2_UA, l2_connected},
		{ST_L2_6, EV_L2_UA, l2_released},
		{ST_L2_7, EV_L2_UA, l2_mdl_error_ua},
		{ST_L2_8, EV_L2_UA, l2_mdl_error_ua},
		{ST_L2_4, EV_L2_DM, l2_reestablish},
		{ST_L2_5, EV_L2_DM, l2_st5_dm_release},
		{ST_L2_6, EV_L2_DM, l2_st6_dm_release},
		{ST_L2_7, EV_L2_DM, l2_mdl_error_dm},
		{ST_L2_8, EV_L2_DM, l2_st8_mdl_error_dm},
		{ST_L2_1, EV_L2_UI, l2_got_ui},
		{ST_L2_2, EV_L2_UI, l2_got_ui},
		{ST_L2_3, EV_L2_UI, l2_got_ui},
		{ST_L2_4, EV_L2_UI, l2_got_ui},
		{ST_L2_5, EV_L2_UI, l2_got_ui},
		{ST_L2_6, EV_L2_UI, l2_got_ui},
		{ST_L2_7, EV_L2_UI, l2_got_ui},
		{ST_L2_8, EV_L2_UI, l2_got_ui},
		{ST_L2_7, EV_L2_FRMR, l2_got_FRMR},
		{ST_L2_8, EV_L2_FRMR, l2_got_FRMR},
		{ST_L2_7, EV_L2_SUPER, l2_st7_got_super},
		{ST_L2_8, EV_L2_SUPER, l2_st8_got_super},
		{ST_L2_7, EV_L2_I, l2_got_iframe},
		{ST_L2_8, EV_L2_I, l2_got_iframe},
		{ST_L2_5, EV_L2_T200, l2_st5_tout_200},
		{ST_L2_6, EV_L2_T200, l2_st6_tout_200},
		{ST_L2_7, EV_L2_T200, l2_st7_tout_200},
		{ST_L2_8, EV_L2_T200, l2_st8_tout_200},
		{ST_L2_7, EV_L2_T203, l2_st7_tout_203},
		{ST_L2_7, EV_L2_ACK_PULL, l2_pull_iqueue},
		{ST_L2_7, EV_L2_SET_OWN_BUSY, l2_set_own_busy},
		{ST_L2_8, EV_L2_SET_OWN_BUSY, l2_set_own_busy},
		{ST_L2_7, EV_L2_CLEAR_OWN_BUSY, l2_clear_own_busy},
		{ST_L2_8, EV_L2_CLEAR_OWN_BUSY, l2_clear_own_busy},
		{ST_L2_4, EV_L2_FRAME_ERROR, l2_frame_error},
		{ST_L2_5, EV_L2_FRAME_ERROR, l2_frame_error},
		{ST_L2_6, EV_L2_FRAME_ERROR, l2_frame_error},
		{ST_L2_7, EV_L2_FRAME_ERROR, l2_frame_error_reest},
		{ST_L2_8, EV_L2_FRAME_ERROR, l2_frame_error_reest},
		{ST_L2_1, EV_L1_DEACTIVATE, l2_st14_persistent_da},
		{ST_L2_2, EV_L1_DEACTIVATE, l2_st24_tei_remove},
		{ST_L2_3, EV_L1_DEACTIVATE, l2_st3_tei_remove},
		{ST_L2_4, EV_L1_DEACTIVATE, l2_st14_persistent_da},
		{ST_L2_5, EV_L1_DEACTIVATE, l2_st5_persistent_da},
		{ST_L2_6, EV_L1_DEACTIVATE, l2_st6_persistent_da},
		{ST_L2_7, EV_L1_DEACTIVATE, l2_persistent_da},
		{ST_L2_8, EV_L1_DEACTIVATE, l2_persistent_da},
	};

#define L2_FN_COUNT (sizeof(L2FnList)/sizeof(struct FsmNode))
//---------------------------------------------------------------------------------------
	

	void
	releasestack_isdnl2(IsdnL2::Layer2* l2)
	{
		//FsmDelTimer(&l2->t200, 21);
		l2->t200.Deactivate();
		//FsmDelTimer(&l2->t203, 16);
		l2->t203.Deactivate();
		//skb_queue_purge(&l2->i_queue);
		l2->m_iQueue.Clear();
		//skb_queue_purge(&l2->ui_queue);
		l2->m_uiQueue.Clear();
		ReleaseWin(l2);
	}


	//-------------------------------------
	IsdnL2::L2::L2(const shared_ptr<const L2Profile> prof, IsdnL2* own):
	pl2m(own->CreateFsm_l2() ),
	t200(pl2m),t203(pl2m),owner(own),
	m_TeiManage(own,owner->GetInfra()),	
	m_windowAr(8, static_cast<IsdnPacket*>(0)) //InitWin
	{
		//-----взято из init_d_st
		//TUT_ASSERT(LAPD_flag  && "LAPB-mode not supported in current implementation.");
		T200 = 1000; // 1000
		N200 = 3; //try 3 times
		T203 = 10000; // 10000; // 10000
		//tei = -1;
		sap = 0;
		maxlen = MAX_DFRAME_LEN;//	260 - define in hisax


		m_flag.LAPB = 0; // =1 не поддерживается в тек.версии
		m_flag.LAPD = 1; // =1 2х байтовое адр. поле, поведение соответсв LAPD
		m_flag.ORIG = 0;  //  на NT стороне true (управляет инверсией C/R - bit)
		m_flag.MOD128 = 1; // метод инкремента нумерации пакетов (==0 инкремент по модулю 8, исп при LAPB)
		m_flag.PEND_REL = 0;
		m_flag.L3_INIT = 0;	 // =1  после EstablishReq от 3ур. В завис от этого 3ур передается conformation/indication
		m_flag.T200_RUN = 0;
		m_flag.ACK_PEND = 0;
		m_flag.REJEXC = 0;
		m_flag.OWN_BUSY = 0;
		m_flag.PEER_BUSY = 0;
		m_flag.DCHAN_BUSY = 0;
		m_flag.L1_ACTIV = 0;
		m_flag.ESTAB_PEND = 0;
		m_flag.PTP = 0;
		m_flag.FIXED_TEI = 0; // =1 - запрет сервиса назначения tei. Назначающая сторона(NT) должна иметь 0. TEside - непонятно
		m_flag.L2BLOCK = 0;
		
		prof->Set(owner);

		vs = 0; va = 0; vr = 0; sow = 0; rc = 0; // ?при инициализации L2 не инитилось
		tx_cnt=0;
	}

	void IsdnL2::L2::ChangeMaxWindow(int newSize)
	{
		m_windowAr.clear();
		m_windowAr.resize(newSize, 0);
		
	}

	void IsdnL2::L2::SetST_L2_4() { pl2m->ChangeState(ST_L2_4);}

	void IsdnL2::L2::SetST_L2_1() { pl2m->ChangeState(ST_L2_1);}

	bool IsdnL2::L2::IsST_L2_1() {return(pl2m->GetState() == ST_L2_1);}

	bool IsdnL2::L2::IsST_L2_2() {return(pl2m->GetState() == ST_L2_2);}

	bool IsdnL2::L2::IsST_L2_3() {return(pl2m->GetState() == ST_L2_3);}

	bool IsdnL2::L2::IsST_L2_4() {return(pl2m->GetState() == ST_L2_4);}

	bool IsdnL2::L2::IsST_L2_5() {return(pl2m->GetState() == ST_L2_5);}

	bool IsdnL2::L2::IsST_L2_6() {return(pl2m->GetState() == ST_L2_6);}

	bool IsdnL2::L2::IsST_L2_7() {return(pl2m->GetState() == ST_L2_7);}

	bool IsdnL2::L2::IsST_L2_8() {return(pl2m->GetState() == ST_L2_8);}
	//-------------------------------------

	IsdnL2::L2::~L2()
	{
		owner->DeleteFsm_l2();
		std::for_each(m_windowAr.begin(),m_windowAr.end(),DelPack);        
	}

	//-------------------------------------

	FsmMachine* IsdnL2::CreateFsm_l2()
	{
		FsmMachine* pFsm = new FsmMachine(L2FnList, L2_FN_COUNT, L2_STATE_COUNT, L2_EVENT_COUNT, 
										  strL2Event, strL2State, m_pInfra, this,  this , 0 , "L2Machine");

		return pFsm;
	}

	void IsdnL2::DeleteFsm_l2()
	{delete m_l2obj.pl2m;}

//-------------------------------------------------------------------------------------------------

	dword IsdnL2::L2::Random_ri()
	{
		unsigned int x;

		owner->GetRandomBytes(&x, sizeof(x));

		return(x & 0xffff);
	}

	//---------------------------------------------------------------------------------------------------------

	void IsdnL2::L2::DatIndTei(IsdnPacket* pPacket)
	{
		int mt;

		if ( pPacket->GetLenData() < l2addrsize(this) + 1)  // <3
		{
			if ( owner->getTraceOn() )
			{
				LogWriter(owner).Write() << "-Short mgr frame " << pPacket->GetLenData();
			}
			goto del_ret;           
		}

		//------------------

		if ( (pPacket->GetByIndex(0) != (TEI_SAPI << 2) ) || //((63 << 2) | 2)
			 (pPacket->GetByIndex(1) != ((GROUP_TEI << 1) | 1)) ) //((127 << 1) | 1)
		{
			if ( owner->getTraceOn() )
			{
				LogWriter(owner, getLogTags().frameErrorInfo).Write() << "Wrong mgr sapi/tei: " << pPacket->GetByIndex(0) << "/"
				<< pPacket->GetByIndex(1);
			}
			goto del_ret;
		}

		//------------------

		if ( (pPacket->GetByIndex(2) & 0xef) != UI )
		{
			if ( owner->getTraceOn() )
			{
				LogWriter(owner, getLogTags().frameErrorInfo).Write() 
                    << "-Mgr frame is not ui " << pPacket->GetByIndex(2);
			}
			goto del_ret;
		}

		//------------------

		//skb_pull(skb, 3);
		pPacket->ClearFront(3);

		if ( pPacket->GetLenData() < 5 )
		{
			if ( owner->getTraceOn() )
			{
				LogWriter(owner, getLogTags().frameErrorInfo).Write() 
                    << "Short mgr frame " << pPacket->GetLenData();
			}
			goto del_ret;
		}

		//------------------

		if ( pPacket->GetByIndex(0) != TEI_ENTITY_ID )
		{
			/* wrong management entity identifier, ignore */
			if ( owner->getTraceOn() )
			{
				LogWriter(owner, getLogTags().frameErrorInfo).Write() 
                    << "tei handler wrong entity id " << pPacket->GetByIndex(0);
			}
			goto del_ret;
		}

		//------------------

		mt = pPacket->GetByIndex(3);
		switch ( mt )
		{
		case(ID_REQUEST):/*&& test_flag FLG_LAPD_NET*/
			m_TeiManage.tei_m->Event(EV_ASSIGN_REQ, pPacket);
			break;
		case(ID_ASSIGNED):
			m_TeiManage.tei_m->Event(EV_ASSIGN, pPacket);
			break;
		case(ID_DENIED):
			m_TeiManage.tei_m->Event(EV_DENIED, pPacket);
			break;
		case(ID_CHK_REQ):
			m_TeiManage.tei_m->Event(EV_CHKREQ, pPacket);
			break;
		case(ID_REMOVE):
			m_TeiManage.tei_m->Event(EV_REMOVE, pPacket);
			break;
		default:
			if ( owner->getTraceOn() )
			{
				LogWriter(owner, getLogTags().frameErrorInfo).Write() << "tei handler wrong mt " << mt;
			}
		}//end switch

		del_ret:
		pPacket->Delete();
	}
//-------------------------------------------------------------------------------------------------
	void IsdnL2::L2::DatIndOther(IsdnPacket* pPacket)
	{
		const int offset = l2addrsize(this);//u_char *datap = skb->data;		
		int ret = 1;
		int c = 0;

        if (pPacket->GetLenData() <= offset)
        {
            c = 'N';
            goto _end_case;            
        }
		
		//start case
		if ( !(pPacket->GetByIndex(offset) & 1) )
		{	/* I-Frame */
			if ( !(c = iframe_error(this, pPacket)) )
				ret = pl2m->Event(EV_L2_I, pPacket);			
			goto _end_case;
		}

		//------next case

		if ( IsSFrame(pPacket->GetByIndex(offset), this) )
		{	/* S-Frame */
			if ( !(c = super_error(this, pPacket)) )				
				ret = pl2m->Event(EV_L2_SUPER, pPacket);
			goto _end_case;
		}

		//------next case

		if ( IsUI(pPacket->GetByIndex(offset)) )
		{
			if ( !(c = UI_error(this, pPacket)) )				
				ret = pl2m->Event(EV_L2_UI, pPacket);
			goto _end_case;
		}

		//------next case

		if ( IsSABME(pPacket->GetByIndex(offset), this) )
		{
			if ( !(c = unnum_error(this, pPacket, CMD)) )				
				ret = pl2m->Event(EV_L2_SABME, pPacket);
			goto _end_case;
		}

		//------next case

		if ( IsUA(pPacket->GetByIndex(offset)) )
		{
			if ( !(c = unnum_error(this, pPacket, RSP)) )				
				ret = pl2m->Event(EV_L2_UA, pPacket);
			goto _end_case;
		}

		//------next case

		if ( IsDISC(pPacket->GetByIndex(offset)) )
		{
			if ( !(c = unnum_error(this, pPacket, CMD)) )				
				ret = pl2m->Event( EV_L2_DISC, pPacket);
			goto _end_case;
		}

		//------next case

		if ( IsDM(pPacket->GetByIndex(offset)) )
		{
			if ( !(c = unnum_error(this, pPacket, RSP)) )				
				ret = pl2m->Event(EV_L2_DM, pPacket);
			goto _end_case;
		}

		//------next case

		if ( IsFRMR(pPacket->GetByIndex(offset)) )
		{
			if ( !(c = FRMR_error(this,pPacket)) )				
				ret = pl2m->Event(EV_L2_FRMR, pPacket);
			goto _end_case;
		}

		//---------default

		{
            c = 'L';	
            goto _end_case;
		}

		_end_case:

		if ( c )
		{
			pPacket->Delete();
			//FsmEvent(&m_l2obj.l2m, EV_L2_FRAME_ERROR, (void *)(long)c);
			pl2m->Event(EV_L2_FRAME_ERROR, (void *)(int)c);
			ret = 0;
            getStat().BadPackInc();
		}
		if ( ret )
			pPacket->Delete();
	}

	//-------------------------------------------------------------------------------------------------

	void IsdnL2::L2::EstabReq()
	{
		if ( test_flag(m_flag.L1_ACTIV) )
		{
			if ( test_flag(m_flag.LAPD) || test_flag(m_flag.ORIG) )
			{
				pl2m->Event(EV_L2_DL_ESTABLISH_REQ, 0);
				//FsmEvent( &m_l2obj.l2m, EV_L2_DL_ESTABLISH_REQ, NULL);
			}
		}
		else
		{
			if ( test_flag(m_flag.LAPD) || test_flag(m_flag.ORIG) )
			{
				test_and_set_flag(m_flag.ESTAB_PEND);
			}
			//owner->GetUpIntf()->EstablishConf();	
			//l2->l2l1(PH_ACTIVATE, NULL);
			owner->GetDownIntf()->ActivateRequest();
		}
	}
//-------------------------------------------------------------------------------------------------
	void IsdnL2::L2::RelReq()
	{
		if ( test_flag(m_flag.LAPB) )
		{
			//l2->l2l1(PH_DEACTIVATE, NULL);
			owner->GetDownIntf()->DeactivateRequest();
		}
		//FsmEvent(&m_l2obj.l2m, EV_L2_DL_RELEASE_REQ, NULL);
		pl2m->Event(EV_L2_DL_RELEASE_REQ, 0);
	}
//-------------------------------------------------------------------------------------------------
	void IsdnL2::L2::DatReq(ISDN::IPacket *pPacket)
	{
		//if (  FsmEvent( &m_l2obj.l2m, EV_L2_DL_DATA, pPacket )  ) 
		if ( pl2m->Event(EV_L2_DL_DATA, pPacket) )
			//delete (pPacket,GetInfra());
			pPacket->Delete();
	}
//-------------------------------------------------------------------------------------------------

	void IsdnL2::L2::UDatReq(UPacket *pPacket)
	{
		//if (  FsmEvent( &m_l2obj.l2m, EV_L2_DL_UNIT_DATA, pPacket )  ) 
		if ( pl2m->Event(EV_L2_DL_UNIT_DATA, pPacket) )
			//delete (pPacket,GetInfra() );
			pPacket->Delete();
	}
//-------------------------------------------------------------------------------------------------
	void IsdnL2::L2::MAsgnReq(int arg)
	{
		//FsmEvent(&m_l2obj.l2m, EV_L2_MDL_ASSIGN, arg); // l2_got_tei
		pl2m->Event(EV_L2_MDL_ASSIGN,(void*)(long) arg); // l2_got_tei , send SABME
	}
//-------------------------------------------------------------------------------------------------
	void IsdnL2::L2::MRmovReq()
	{
		pl2m->Event(EV_L2_MDL_REMOVE, 0);
		//FsmEvent(&m_l2obj.l2m, EV_L2_MDL_REMOVE,NULL);
	}
//-------------------------------------------------------------------------------------------------
	void IsdnL2::L2::MErrResp()
	{
		pl2m->Event(EV_L2_MDL_ERROR, 0);
		//FsmEvent(&m_l2obj.l2m, EV_L2_MDL_ERROR, NULL);
	}
//-------------------------------------------------------------------------------------------------
	void IsdnL2::L2::DatInd(IsdnPacket *pPacket)
	{
        getStat().AllRxInc();
        if ( pPacket->GetLenData() <= l2addrsize(this) )
        {
            pl2m->Event(EV_L2_FRAME_ERROR, (void *) 'N');
            pPacket->Delete();
            getStat().BadPackInc();
            return;
        }
		int sapi;
		sapi = pPacket->GetByIndex(0) >> 2;	// m_pBuff->data[0] >> 2;
		//sk_buff* skb=pPacket->getRawPacket();

		if ( sapi == TEI_SAPI )
		{
			if ( test_flag(m_flag.FIXED_TEI) )
			{
				pPacket->Delete();
				return;
			}
			DatIndTei(pPacket);
		}
		else
		{			
			DatIndOther(pPacket);//l1l2
		}

	}
//-------------------------------------------------------------------------------------------------

    void IsdnL2::L2::LogPacketSend(IsdnPacket* pPack) const 
    {                
        owner->SimpleLog("Send Packet to Driver", owner->m_logKinds.general);
        owner->m_PV.Draw(pPack, m_isUserSide/*m_pRole->IsUserSide()*/);
    }

	void IsdnL2::L2::ActivInd()
	{
		test_and_set_flag(m_flag.L1_ACTIV);
		if ( test_and_clear_flag(m_flag.ESTAB_PEND) )
			pl2m->Event(EV_L2_DL_ESTABLISH_REQ, 0);
		//FsmEvent(&m_l2obj.l2m, EV_L2_DL_ESTABLISH_REQ, NULL);
	}

	void IsdnL2::L2::DeactConf()
	{
		test_and_clear_flag(m_flag.L1_ACTIV);
		pl2m->Event(EV_L1_DEACTIVATE, 0);
		//FsmEvent(&m_l2obj.l2m, EV_L1_DEACTIVATE, NULL);
	}

	void IsdnL2::L2::PullCnf()
	{
		pl2m->Event(EV_L2_ACK_PULL, 0);
		//FsmEvent(&m_l2obj.l2m, EV_L2_ACK_PULL,NULL );
	}

	void IsdnL2::L2::PausInd()
	{
		test_and_set_flag(m_flag.DCHAN_BUSY);
	}

	void IsdnL2::L2::PausConf()
	{
		test_and_clear_flag(m_flag.DCHAN_BUSY);
	}


	void IsdnL2::L2::tei_l2tei( int pr, void *arg)
	{
		//struct IsdnCardState *cs;

		if ( test_flag(m_flag.FIXED_TEI) )
		{
			if ( pr == (MDL_ASSIGN | INDICATION) )
			{
				if ( owner->getTraceOn() )
				{
					LogWriter(owner, owner->getLogTags().tei).Write() << "fixed assign tei " << tei;
				}


				//st->l3.l3l2(st, MDL_ASSIGN | REQUEST, (void *) (long) st->l2.tei);
				owner->MAssignReq(/*(void *) (long)*/tei);
				//cs = (struct IsdnCardState *) st->l1.hardware;
				//cs->cardmsg(cs, MDL_ASSIGN | REQUEST, NULL);
			}
			return;
		}
		switch ( pr )
		{
		case (MDL_ASSIGN | INDICATION):
			//FsmEvent(&l2->m_TeiManage.tei_m, EV_IDREQ, arg);
			m_TeiManage.tei_m->Event(EV_IDREQ, arg);
			break;
		case (MDL_ERROR | REQUEST):
			//FsmEvent(&l2->m_TeiManage.tei_m, EV_VERIFY, arg);
			m_TeiManage.tei_m->Event(EV_VERIFY, arg);
			break;
		default:
			break;
		}
	}
};// ISDN::

