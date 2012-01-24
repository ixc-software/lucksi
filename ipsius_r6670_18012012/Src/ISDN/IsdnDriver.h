////Образец
////Удалить из проэкта
//
//#ifndef ISDNDRIVER_H
//#define ISDNDRIVER_H
//
//#include"hisax.h"
//
//class IsdnDriver
//{
//	
//	PStack* m_pStack;
//	
//	void L1L2SendMsg(int event, void *arg)
//	{
//		m_pStack->l1.l1l2(m_pStack, event, arg);
//	}
//	
//	
//protected:
//	
//	// обработчик входящих сообщений
//	virtual void OnDataRequest(DssPacket &packet) = 0;
//	virtual void OnActivateRequest() = 0;
//	
//	// отправка сообщений к L2
//	void DataIndication(DssPacket &packet)
//	{
//		L1L2SendMsg((PH_DATA | INDICATION), packet.CreateRawPacket());
//	}
//  		
//	
//public:
//	
//	IsdnDriver(PStack* pStack) : m_pStack(pStack)
//	{
//	}
//	
//	static void L2L1MsgHandler(struct PStack *, int event, void *arg)  
//	{
//     IsdnDriver *pDrv = (IsdnDriver*)pStack->l1.hardware;
//     
//     switch (event)
//     {
//     	  case (PH_DATA | REQUEST):
//     	  	pDrv->OnDataRequest(DssPacket((sk_buff*)arg));
//     	  	break;
//     	  	
//     	   case (PH_ACTIVATE | REQUEST):
//     	  	pDrv->OnActivateRequest();     	   	
//     	   	break;
//     	   	
//     	   // ...
//     }
//	}	
//	
//};
//
//
//class DssPacket
//{
//public:
//		
//		Packet(sk_buff *pBuff);
//		
//		sk_buff* CreateRawPacket();  // копия через new
//		
//		// ...
//}
//
//// ---------------------------
//
//
///*
//     Вопрос создания и инициализации стека, подшивание к нему драйвера
//
//*/
//
//void InitStack(struct PStack *pStack, IsdnDriver *pDrv)
//{
//	pStack->l1.hardware = pDrv;
//	
//	pStack->l2.l2l1 = IsdnDriver::L2L1MsgHandler;
//	
//	// ...
//}
//
//
//#endif
