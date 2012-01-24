#ifndef TRANSPORTFACTORY_H
#define TRANSPORTFACTORY_H

#include "Utils/IBasicInterface.h"
#include "Utils/HostInf.h"
#include "SafeBiProto/ISbpTransport.h"
#include "Domain/IDomain.h"

namespace BfBootCli
{
    class ITransportFactory : public Utils::IBasicInterface
    {
    public:
        ESS_TYPEDEF(Error);
        virtual boost::shared_ptr<SBProto::ISbpTransport> Create(Domain::IDomain& domain) = 0; // can throw

        //virtual int MaxPackSize() const
    };

    // ------------------------------------------------------------------------------------
    
    class SbpTcpCreator : public ITransportFactory
    {
        Utils::HostInf m_dstAddr;
        size_t m_maxSendSize;
    //ITransportFactory impl
    private:
        boost::shared_ptr<SBProto::ISbpTransport> Create(Domain::IDomain& domain);
    public:
        SbpTcpCreator(const Utils::HostInf& dstAddr, size_t maxSendSize = 0) 
            : m_dstAddr(dstAddr), m_maxSendSize(maxSendSize)
        {}

        //SbpTcpCretor(const BroadcastMsg& msg) -- ?
    };

    // ------------------------------------------------------------------------------------

    class SbpUartCreator : public ITransportFactory
    {
        int m_com;
        int m_boudRate;
        size_t m_maxSendSize;

    //ITransportFactory impl
    private:
        boost::shared_ptr<SBProto::ISbpTransport> Create(Domain::IDomain& domain);
    public:
        SbpUartCreator(int com, int boudRate, size_t maxSendSize = 0) 
            : m_com(com), m_boudRate(boudRate), m_maxSendSize(maxSendSize)
        {
            ESS_ASSERT(m_com >= 0);
        }
    };

} // namespace BfBootCli

#endif

