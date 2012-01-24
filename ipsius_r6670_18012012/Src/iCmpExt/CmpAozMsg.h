#ifndef _CMP_AOZ_MSG_
#define _CMP_AOZ_MSG_

#include "SafeBiProtoExt/SbpMsgSender.h"
#include "iCmp/ChMngProtoShared.h"

namespace iCmp 
{
	class AbCofidecParams;
	class AbLineParams;
};

namespace iCmpExt
{

	class ICmpConnection;

	class CmpAozMsg
	{
	public:
		CmpAozMsg(ICmpConnection &connection) :
			m_connection(connection)
		{}

		void CofidecSetup(int chMask, const iCmp::AbCofidecParams &params);

		void LineSetup(int chMask, const iCmp::AbLineParams &params);

		void DtmfSetup(boost::function<void (bool, const std::string &)> result,
			int chMask, bool useInternal,
			iCmp::FreqRecvMode mode, 
			const std::string &params = "");

	private:
		ICmpConnection &m_connection;
	};

}

#endif




