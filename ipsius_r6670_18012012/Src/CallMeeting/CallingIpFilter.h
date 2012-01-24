#ifndef _CALLING_IP_FILTER_H_
#define _CALLING_IP_FILTER_H_

#include "stdafx.h"
#include "CallMeetingStat.h"

namespace CallMeeting
{
	struct CallingIpFilter : boost::noncopyable
	{
		CallingIpFilter(const QString &ip) : Ip(ip){}

		void StatAll(QStringList &res) const
		{
			res << QString("IpFilter: %1").arg(Ip);
			Stat.ToString(res);
		}
		void StatShort(Utils::TableOutput &table) const
		{
			Stat.ShortStat(Ip, table);
		}

		QString Ip;
		CallMeetingStat Stat;
	};
};

#endif
