#include "stdafx.h"
#include "Utils/OutputFormatters.h"
#include "GateBoardInfoProc.h"

namespace Dss1ToSip
{
	QStringList BoardInfo::ToString(const BoardInfo::List &in)
	{
		Utils::TableOutput table("  ", ' ');
		table.AddRow("Board name", "State", "Alias", "Calls", "Talks", "DChannel");
		for(List::const_iterator i = in.begin(); i != in.end(); ++i)
		{
			table.AddRow(i->BoardName, 
				i->State, 
				i->Alias, 
				QString("%1").arg(i->Calls), 
				QString("%1").arg(i->Talks), 
				i->DChannel);
		}
		return table.Get();
	}
};

