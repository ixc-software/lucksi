#ifndef SENDTOTDM_H
#define SENDTOTDM_H

#include "ISendDirection.h"
#include "ITdmManager.h"
#include "DataPacket.h"

namespace TdmMng
{
    // Send to TDM
    class SendToTdm : 
        public ISendDirection,
        boost::noncopyable
    {
        DirectionID m_id;
        ITdmIn &m_in;        

    // ISendDirection impl:
    private:

        void Send(DataPacket &pack, bool canDetach)
        {
            m_in.OnData(pack/*, bool canDetach*/);
        }

        bool Equal(const std::string &ip, int port) const
        {
            return m_id.Equal(ip, port);
        }

        bool Equal(const Utils::SafeRef<ISendSimple> &ref) const
        {
            return false;
        }

    public:

        SendToTdm(const std::string &ip, int port, ITdmIn &in)
            : m_id(ip, port), m_in(in)            
        {
            m_in.OnConnect();
        }

        ~SendToTdm()
        {
            m_in.OnDisconnect();
        }
    };
} // namespace TdmMng

#endif
