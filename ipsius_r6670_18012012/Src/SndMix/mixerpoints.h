#ifndef __MIXERPOINTS__
#define __MIXERPOINTS__

#include "iDSP/GenUtils.h"
#include "iDSP/FullCicleGen.h"

#include "BasicPoint.h"

namespace SndMix
{
    using boost::scoped_ptr;

    // IBasicPoint implementation thru BasicPoint
    class ConfBasicPoint : public BasicPoint, public IBasicPoint
    {
        IConferenceMng &m_owner;
        
    // IBasicPoint impl
    private:

        bool GetTdmPointInfo(std::string &devName, int &chNum) const
        {
            return false;
        }

        bool RtpSend(const std::string &dstIp, int dstPort) 
        {
            return false;
        }

        // override in childs 
        // void ProcessRead(dword ticks, Utils::StatElementForInt &sockStat)

        void AddToConf(SafeRef<IMixPointOwner> owner, PointMode mode)
        {
            RegisterPoint(owner, mode, MixPointImplProfile::CTypicalPacket);
        }

        void RemoveFromConf()
        {
            UnregisterPoint();
        }

        IMixPoint* ConfPoint() // can be null, ok 
        { 
            return Point();
        }

    protected:

        ConfBasicPoint(IConferenceMng &owner, const std::string &pointType, int handle) : 
             BasicPoint(owner.RtpInfra(), *owner.LogCreator(), pointType, handle),
             m_owner(owner)
        {
        }

        IConferenceMng& Owner()
        {
            return m_owner;
        }

    };

    // ---------------------------------------------------------------
    
    /*
            GenPoint is 'active', driven by ticks value in ProcessRead()
            Only PmSend mode supported
    */
    class GenPoint : public ConfBasicPoint
    {
        enum
        {
            CGenInterval = 10,
            CGenIntervalSamples = CGenInterval * 8,
        };

        scoped_ptr<iDSP::FullCicleGenTwoToneLine> m_gen;
        dword m_lastGenTime;

        bool TryGen(dword ticks)
        {
            ESS_ASSERT(Point() != 0);
            ESS_ASSERT(m_gen != 0);

            if (ticks - m_lastGenTime < CGenInterval) return false;

            // put to mixer CGenIntervalSamples
            {
                CyclicBuff::WrIter i(Point()->ToMixerBuff(), CGenIntervalSamples);
                while( !i.End() )
                {
                    i.WriteAndMove( m_gen->NextSample() );
                }
            }            

            m_lastGenTime += CGenInterval; 

            return true;
        }

        void BeforeAddToConf(PointMode mode)  // override
        {
            if (mode != PmSend) ESS_THROW(Error::BadPointModeForGen);
            m_lastGenTime = Platform::GetSystemTickCount();
        }

    // IBasicPoint impl partial
    private:

        void ProcessRead(dword ticks, Utils::StatElementForInt &sockStat)
        {
            if (Point() == 0) return;

            while( TryGen(ticks) );
        }

    public:
                
        GenPoint(IConferenceMng &owner, int handle, const iDSP::Gen2xProfile &profile) : 
          ConfBasicPoint(owner, "Gen", handle),
          m_lastGenTime(0)
        {
            using namespace iDSP;

            // create gen
            if ( !FullCicleGenTwoTone::Validate(profile) )
            {
                ESS_THROW(Error::BadGenProfile);
            }

            m_gen.reset( new FullCicleGenTwoToneLine(profile) );
        }

        ~GenPoint() { Finalize(); }

    };

    // ---------------------------------------------------------------

    /*
            RtpPoint is driven by RTP session frequrency. 
            Write to mixer on every incoming packet.
            Read from mixer then enough data avaible. 

    */
    class RtpPoint : 
        public ConfBasicPoint,
        public iRtp::IRtpCoreToUser
    {
        iRtp::RtpCoreSession m_rtp;
        Utils::HostInf m_sendHost;

        /*
        std::string BeforeAddToConf(PointMode mode)  // override
        {
            return "RTP-" + Utils::IntToString( m_rtp.getLocalRtpAddr().Port() );
        } */

    // IBasicPoint impl partial 
    private:

        bool RtpSend(const std::string &dstIp, int dstPort) 
        {
            using Utils::HostInf;

            if (dstIp.empty())
            {
                m_sendHost.Clear();
                return true;
            }

            // set m_sendHost
            {
                HostInf hi;

                std::string err;
                std::string s = dstIp + ":" + Utils::IntToString(dstPort);
                if (!HostInf::FromString(s, hi, HostInf::HostAndPort, &err))
                {
                    ESS_THROW_MSG(Error::BadHostPort, s + " " + err);
                }

                m_sendHost = hi;
            }

            // set m_rtp
            m_rtp.setDestAddr(m_sendHost);
            m_rtp.setPayload( Codec().EncoderInfo().PayloadType() );

            return true;
        }

        void ProcessRead(dword ticks, Utils::StatElementForInt &sockStat)
        {
            m_rtp.SocketPoll(false, sockStat);

            if (Point() != 0) 
            {
                while ( RtpSendPacket(m_rtp, !m_sendHost.Empty()) );
            }
        }

    // iRtp::IRtpCoreToUser impl
    private:

        void RxData(Utils::BidirBuffer* pBuff, iRtp::RtpHeaderForUser header)
        {
            RtpGetPacket(pBuff, header);
        }

        void RxEvent(iRtp::RtpEvent ev, dword timestamp)
        {
            // nothing
        }

        void RtpErrorInd(iRtp::RtpError error)
        {
            // TODO -- push event
            // m_owner.Queue().Push(...)
        }

        void NewSsrcRegistred(dword ssrc)
        {
            // nothing
        }

    public:

        RtpPoint(IConferenceMng &owner, int handle,
            const std::string &codecName, const std::string &codecParams,                 
            /* output */ int &udpPort) : 
            ConfBasicPoint(owner, "Rtp", handle),
            m_rtp(owner.RtpParams(), Log(), owner.RtpInfra(), *this)
        {            
            InitCodec(codecName, codecParams);

            udpPort = m_rtp.getLocalRtpAddr().Port();
        }

        ~RtpPoint() { Finalize(); }

    };

    // ---------------------------------------------------------------

    /*
            TdmPoint driven by TDM IRQ frequerency -- do both, read and write. 
    */
    class TdmPoint : 
        public ConfBasicPoint,
        public TdmMng::ISendSimple,
        public TdmMng::IRtpLikeTdmSource,
        public virtual Utils::SafeRefServer
    {
        const std::string m_devName; 
        const int m_chNum;

        int m_tdmBlockSize;

        void BeforeAddToConf(PointMode mode) // override
        {
            // bind to TdmChannel
            Owner().ConfToTdm().BindToConference(m_devName, m_chNum, this, this);

            // return "TDM-" + Utils::IntToString(m_chNum);
        }

        void AfterRemoveFromConf() // override
        {
            // unbind from TdmChannel
            Owner().ConfToTdm().UnbindFromConference(m_devName, m_chNum, this, this);
        }


    // TdmMng::IRtpLikeTdmSource impl
    private:

        Utils::BidirBuffer* DetachData()
        {
            if (Point() == 0) return 0;

            return DataPeek(m_tdmBlockSize);
        }

    // TdmMng::ISendSimple impl
    private:

        void Send(const Utils::BidirBuffer &buff)
        {
            ESS_ASSERT(buff.Size() == m_tdmBlockSize);

            if (Point() != 0)
            {
                WriteBuff(buff);
            }

        }

    // IBasicPoint impl partial 
    private:

        bool GetTdmPointInfo(std::string &devName, int &chNum) const
        {
            devName = m_devName;
            chNum = m_chNum;
            return true;
        }

        void ProcessRead(dword ticks, Utils::StatElementForInt &sockStat)
        {
            // nothing
        }

    public:

        TdmPoint(IConferenceMng &owner, int handle,
            const std::string &devName, int chNum) : 
            ConfBasicPoint(owner, "Tdm", handle),
            m_devName(devName),
            m_chNum(chNum)
        {
            // create codec
            {
                int tdmBlockSize;
                bool aLow;
                owner.ConfToTdm().GetDeviceInfo(devName, tdmBlockSize, aLow);

                m_tdmBlockSize = tdmBlockSize;

                InitCodec(aLow);
            }

        }

        ~TdmPoint() { Finalize(); }

    };

    
}  // namespace SndMix

#endif