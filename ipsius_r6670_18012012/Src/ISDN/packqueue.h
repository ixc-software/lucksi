#ifndef PACKQUEUE_H
#define PACKQUEUE_H

#include "stdafx.h"
#include "isdnpack.h"

namespace ISDN
{
	class PacketQueue//L2 is Owner
    {
        
    public:

		~PacketQueue() {Clear();}

        void AddFront(IsdnPacket* pPack);//skb_queue_head ѕроверить!!! откуда делают skb_dequeue() c head or tail????
		
        void AddBack (IsdnPacket* pPack);//skb_queue_tail
		
		IsdnPacket* Dequeue ();//sk_dequeue
		
        void Clear();//skb_purge
		
        bool IsEmpty();//skb_purgen()
		
	private:

        static void DelPack(IsdnPacket* pPack)
        {
            pPack->SetQueue(0); pPack->Delete();
        }//используетс€ for_each

		std::list<IsdnPacket*> m_queue;
    };
}//ISDN

#endif
