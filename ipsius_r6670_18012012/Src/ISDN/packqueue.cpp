
#include "stdafx.h"
#include"packqueue.h"
#include "Utils/ErrorsSubsystem.h"

namespace ISDN
{

	void PacketQueue::AddFront(IsdnPacket* pPack)//skb_queue_head ���������!!! ������ ������ skb_dequeue() c head or tail????
	{
		ESS_ASSERT( !pPack->IsInQueue() && "Try add Packet alredy in queue");
        pPack->SetQueue(this);
		m_queue.push_front(pPack);
	}

	void PacketQueue::AddBack (IsdnPacket* pPack)//skb_queue_tail
	{
		ESS_ASSERT( !pPack->IsInQueue() && "Try add Packet alredy in queue");
        pPack->SetQueue(this);
		m_queue.push_back(pPack);
	}

    // ������� �� �������
    IsdnPacket* PacketQueue::Dequeue ()
    {
        ESS_ASSERT( !m_queue.empty() );//��� ����������?
        IsdnPacket* pRet=m_queue.front();
        m_queue.pop_front();
        pRet->SetQueue(0);//�������� ����� ��� �� ������������� �������
        return pRet;
    }

    void PacketQueue::Clear()//skb_purge
    {
	    std::for_each(m_queue.begin(),m_queue.end(),DelPack);
	    m_queue.clear();
    }

    bool PacketQueue::IsEmpty()//skb_purgen()
    {
	    return m_queue.empty();
    }


}; // namespace ISDN

