#ifndef _NOBJ_BOARD_EMUL_PAIR_H_
#define _NOBJ_BOARD_EMUL_PAIR_H_

#include "stdafx.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"

#include "NObjBoardEmul.h"

namespace BfEmul
{
	class NObjEmulRunner;

	class NObjBoardEmulPair : public Domain::NamedObject,
		public DRI::INonCreatable
	{		
		Q_OBJECT;
		typedef NObjBoardEmul T;
	public:
		NObjBoardEmulPair(NObjEmulRunner &owner, const Domain::ObjectName &name,
			Platform::dword boardId1, Platform::dword boardId2);
		~NObjBoardEmulPair();
		
		Q_INVOKABLE void Enable();
		Q_INVOKABLE void Disable(DRI::IAsyncCmd *pAsyncCmd);
		void Enable(bool enabled);
		Q_PROPERTY(bool Enabled READ Enabled);
		bool Enabled() const;
				
		bool IsBoardPresent(Platform::dword boardId) const;		

    private:		
        void OnTimer(iCore::MsgTimer *pT);

        NObjEmulRunner &m_owner;
        Utils::ThreadTaskRunner m_runner;
        iCore::MsgTimer m_tWaitComplete;

		NObjBoardEmul* m_board1;
		NObjBoardEmul* m_board2;
	};
};

#endif
