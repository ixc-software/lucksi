#ifndef _NOBJ_RULE_PAR_CALL_MEETING_H_
#define _NOBJ_RULE_PAR_CALL_MEETING_H_

#include "stdafx.h"

namespace CallMeeting
{

	class CallMeetingRulePar
	{
	public:
		CallMeetingRulePar(int density = 0, int percent = 0)
		{
			m_density = density;
			m_percent = percent;
		}

		bool LoadRule(const QString &str, const QString &splitter)
		{
			int i = str.indexOf(splitter);
			if(i <= 0 || i + 1 >= str.size()) return false;
			
			bool ok = false;
			m_density = str.left(i).toInt(&ok);

			if(ok) m_percent = str.mid(i + 1).toInt(&ok);
			return ok;
		}

		QString SaveRule(const QString &splitter) const
		{
			return QString("%1%2%3").arg(m_density).arg(splitter).arg(m_percent);
		}

		int Density() const	{	return m_density;	}

		int Percent() const	{	return m_percent;	}

		static void Test()
		{
			TestPair("10 | 25", "10|25");
			TestPair("10| 25", "10|25");
			TestPair("10 |25", "10|25");
			TestPair(" 10 |25", "10|25");
			TestPair(" 10 |25 ", "10|25");
			TestPair(" 10 | 25", "10|25");
			TestPair(" 10 | 25 ", "10|25");
		}
		static void TestPair(const QString &in,const QString &out)
		{
			CallMeetingRulePar rule;
			ESS_ASSERT(rule.LoadRule(in, "|"));
			ESS_ASSERT(rule.SaveRule("|") == out);
		}

	private:
		int m_density;
		int m_percent;
	};
};


#endif

