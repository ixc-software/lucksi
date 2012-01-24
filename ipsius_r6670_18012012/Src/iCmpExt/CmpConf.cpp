#include "stdafx.h"
#include "CmpConf.h"
#include "Utils/IntToString.h"


namespace 
{
/*
	class CmpConfStateRequest : public iCmpExt::CmpRespReceiver
	{
	public:
		CmpConfStateRequest(iCmpExt::ICmpConnection &cmpConnection,
			iCmpExt::CmpPointLog &log) : 
			iCmpExt::CmpRespReceiver(cmpConnection),
			m_log(log)
		{
		}

		~CmpConfStateRequest()
		{
			if (m_log.LogActive()) m_log.Log("Deleted.");
		}

	// iCmpExt::CmpRespReceiver
	private:

		void OnBfRespConf(const std::string &params)
		{
			if (m_log.LogActive()) m_log.Log("State: " + params);
		}

	private:
		iCmpExt::CmpPointLog m_log;
	};
*/
}

namespace iCmpExt
{

	CmpConf::CmpConf( ICmpConfPointEvents &owner, Utils::WeakRef<ICmpConnection&> cmpConnection, iLogW::ILogSessionCreator &logCreator, CmpHandleManager &handleManager, bool autoMode, int blockSize /*= 160*/ ) : 
		m_confPoint(this, owner, cmpConnection),
		m_handle(handleManager),
		m_log(logCreator, cmpConnection.Value().NameBoard() + "_CmpConf_" + Utils::IntToString(Handle()))
	{
		if (m_log.LogActive()) m_log.Log("Created. Board: " +  cmpConnection.Value().NameBoard());

		m_confPoint.Sender().Send<iCmp::PcCmdConfCreate>(Handle(), autoMode, blockSize);
	}

	// -------------------------------------------------------------------------------------

	CmpConf::~CmpConf()
	{
		ICmpConnection *connection  = m_confPoint.Connection();

		if (connection == 0) return;
		
		// в запросе статистики лог сессия этого обьекта сохраняется в Cmp команде, 
		// это приводит к срабатыванию assert в SafeRefServer при удалении parrent log session

//		connection->SbpSender(
//			new CmpConfStateRequest(*connection, m_log)).Send<iCmp::PcCmdConfGetState>(Handle());

		m_confPoint.Sender().Send<iCmp::PcCmdConfDelete>(Handle());
	}

	// -------------------------------------------------------------------------------------

	void CmpConf::ConfMode( const std::string &mode )
	{
		//			if (LogActive()) Log("Mode - " + mode);
		m_confPoint.Sender().Send<iCmp::PcCmdConfMode>(Handle(), mode);
	}

	// -------------------------------------------------------------------------------------

	void CmpConf::AddPoint( const ICmpConfPoint &point, bool send, bool recv )
	{
		if (m_log.LogActive()) 
			m_log.Log("Add point. " + Utils::IntToString(point.Handle()));

		m_confPoint.Sender().Send<iCmp::PcCmdConfAddPointToConf>(point.Handle(), Handle(), send, recv);
	}

	// -------------------------------------------------------------------------------------

	void CmpConf::RemovePoint( const ICmpConfPoint &point )
	{
		if (m_log.LogActive()) 
			m_log.Log("RemovePoint point. " + Utils::IntToString(point.Handle()));

		m_confPoint.Sender().Send<iCmp::PcCmdConfRemovePointFromConf>(point.Handle());
	}
};




