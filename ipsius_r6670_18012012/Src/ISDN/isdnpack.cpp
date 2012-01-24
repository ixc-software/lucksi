
#include "stdafx.h"
#include "isdnpack.h"
#include "Utils/ErrorsSubsystem.h"

namespace ISDN
{
	IsdnPacket::~IsdnPacket()
	{
        ESS_ASSERT(!m_pIsInQueue  &&  "Call Destructor of packet in queue");
	}

	bool IsdnPacket::AddFront(const std::vector<byte> &data) //memcpy(skb_push(skb, i), header, i);
	{
        AssertActive();

		if (data.empty()) return false; //Или исключение или ассерт??

		std::copy(  data.begin(), data.end(), std::inserter( m_Data,m_Data.begin() )  );		
		return true;
	}

	bool IsdnPacket::AddBack(std::vector<byte>const &data)  //memcpy(skb_put(skb, i), header, i);
	{
        AssertActive();

		if (data.empty()) return false;   //Или исключение или ассерт?

		std::copy(data.begin(), data.end(), std::inserter(m_Data, m_Data.end()));
		return true;
	}

	void IsdnPacket::ClearFront(int len)    //skb_pull- убрать len-байт от начала
	{
        AssertActive();

        ESS_ASSERT( len <= m_Data.size() );

		while (len--)
		{
            m_Data.pop_front();
        }
	}

    void IsdnPacket::Delete()
    {
        AssertActive();
        //delete (this, m_pAllocator);
        IsdnPacket::operator delete(this, m_pAllocator);
    }
}//namespace ISDN

