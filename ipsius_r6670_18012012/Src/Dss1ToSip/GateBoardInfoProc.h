#pragma once

#include "Utils/IBasicInterface.h"

namespace Dss1ToSip
{
	struct BoardInfo
	{
		BoardInfo() 
		{
			Calls = Talks = 0;
		}
		QString Alias;
		QString DChannel;
		QString BoardName;
		QString State;
		int Calls;
		int Talks;
		typedef std::list<BoardInfo> List;
		static QStringList ToString(const List &in);
	};

	class IBoardInfoReq : public Utils::IBasicInterface
	{
	public:
		virtual void CallsInfoReq(QStringList &info, bool brief) const = 0;
		virtual void BoardInfoReq(BoardInfo::List &info) const = 0;
	};
}



