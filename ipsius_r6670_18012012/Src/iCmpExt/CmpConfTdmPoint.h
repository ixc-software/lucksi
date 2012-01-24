#ifndef _CMP_CONF_TDM_PPOINT_H_
#define _CMP_CONF_TDM_PPOINT_H_

#include "CmpConfPoint.h"
#include "CmpHandleManager.h"
#include "CmpPointLog.h"
#include "Utils/IntToString.h"

namespace iCmpExt
{
	class CmpConfTdmPoint : public ICmpConfPoint
	{
	public:
		CmpConfTdmPoint(ICmpConfPointEvents &owner,
			Utils::WeakRef<ICmpConnection&> cmpConnection,
			iLogW::ILogSessionCreator &logCreator,
			CmpHandleManager &handleManager,
			int channel) : 
			m_confPoint(this, owner, cmpConnection),
			m_handle(handleManager),
			m_log(logCreator, cmpConnection.Value().NameBoard() + "_CmpConfTdmPoint_" + Utils::IntToString(Handle()))
		{
			if(m_log.LogActive()) m_log.Log(
				"Created. Board: " +  cmpConnection.Value().NameBoard() + " Channel: " + Utils::IntToString(channel));

			m_confPoint.Sender().Send<iCmp::PcCmdConfCreateTdmPoint>(
				Handle(), cmpConnection.Value().NameBoard(), channel);

			m_dataCapture.reset(cmpConnection.Value().CreateChDataCapture(channel, 
				m_log.Log().NameStr()));
		}

		~CmpConfTdmPoint()
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
		boost::scoped_ptr<iCmpExt::ChDataCapture> m_dataCapture;
 
	};

};

#endif






