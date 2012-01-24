#include "stdafx.h"

#include "TdmChannelSender.h"
#include "TdmChannel.h"
#include "BoolALawToRtpPayload.h"
#include "SendToRtp.h"
#include "SendToTdm.h"

// ------------------------------------------------------------------

namespace
{
    using namespace TdmMng;

    class SendDirectionWrapper : public ISendDirection
    {
        Utils::SafeRef<ISendSimple> m_ref;

    // ISendDirection impl
    private:

        void Send(DataPacket &pack, bool canDetach)
        {
            m_ref->Send( pack.Buffer() );
        }

        bool Equal(const std::string &ip, int port) const
        {
            return false;
        }

        bool Equal(const Utils::SafeRef<ISendSimple> &ref) const
        {
            return (ref == m_ref);
        }

    public:

        SendDirectionWrapper(const Utils::SafeRef<ISendSimple> &ref) : m_ref(ref)
        {
        }

    };

}  // namespace

// ------------------------------------------------------------------

namespace TdmMng
{

    void TdmChannelSender::StartSend(const std::string &ip, int port, bool useAlaw)
    {
        // dublicate destination
        int indx = Find(ip, port);
        if (indx >= 0) 
        {
            ESS_THROW_T(TdmException, terDublicateSender);
        }

        // just one sender (RTP session limitation now)
        if (m_list.Size() > 0)
        {
            ESS_THROW_T(TdmException, terMoreThenOneSender);
        }

        // Is destination local or remote - ?
        if (!IsLocalTransit(ip))
		{            
			Add( new SendToRtp(m_mng, ip, port, useAlaw, m_rtp) );
			return;
		}

        TdmChannel *pCh = m_mng.FindChannelByRtpPort(port);

        if (pCh == 0) ESS_THROW_T(TdmException, terLocalChannelNotFound);
        
        // debug
        int chNum = pCh->Number();  
        ESS_ASSERT(pCh->RtpPort() == port);

        // add
        Add( new SendToTdm(ip, port, pCh->GetTdmInput()) );
    }

    // --------------------------------------------------------

    void TdmChannelSender::StopSend( const std::string &ip, int port )
    {
        Delete( Find(ip, port) );
    }

    // --------------------------------------------------------

    int TdmChannelSender::Find( const std::string &ip, int port ) const
    {
        for(int i = 0; i < m_list.Size(); ++i)
        {
            if ( m_list[i]->Equal(ip, port) ) return i;
        }

        return -1;
    }

    // --------------------------------------------------------

    int TdmChannelSender::Find( const Utils::SafeRef<ISendSimple> &ref ) const
    {
        for(int i = 0; i < m_list.Size(); ++i)
        {
            if ( m_list[i]->Equal(ref) ) return i;
        }

        return -1;
    }

    // --------------------------------------------------------

    TdmChannelSender::TdmChannelSender( ITdmManager &mng, iRtp::RtpCoreSession &rtp ) : 
        m_mng(mng), 
        m_rtp(rtp),
        m_pause(false)
    {
    }


    // --------------------------------------------------------

    void TdmChannelSender::StartSend(const Utils::SafeRef<ISendSimple> &ref)
    {
        int index = Find(ref);
        if (index >= 0) 
        {
            ESS_THROW_T(TdmException, terDublicateSender);
        }

        Add( new SendDirectionWrapper(ref) );
    }

    // --------------------------------------------------------

    void TdmChannelSender::StopSend(const Utils::SafeRef<ISendSimple> &ref)
    {
        Delete( Find(ref) );
    }

    // --------------------------------------------------------

    void TdmChannelSender::Send(DataPacket &dataFromTdm)
    {
        if (m_pause) return;

        bool canDetach = (m_list.Size() == 1);

        for(int i = 0; i < m_list.Size(); ++i)            
        {
            m_list[i]->Send(dataFromTdm, canDetach);
        }
    }

    // --------------------------------------------------------

    void TdmChannelSender::PauseSend()
    {
        m_pause = true;
    }

    // --------------------------------------------------------

    void TdmChannelSender::Add( ISendDirection *p )
    {       
        m_pause = false;
        m_list.Add(p);
    }

    void TdmChannelSender::Delete( int index )
    {
        if (index < 0) 
        {
            ESS_THROW_T(TdmException, terNothingToStopSend);
        }

        m_list.Delete(index);
    }

}  // namespace TdmMng

