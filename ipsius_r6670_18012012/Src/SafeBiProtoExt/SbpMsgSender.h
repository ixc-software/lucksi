#ifndef _SBP_MSG_SENDER_H_
#define _SBP_MSG_SENDER_H_

#include "Utils/ErrorsSubsystem.h"

namespace SBProto {	class ISafeBiProtoForSendPack; }

namespace SBProtoExt
{
	class SbpMsgSender
	{
	public:
		SbpMsgSender(SBProto::ISafeBiProtoForSendPack *proto = 0) : 
			m_proto(proto), m_msgSended(false)
		{}

		~SbpMsgSender()
		{
			ESS_ASSERT(m_msgSended);
		}

		template <class Msg> void Send()
		{
			if(m_proto != 0)	Msg::Send(*m_proto);
			m_msgSended = true;
		}

		template <class Msg, class Par1> void Send(const Par1 &par1)
		{
			if(m_proto != 0)	Msg::Send(*m_proto, par1);
			m_msgSended = true;
		}

		template <class Msg, class Par1, class Par2> void Send(const Par1 &par1, const Par2 &par2)
		{
			if(m_proto != 0)	Msg::Send(*m_proto, par1, par2);
			m_msgSended = true;
		}

		template <class Msg, class Par1, class Par2, class Par3> 
			void Send(const Par1 &par1, const Par2 &par2, const Par3 &par3)
		{
			if(m_proto != 0)	Msg::Send(*m_proto, par1, par2, par3);
			m_msgSended = true;
		}

		template <class Msg, class Par1, class Par2, class Par3, class Par4> 
			void Send(const Par1 &par1, const Par2 &par2, const Par3 &par3, 
			const Par4 &par4)
		{
			if(m_proto != 0)	Msg::Send(*m_proto, par1, par2, par3, par4);
			m_msgSended = true;
		}

		template <class Msg, class Par1, class Par2, class Par3, class Par4, class Par5> 
			void Send(const Par1 &par1, const Par2 &par2, const Par3 &par3, 
			const Par4 &par4, const Par5 &par5)
		{
			if(m_proto != 0)	Msg::Send(*m_proto, par1, par2, par3, par4, par5);
			m_msgSended = true;
		}

		template <class Msg, class Par1, class Par2, class Par3, class Par4, class Par5, class Par6> 
			void Send(const Par1 &par1, const Par2 &par2, 
			const Par3 &par3, const Par4 &par4, const Par5 &par5, 
			const Par6 &par6)
		{
			if(m_proto != 0)	Msg::Send(*m_proto, par1, par2, par3, par4, par5, par6);
			m_msgSended = true;
		}

		template <class Msg, class Par1, class Par2, class Par3, class Par4, class Par5, class Par6, class Par7> 
			void Send(const Par1 &par1, const Par2 &par2, const Par3 &par3, 
			const Par4 &par4, const Par5 &par5, const Par6 &par6, 
			const Par7 &par7)
		{
			if(m_proto != 0)	Msg::Send(*m_proto, par1, par2, par3, par4, par5, par6, par7);
			m_msgSended = true;
		}

	private:
		SBProto::ISafeBiProtoForSendPack *m_proto;
		bool m_msgSended;
	};
};

#endif
