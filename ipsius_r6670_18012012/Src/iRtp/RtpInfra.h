#ifndef _RTP_INFRA_H_
#define _RTP_INFRA_H_

#include "stdafx.h"
#include "Utils/Random.h"
#include "Utils/IBidirBuffCreator.h"
#include "iLog/LogWrapper.h"
#include "RtpParams.h"
#include "IRtpInfra.h"

namespace iCore { class MsgThread; };


namespace iRtp
{   
    class RtpInfra : boost::noncopyable, public IRtpInfra
    {                        
    public:       
        RtpInfra(iCore::MsgThread &thread,
            const RtpInfraParams &m_params,
            Utils::IBidirBuffCreator &buffCreator);

        ~RtpInfra();

    // IRtpInfra
    private:

        SocketPair CreateSocketPair(ISocketToRtp& socketOwner, bool useRtcp); // can throw // override   

        dword GenRndSSRC() 
        {
            return m_rnd.Next();
        }

        BidirBuffer* CreateBuff(int offset)
        {
            return m_buffCreator.CreateBidirBuff(offset);
        }   

        BidirBuffer* CreateBuff()
        {
            return m_buffCreator.CreateBidirBuff();
        }   

        void FreeSocketPort(int port);

    private:
        Utils::IBidirBuffCreator& m_buffCreator;
        Utils::Random m_rnd;
        class Impl;
        boost::scoped_ptr<Impl> m_impl;
    };
} // namespace iRtp

#endif
