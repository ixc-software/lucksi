#ifndef _CMP_CONF_H_
#define _CMP_CONF_H_

#include "CmpConfPoint.h"
#include "CmpHandleManager.h"
#include "CmpPointLog.h"

namespace iCmpExt
{
	class CmpConf : public ICmpConf
	{
	public:
		CmpConf(ICmpConfPointEvents &owner,
			Utils::WeakRef<ICmpConnection&> cmpConnection,
			iLogW::ILogSessionCreator &logCreator,
			CmpHandleManager &handleManager,
			bool autoMode,
			int blockSize = 160);

		~CmpConf();

	// ICmpConf
	private:
		int Handle() const {	return m_handle.Handle(); }

		void ConfMode(const std::string &mode);

		void AddPoint(const ICmpConfPoint &point, bool send, bool recv);

		void RemovePoint(const ICmpConfPoint &point);

	private:
		CmpConfPoint m_confPoint;
		CmpHandleWrapper m_handle;
		CmpPointLog m_log;
	};
};

#endif




