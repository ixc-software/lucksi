#ifndef _IMINI_FLASHER_CMD_H_
#define _IMINI_FLASHER_CMD_H_

#include "stdafx.h"
#include "ILogForFlasher.h"

namespace MiniFlasher
{
	class IMiniFlasherCmd : public Utils::IBasicInterface
	{
	public:
		virtual void Start() = 0;
		virtual void Process() = 0;
	};

	class IMiniFlasherCmdOwner : public ILogForFlasher
	{
	public:
		virtual void Reset() = 0;
		virtual void EndProcess(bool ok, const std::string &str = "") = 0;
	};
};


#endif
