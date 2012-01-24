#include "stdafx.h"

#include "Utils/TimerTicks.h"

#include "NObjEmulRunner.h"
#include "NObjBoardEmulPair.h"
#include "E1PcEmul.h"

namespace
{
    const int CPoolTimerMs = 1000;
}

namespace BfEmul
{
	NObjBoardEmulPair::NObjBoardEmulPair(NObjEmulRunner &owner, const Domain::ObjectName &name,
		Platform::dword boardId1, Platform::dword boardId2) :
		Domain::NamedObject(&owner, name, &owner), 
		m_owner(owner),
        m_tWaitComplete(this, &NObjBoardEmulPair::OnTimer),
		m_board1 (new NObjBoardEmul(this, "Board_0", boardId1, owner.RtpRangeCreator(), true)),
		m_board2 (new NObjBoardEmul(this, "Board_1", boardId2, owner.RtpRangeCreator(), true))
	{        
	}

	// -----------------------------------------------------------------------------------------

	NObjBoardEmulPair::~NObjBoardEmulPair()
	{
        Enable(false);

        // NamedObject::FreeChildren();

        Utils::TimerTicks t;
        t.Set(2*1000);

        while(m_runner.Process() != 0)
        {
            if (t.TimeOut()) ESS_HALT("~NObjBoardEmulPair wait timeout");
        }

		ESS_ASSERT(!Enabled());
	}

	// -----------------------------------------------------------------------------------------

 	void NObjBoardEmulPair::Enable()
 	{
         Enable(true);
 	}

	// -----------------------------------------------------------------------------------------

	void NObjBoardEmulPair::Disable(DRI::IAsyncCmd *pAsyncCmd)
	{
		AsyncBegin(pAsyncCmd);
        m_tWaitComplete.Start(CPoolTimerMs, true);

		Enable(false);
	}

	// -----------------------------------------------------------------------------------------

	void NObjBoardEmulPair::Enable(bool enabled)
	{
        // disable
		if (!enabled)
		{
			if( Enabled() )
			{
				PcEmul::Instance().DeletePair(BoardPair(m_board1->StringId(), m_board2->StringId()));				
                m_board2->Stop();
                m_board1->Stop();			
			}
			return;
		}

        // no change
		if (Enabled()) return;		

        // enable
        m_board1->Start(m_runner, m_owner.LogSettings());
        m_board2->Start(m_runner, m_owner.LogSettings());		
        PcEmul::Instance().AddPair(BoardPair(m_board1->StringId(), m_board2->StringId()));
	}

	// -----------------------------------------------------------------------------------------

	bool NObjBoardEmulPair::Enabled() const
	{
		return m_board1->Enabled() && m_board2->Enabled();
	}

	// -----------------------------------------------------------------------------------------

	bool NObjBoardEmulPair::IsBoardPresent(Platform::dword boardId) const
	{
		return m_board1->BoardNumber() == boardId || m_board2->BoardNumber() == boardId;
	}

    // -----------------------------------------------------------------------------------------

    void NObjBoardEmulPair::OnTimer(iCore::MsgTimer *pT)
    {
        ESS_ASSERT(pT == &m_tWaitComplete);
        if (m_runner.Process() != 0)  return;

        if(!AsyncActive()) return;
        m_tWaitComplete.Stop();
        AsyncComplete(true);        
    }


};

