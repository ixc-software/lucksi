#ifndef _MINI_FLASHER_CMD_LIST_H_
#define _MINI_FLASHER_CMD_LIST_H_

#include "stdafx.h"
#include "Utils/ManagedList.h"
#include "IMiniFlasherCmd.h"

namespace MiniFlasher
{
	class MiniFlasherCmdList : boost::noncopyable
	{
	public:
		MiniFlasherCmdList() : m_started(false), m_currentCmd(0)
		{}

		void Add(IMiniFlasherCmd *cmd)
		{
			ESS_ASSERT(!m_started);
			m_commands.Add(cmd);

		}
		void Start()
		{
			ESS_ASSERT(!m_started);
			m_started = true;
			m_commands[0]->Start();
		}
		void Reset()
		{
			ESS_ASSERT(m_started);
			m_started = false;
			m_currentCmd = 0;
			Start();
		}

		void Process()
		{
			ESS_ASSERT(m_started);

			m_commands[m_currentCmd]->Process();
		}
		void RunNext()
		{
			ESS_ASSERT(m_started && !CurrentComIsLast());
			m_commands[++m_currentCmd]->Start();
		}
		bool CurrentComIsLast() const 
		{
			return m_currentCmd + 1 == m_commands.Size();
		}
	private:
		bool m_started;
		Utils::ManagedList<IMiniFlasherCmd> m_commands;
		int m_currentCmd;
	};
};

#endif

