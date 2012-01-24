#ifndef NOBJDTMFTESTER_H
#define NOBJDTMFTESTER_H

#include "iCmpExt/ICmpChannel.h"
#include "iRtp/RtpPcInfra.h"
#include "Domain/NamedObject.h"
#include "iMedia/iCodec.h"
#include "iCore/MsgTimer.h"

namespace HiLevelTests
{
    namespace Dtmf
    {                      

        class NObjDtmfTester 
            : public Domain::NamedObject
        {
            Q_OBJECT;

            class DtmfTesterImpl;                  
            boost::scoped_ptr<DtmfTesterImpl> m_impl;            

            int m_channel;                         
            int m_minRtpPort;
            int m_maxRtpPort;

            iCore::MsgTimer m_timer;

        // use by DtmfTesterImpl
        private:
            void TestFinished(bool ok, QString descr);
            iRtp::RtpInfraParams RtpInfraParams() const;  
            int getChannel() const { return m_channel; }                        
            iCore::MsgThread& getMsgThread() { return Domain::NamedObject::getMsgThread(); }
            iLogW::LogSession& Log() { return Domain::NamedObject::Log(); }           

        private:

            struct FinishInfo
            {
                const QString Descr;
                const bool Ok;
                FinishInfo(QString descr, bool ok) : Descr(descr), Ok(ok) {}                
            };

            void OnTimeout(iCore::MsgTimer*pTimer);
            iCmpExt::ICmpChannelCreator& CastIntf(QString driName);            
            void OnFinished(const FinishInfo& info);

        public:

            NObjDtmfTester(Domain::IDomain *pDomain, const Domain::ObjectName &name);
            ~NObjDtmfTester();            
            
            // b1, b2 - the names of dri-objects which can cast to ICmpChannelCreator
            Q_INVOKABLE void Run(DRI::IAsyncCmd* pAsyncCmd, QString b1, QString b2);

            Q_PROPERTY (int Channel READ m_channel WRITE m_channel);            
            Q_PROPERTY (int MinRtpPort READ m_minRtpPort WRITE m_minRtpPort);
            Q_PROPERTY (int MaxRtpPort READ m_maxRtpPort WRITE m_maxRtpPort);            
        };


    } // namespace Dtmf

} // namespace HiLevelTests

#endif
