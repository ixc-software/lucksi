#ifndef NOBJCONFTESTER_H
#define NOBJCONFTESTER_H

#include "Domain/NamedObject.h"
#include "iCmpExt/ICmpChannel.h"
#include "iCore/MsgTimer.h"
#include "iRtp/RtpPayload.h"

namespace HiLevelTests
{
    namespace ConfTest
    {
        using boost::scoped_ptr;

        struct ConfTestProfile 
        {
            int m_channel;   
            int m_lFreq; 
            int m_rFreq;
            int m_minRtpPort;
            int m_maxRtpPort;

            iRtp::RtpPayload m_srcPayload;
            iRtp::RtpPayload m_receivePayload;

            ConfTestProfile()
                :m_channel(1),
                m_lFreq(700),
                m_rFreq(1000),
                m_minRtpPort(1000),
                m_maxRtpPort(10000),
                m_srcPayload(iRtp::PCMA),
                m_receivePayload(iRtp::PCMU)
            {}
        };

        class NObjConfTester : public Domain::NamedObject, ConfTestProfile
        {	
            Q_OBJECT;        

            iCore::MsgTimer m_timer;
            class TestImpl;		
            scoped_ptr<TestImpl> m_impl;		

            struct FinishInfo
            {
                bool Ok;
                std::string Desc;

                FinishInfo(bool ok, const std::string &desc) : Ok(ok), Desc(desc)
                {
                }
            };

            //void Log(QString msg);
            void OnReset(FinishInfo info);
            void OnFinish(FinishInfo info);

            void OnTimer(iCore::MsgTimer* );

            // Used by TestImpl
        private:
            const ConfTestProfile& getProfile() const
            {
                return *this;
            }
            void Finish(bool ok, const std::string& descr);
            iCore::MsgThread& getThread()
            {
                return getMsgThread();
            }
            iLogW::LogSession& getLog()
            {
                return Log();
            }


        public:

            NObjConfTester(Domain::IDomain *pDomain, const Domain::ObjectName &name);
			~NObjConfTester();
            // board1 - the names of dri-objects which can cast to ICmpChannelCreator (board in emulator pair)            
            // board2 - the names of dri-objects which can cast to ICmpPointCreator (board in emulator pair)            
            // See Test structure below
            Q_INVOKABLE void Run(DRI::IAsyncCmd* pAsyncCmd, QString board1, QString board2, bool useRtpSrc);          

            // optional test settings
            Q_PROPERTY (int Channel READ m_channel WRITE m_channel);   
            Q_PROPERTY (int LFreq READ m_lFreq WRITE m_lFreq);   
            Q_PROPERTY (int RFreq READ m_rFreq WRITE m_rFreq);  
            Q_PROPERTY (int MinRtpPort READ m_minRtpPort WRITE m_minRtpPort);
            Q_PROPERTY (int MaxRtpPort READ m_maxRtpPort WRITE m_maxRtpPort);
            //Q_PROPERTY payload

            // todo Freq [300, 2000], channel[1,30]

        };

    } // namespace ConfTest

} // namespace HiLevelTest


/*
    Test structure.

    useRtpSrc true:
                     <-- left side | right side -->
    m_rtpChannSrc:RtpSigSource            m_rtpPoint:RtpSigSource   m_detector:RtpSigDetector
     |                                                         |    ^
    rtp                                                        |    | 
     |                                                         |    |
     |                                                         |    |
     |   board1:                               board2:    rtpPoint rtpPoint 
     |                                                         V    |
     L-> rtp->tdm         <===tdmChannel===>   tdmPoint --> conference

 // ------------------------------------------------------------------------------------

    useRtpSrc false:
                    <-- left side | right side -->
    m_rtpChannSrc:RtpSigSource                         m_detector:RtpSigDetector
    |                                                          ^
    rtp                                                        | 
    |                                                          |
    |                                                          |
    |   board1:                               board2:       rtpPoint 
    |                                                          |
    L-> rtp->tdm         <===tdmChannel===>   tdmPoint --> conference <-- genPoint     
    
*/

#endif