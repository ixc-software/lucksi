#include "stdafx.h"

#include "Sip/ISipCall.h"
#include "CallTestRtpWithSip.h"

namespace TestRtpWithSip
{
	CallTestRtpWithSip::CallTestRtpWithSip(iCore::MsgThread &thread,
        Utils::IVirtualDestroyOwner &owner,
		Utils::SafeRef<Sip::ISipCall> call,
		Utils::SafeRef<Sip::ISipToRtp> rtp,
		int timeout) : 
        iCore::MsgObject(thread),
        m_owner(owner),
        m_call(call),
		m_timer(this, &T::EndTalk)
    {
        call->SetUserCall(this, rtp);
        call->Alert(this, false);
        call->Accept(this);
        if (timeout) m_timer.Start(timeout);
        /*QString scnName(typeid(*rtp).name());
        std::cout << "Scenario \"" << scnName.section("::", -1) << "\" started." << std::endl;*/
    }

    //------------------------------------------------------------------------------------
    // ISipCallEvents

	void CallTestRtpWithSip::Alerted(const Sip::ISipCall *call)
	{
		// обрабатываем только входящие вызовы 
		ESS_HALT("make only incoming call");
		ESS_ASSERT(m_call.IsEqualIntf(call));
	}

	//------------------------------------------------------------------------------------

	void CallTestRtpWithSip::Connected(const Sip::ISipCall *call)
	{
		// обрабатываем только входящие вызовы 
		ESS_HALT("make only incoming call");
		ESS_ASSERT(m_call.IsEqualIntf(call));
	}

	//------------------------------------------------------------------------------------

	void CallTestRtpWithSip::Terminated(const Sip::ISipCall *call, int statusCode)
	{
		ESS_ASSERT(m_call.IsEqualIntf(call));
        m_call.Clear();
        m_owner.Delete(this);
	}

	//------------------------------------------------------------------------------------

	void CallTestRtpWithSip::EndTalk(iCore::MsgTimer *pT)
	{
		m_call.Clear()->Release(this);
        m_owner.Delete(this);
	}
};




