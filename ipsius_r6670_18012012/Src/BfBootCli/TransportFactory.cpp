#include "stdafx.h"

#include "SafeBiProto/SbpTcpTransportFactory.h"
#include "SafeBiProtoExt/SbpUartTransportFactory.h"
#include "iLog/LogManager.h"
#include "Domain/DomainClass.h"
#include "TransportFactory.h"

namespace BfBootCli
{
    
	boost::shared_ptr<SBProto::ISbpTransport> SbpTcpCreator::Create(Domain::IDomain& domain)
    {
        return SBProto::SbpTcpTransportFactory::CreateTransport(
			domain.getDomain().getMsgThread(), m_dstAddr, 0, m_maxSendSize);
    }

    // ------------------------------------------------------------------------------------

	boost::shared_ptr<SBProto::ISbpTransport> SbpUartCreator::Create(Domain::IDomain& domain)
    {
        SBProtoExt::SbpUartProfile prof;
        prof.BoudRate = m_boudRate;
		boost::shared_ptr<SBProto::ISbpTransport> result;
        std::string err;
        result = SBProtoExt::SbpUartTransportFactory::
			CreateTransport(m_com, prof, err, false, m_maxSendSize);
        if (result == 0) ESS_THROW_MSG(Error, err);
        return result;
    }

} // namespace BfBootCli



