#ifndef _CMP_CONF_GEN_PPOINT_H_
#define _CMP_CONF_GEN_PPOINT_H_

#include "CmpConfPoint.h"
#include "CmpHandleManager.h"
#include "CmpPointLog.h"
#include "Utils/IntToString.h"

namespace iCmpExt
{

	class CmpConfGenPoint : public ICmpConfPoint
	{
	public:
		CmpConfGenPoint(ICmpConfPointEvents &owner,
			Utils::WeakRef<ICmpConnection&> cmpConnection,
			iLogW::ILogSessionCreator &logCreator,
			CmpHandleManager &handleManager,
			const iDSP::Gen2xProfile &data) : 
			m_confPoint(this, owner, cmpConnection),
			m_handle(handleManager),
			m_log(logCreator, cmpConnection.Value().NameBoard() + "_CmpConfGenPoint_" + Utils::IntToString(Handle()))
		{
			if(m_log.LogActive()) m_log.Log("Created. Board: " +  cmpConnection.Value().NameBoard() + ". Info: " + data.ToString());

			m_confPoint.Sender().Send<iCmp::PcCmdConfCreateGenPoint>(Handle(), data);
		}

		~CmpConfGenPoint()
		{
			if(m_log.LogActive()) m_log.Log("Deleted.");

			m_confPoint.Sender().Send<iCmp::PcCmdConfDeletePoint>(Handle());
		}

	// ICmpConfPoint
	public:
		int Handle() const {	return m_handle.Handle(); }

	private:
		CmpConfPoint m_confPoint;
		CmpHandleWrapper m_handle;
		CmpPointLog m_log;
	};

};

#endif


