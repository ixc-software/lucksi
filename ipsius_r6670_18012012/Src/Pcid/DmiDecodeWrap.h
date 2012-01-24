#ifndef _DMI_DECODE_WRAP_H_
#define _DMI_DECODE_WRAP_H_

#include "stdafx.h"

namespace Pcid
{
	class DmiDecodeWrap : boost::noncopyable
	{
	public:
		static bool GetMachineID(QTextStream &in, std::string &info);
	};
}; // namespace Pcid

#endif
