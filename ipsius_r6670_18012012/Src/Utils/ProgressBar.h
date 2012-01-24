#ifndef _PROGRESS_BAR_H_
#define _PROGRESS_BAR_H_

#include "stdafx.h"
#include "Platform/Platform.h"
#include "Utils/IBasicInterface.h"

namespace Utils
{
    using Platform::dword;

    class ILogForProgressBar : public Utils::IBasicInterface
    {
    public:
        virtual void LogProgressBar(const std::string &str, bool eof = true) = 0;
    };

    // -------------------------------------------------------

	class ProgressBar : boost::noncopyable 
	{
		static const int CMaxProgressBarSize = 20;
	public:
		ProgressBar(ILogForProgressBar &log, dword progressBarSize = CMaxProgressBarSize, char head = '-', char step = '*');
		void Start(dword dataSize);
		void PrintStep(dword pos);
		void Finish(bool ok = true);
        bool IsActive() const { return m_isActive; }
		static void TestProgressBar();
	private:
		void Log(const std::string &msg, bool eof);
		void Reset();
	private:
		ILogForProgressBar &m_log;
		const char m_head;
		const char m_step;
		bool m_isActive;
		dword m_dataSize;
		dword m_progressBarSize;
		dword m_currentPos;
		dword m_progressStepSize;
	};
    
} // namespace Utils

#endif
