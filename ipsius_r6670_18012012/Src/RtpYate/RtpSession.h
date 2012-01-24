#ifndef __RTPSESSION__
#define __RTPSESSION__

#include "RtpParams.h"
#include "IRtpUserReceiver.h"

namespace Rtp
{
    class RtpSession
    {
    public:    
        RtpSession(IRtpUserReceiver *pOwner, const QHostAddress &localIp, bool rtcp);
        //RtpSession(IRtpUserReceiver *pOwner, bool rtcp);    
        
        bool StartRtp(const RtpParams& params, int m_msleep = 5, 
            TelEngine::Thread::Priority threadPriority = TelEngine::Thread::Normal);
        void SendData(const QByteArray &data, bool marker = 0, unsigned long tStamp = 0);
        bool SendDtmf(char dtmf, int duration = 0);

        void AddDirection(RtpParams::Direction direction);
        iNet::PortNumber getLocalPort() const;
       
    private:
        class Impl;
        boost::shared_ptr<Impl> m_pImpl;
    };


} //namespace

#endif
