#include "stdafx.h"
#include "ProgressBar.h"

namespace 
{
	const bool CProcessBarActive = true;
};

// -------------------------------------------------------------------------------------------------------------------

namespace Utils
{

	ProgressBar::ProgressBar(ILogForProgressBar &log, dword progressBarSize, char head, char step) :
		m_log(log), m_head(head), m_step(step)
	{
		Reset();
		m_progressBarSize = progressBarSize;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ProgressBar::Start(dword dataSize) 
	{
		if (!CProcessBarActive) return;

		ESS_ASSERT(!m_isActive);
		ESS_ASSERT(m_progressBarSize > 0);

		m_isActive = true;
		m_dataSize = dataSize;
		m_progressStepSize = (m_dataSize%m_progressBarSize  != 0) + m_dataSize/m_progressBarSize ; 
		m_currentPos = 0;
		// print head
		Log(std::string(m_progressBarSize, m_head), true);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ProgressBar::PrintStep(dword pos) 
	{
		if (!CProcessBarActive) return;

		ESS_ASSERT(m_isActive);
		ESS_ASSERT(pos >= m_currentPos);

		if (pos >= m_dataSize) pos = m_dataSize;
		else if(m_currentPos + m_progressStepSize > pos) return;

		while(m_currentPos + m_progressStepSize <= pos)
		{
			m_currentPos += m_progressStepSize;
			Log(std::string(1, m_step), false);
		}

		if (pos >= m_dataSize) Finish(true);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ProgressBar::Finish(bool ok)
	{
		if(!m_isActive) return;
		
		if (ok)
		{
			for(int i = m_progressBarSize - (m_currentPos / m_progressStepSize);
				i > 0; --i)
			{
				Log(std::string(1, m_step), false);
			}
		}
		Log(std::string(), true);
		Reset();
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ProgressBar::Reset()
	{
		m_isActive = false;
		m_dataSize = 0;
		m_currentPos = 0;
		m_progressStepSize = 0;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ProgressBar::Log(const std::string &msg, bool eof)
	{
		m_log.LogProgressBar(msg, eof);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ProgressBar::TestProgressBar()
	{
		class TempLog : boost::noncopyable,
			public Utils::ILogForProgressBar
		{
		public:
			void LogProgressBar(const std::string &msg, bool endl)
			{
				std::cout << msg;
				if(endl) std::cout << std::endl;
			}
		};

		TempLog log;
		Utils::ProgressBar bar1(log, 40);
		bar1.Start(700);
		bar1.PrintStep(900);

		Utils::ProgressBar bar(log, 40);
		bar.Start(700);
		dword i = 0;
		for(; i < 200; i+= 10) 
		{
			Platform::Thread::Sleep(200);
			bar.PrintStep(i);
		}
		for(; i < 400; i+= 50) 
		{
			Platform::Thread::Sleep(200);
			bar.PrintStep(i);
		}
		for(; i < 650; i+= 5) 
		{
			Platform::Thread::Sleep(200);
			bar.PrintStep(i);
		}
		bar.PrintStep(900);
	};

} // namespace Utils
