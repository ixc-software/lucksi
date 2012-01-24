#ifndef _CMP_CONF_PPOINT_H_
#define _CMP_CONF_PPOINT_H_

#include "ICmpConnection.h"
#include "iCmpConfPoint.h"
#include "CmpConfReceiver.h"

namespace iCmpExt
{

    class CmpConfPoint : boost::noncopyable,
		ICmpConfResponseReceiver
	{
	public:

		CmpConfPoint(const ICmpConfPoint *owner,
			ICmpConfPointEvents &user,
			Utils::WeakRef<ICmpConnection&> cmpConnection) :
			m_errorHandler(boost::bind(&ICmpConfPointEvents::PointDeleted, &user, owner, _1)),
			m_cmpConnection(cmpConnection)
		{}

		SBProtoExt::SbpMsgSender Sender()
		{
			ICmpConnection *connection = Connection();

			if(connection == 0) return SBProtoExt::SbpMsgSender();

			Utils::WeakRef<ICmpConfResponseReceiver*> ref(
				m_selfRefHost.Create<ICmpConfResponseReceiver*>(this));
			
			return connection->SbpSender(
				new CmpConfResponseReceiver(*connection, ref));
		}
		
		ICmpConnection* Connection() 
		{
			if(m_cmpConnection.Empty()) return 0;

			if(!m_cmpConnection.Value().IsCmpActive()) return 0;

			return &m_cmpConnection.Value();
		}

    // ICmpConfResponseReceiver
	private:
        void BfRespConf(const std::string &params)
        {
			ErrorOccur("Wrong response - " + params);
        }

        void ErrorOccur(const std::string &desc)
        {
			m_errorHandler(desc);
        }

    private:
		boost::function<void (const std::string&)> m_errorHandler;
		Utils::WeakRef<ICmpConnection&> m_cmpConnection;
		Utils::WeakRefHost m_selfRefHost;
	};
};

#endif






