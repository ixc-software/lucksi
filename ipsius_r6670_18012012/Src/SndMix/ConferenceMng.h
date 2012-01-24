#ifndef __CONFERENCEMNG__
#define __CONFERENCEMNG__

#include "Utils/ManagedList.h"
#include "Utils/IBasicInterface.h"
#include "Utils/IntToString.h"
#include "Utils/BidirBuffer.h"
#include "Utils/StatisticElement.h"

#include "iRtp/IRtpCoreToUser.h"
#include "iRtp/RtpCoreSession.h"
#include "iLog/iLogSessionCreator.h"
#include "TdmMng/EventsQueue.h"

#include "MixerError.h"
#include "MixMode.h"
#include "MixPointImpl.h"
#include "MixerCore.h"
#include "IBasicPoint.h"
#include "IConfToTdm.h"
#include "mixerpoints.h"

namespace SndMix
{

    
    // объект управления конференциями
    class ConferenceMng : public IConferenceMng, boost::noncopyable
    {
        scoped_ptr<iLogW::LogSession> m_log; 
        iLogW::LogRecordTag m_infoTag;

        iRtp::RtpParams m_rtpParams;
        iRtp::IRtpInfra &m_rtpInfra;
        TdmMng::EventsQueue &m_evQueue;
        IConfToTdm &m_tdm;

        Utils::ManagedList<MixerCore> m_conf;      // [conf index/handle]
        Utils::ManagedList<IBasicPoint> m_points;   // [point index/handle]

        Utils::StatElementForInt m_sockStat;

        template<class TElem>
        static void VerifyIndex(const Utils::ManagedList<TElem> &v, int index, 
            const char *pMsg)
        {
            if ( (index < 0)             || 
                (index >= v.Size())      ||
                (v[index] == 0) )
            {
                std::ostringstream oss;
                oss << "Bad " << pMsg << " handle " << index; 
                ESS_THROW_MSG(Error::BadHandle, oss.str());
            }
        }

        void VerifyConfIndex(int index) const
        {
            VerifyIndex(m_conf, index, "conf");
        }

        void VerifyPointIndex(int index) const
        {
            VerifyIndex(m_points, index, "point");
        }

        int GetFreePointIndex()
        {
            int i = m_points.Find( (IBasicPoint*)0 );

            if (i < 0)
            {
                i = m_points.Size();
                m_points.Add(0);
            }

            return i;
        }

        template<class T>
        static void ValidateHandle(Utils::ManagedList<T> &v, int handle)
        {
            if (handle >= CConfHandleLimit) ESS_THROW(Error::HandleLimit);

            if (handle >= v.Size())
            {
                v.AddEmptyItems(handle - v.Size() + 1);
                return;
            }

            if (v[handle] != 0) ESS_THROW(Error::HandleNotEmpty);
        }


    // IConferenceMng impl
    private:

        const iRtp::RtpParams& RtpParams() const
        {
            return m_rtpParams;
        }

        iRtp::IRtpInfra& RtpInfra()
        {
            return m_rtpInfra;
        }

        TdmMng::EventsQueue& Queue()
        {
            return m_evQueue;
        }

        IConfToTdm& ConfToTdm()
        {
            return m_tdm;
        }

        SafeRef<iLogW::ILogSessionCreator> LogCreator()
        {
            return m_log->LogCreator();
        }

    public:
        
        ConferenceMng(iLogW::ILogSessionCreator &logCreator,
            TdmMng::EventsQueue &evQueue,
            const iRtp::RtpParams &rtpParams,
            iRtp::IRtpInfra &rtpInfra,
            IConfToTdm &tdm) : 
            m_log( logCreator.CreateSession("ConferenceMng", true) ),
            m_infoTag( m_log->RegisterRecordKindStr("info") ),
            m_rtpParams(rtpParams),
            m_rtpInfra(rtpInfra),
            m_evQueue(evQueue),
            m_tdm(tdm),
            m_conf(true, 0, true),
            m_points(true, 0, true)
        {
                if ( m_log->LogActive() )
                {
                    *m_log << m_infoTag << "Started!" << iLogW::EndRecord;
                }

        }
        
        void CreateConference(int confHandle, bool autoMode, int blockSize)
        {
            ValidateHandle(m_conf, confHandle);
            m_conf.Set( confHandle, 
                new MixerCore(*m_log, autoMode, confHandle, blockSize) );
        }

        void ConfSetMode(int confHandle, const MixMode &mode)
        {
            VerifyConfIndex(confHandle);
            m_conf[confHandle]->SetMode(mode);
        }

        void DeleteConference(int confHandle)
        {
            VerifyConfIndex(confHandle);

            if ( m_log->LogActive() )
            {
                std::vector<MixPointState> i = ConfGetState(confHandle);
                *m_log << m_infoTag << "DeleteConference " 
                       << MixPointState::ToString(i) << iLogW::EndRecord;
            }

            // remove add points from conference before delete
            for(int i = 0; i < m_points.Size(); ++i)
            {
                IBasicPoint *p = m_points[i];
                if (p == 0) continue;

                if ( m_conf[confHandle]->PointIn( p->ConfPoint() ) )
                {
                    RemovePointFromConf(i);
                }

            }

            // delete conf 
            m_conf.Set(confHandle, 0);
        }

        std::vector<MixPointState> ConfGetState(int confHandle)
        {
            VerifyConfIndex(confHandle);
            return m_conf[confHandle]->State();
        }

        void CreateTdmPoint(int pointHandle, const std::string &devName, int chNum)
        {
            // find dublicates
            for(int i = 0; i < m_points.Size(); ++i)
            {
                IBasicPoint *p = m_points[i];
                if (p == 0) continue;

                std::string currDevName; 
                int currChNum;
                if ( p->GetTdmPointInfo(currDevName, currChNum) )
                {
                    if ((currDevName == devName) && (currChNum == chNum))
                    {
                        ESS_THROW(Error::DublicateTdmPoint);
                    }
                }
            }

            // create
            ValidateHandle(m_points, pointHandle);
            IBasicPoint *p = new TdmPoint(*this, pointHandle, devName, chNum);
            m_points.Set(pointHandle, p);
        }

        void CreateRtpPoint(int pointHandle, 
            const std::string &codecName, 
            const std::string &codecParams,
            int &udpPort)
        {
            // create
            ValidateHandle(m_points, pointHandle);
            IBasicPoint *p = new RtpPoint(*this, pointHandle, codecName, codecParams, udpPort);
            m_points.Set(pointHandle, p);
        }

        void CreateGenPoint(int pointHandle, const iDSP::Gen2xProfile &profile)
        {
            ValidateHandle(m_points, pointHandle);
            IBasicPoint *p = new GenPoint(*this, pointHandle, profile);
            m_points.Set(pointHandle, p);
        }

        void DeletePoint(int pointHandle)
        {
            VerifyPointIndex(pointHandle);

            // delete item
            m_points.Set(pointHandle, 0);
        }

        void RtpPointSend(int pointHandle, const std::string &dstIp, int dstPort)
        {
            VerifyPointIndex(pointHandle);

            if ( !m_points[pointHandle]->RtpSend(dstIp, dstPort) )
            {
                ESS_THROW(Error::BadPointType);
            }
        }

        void AddPointToConf(int pointHandle, int confHandle, PointMode mode)
        {
            VerifyPointIndex(pointHandle);
            VerifyConfIndex(confHandle);

            m_points[pointHandle]->AddToConf( m_conf[confHandle], mode );
        }

        void RemovePointFromConf(int pointHandle)
        {
            VerifyPointIndex(pointHandle);

            m_points[pointHandle]->RemoveFromConf();
        }        

        // call it with Mixer freq - !!
        void Process()
        {
            // sockets read
            dword t = Platform::GetSystemTickCount();
            for(int i = 0; i < m_points.Size(); ++i)
            {
                IBasicPoint *p = m_points[i];
                if (p == 0) continue;
                p->ProcessRead(t, m_sockStat);
            }

            // do mix + send
            for(int i = 0; i < m_conf.Size(); ++i)
            {
                MixerCore *p = m_conf[i];
                if (p == 0) continue;
                p->DoMixCycle();
            }

        }

    };
    
}  // namespace SndMix

#endif
