
#ifndef __UDPLOCALPOINT__
#define __UDPLOCALPOINT__

// UdpLocalPoint.h

#include "ISocketDataTransformer.h"
#include "UlvTypes.h"
#include "UlvError.h"
#include "UdpLogExceptions.h"

#include "iNet/MsgUdpSocket.h"

namespace Ulv
{
    // Call-back interface to UdpLocalPoint user
    class IUdpLocalPointEvents :
        public Utils::IBasicInterface
    {
    public:
        virtual void NewData(const QString &data, const Utils::HostInf &src) = 0;
        virtual void ErrorOccur(boost::shared_ptr<Error> error) = 0;
    };

    // ----------------------------------------------------------------------

    // UDP socket + type of received through this socket data
    class UdpLocalPoint : 
        public virtual Utils::SafeRefServer,
        public iNet::IUdpSocketToUser,
        boost::noncopyable
    {
        IUdpLocalPointEvents &m_owner;
        boost::scoped_ptr<ISocketDataTransformer> m_dataTransformer;

        iNet::MsgUdpSocket m_socket;

        // pErr deleted by shared_ptr
        boost::shared_ptr<Error> WrappErr(Error *pErr)
        {
            boost::shared_ptr<Error> res;
            res.reset(pErr);

            return res;
        }

    // iNet::IUdpSocketToUser impl
    private:
        void ReceiveData(boost::shared_ptr<iNet::SocketData> data)
        {
            /*
            try
            {
                m_owner.NewData(m_dataTransformer->Transform(data), data->getHostinf());
            }
            catch(OddWStringBytesCount &e)
            {
                m_owner.ErrorOccur(WrappErr(new WStringBytesCountError));
            }
            */
            bool ok = false;
            QString strData = m_dataTransformer->Transform(data, &ok);

            if (ok)
            {
                m_owner.NewData(strData, data->getHostinf());
                return;
            }

            m_owner.ErrorOccur(WrappErr(new WStringBytesCountError));
        }

        void SocketErrorOccur(boost::shared_ptr<iNet::SocketError> error)
        {
            m_owner.ErrorOccur(WrappErr(new UdpSocketError(error)));
        }

    public:
        // Managing pDataTansformer destruction
        UdpLocalPoint(iCore::MsgThread &thread, IUdpLocalPointEvents &owner,
                      ISocketDataTransformer *pDataTansformer,
                      const Utils::HostInf &host) :
            m_owner(owner), m_dataTransformer(pDataTansformer), m_socket(thread, this)
        {
            /*try
            {
                m_socket.Bind(host);
            }
            catch(iNet::UdpBindExcept &e)
            {
                m_owner.ErrorOccur(WrappErr(new UdpSocketBindError));
            }*/


            if (m_socket.Bind(host)) return;

            m_owner.ErrorOccur(WrappErr(new UdpSocketBindError));
        }

    };

} // namespace Ulv

#endif 

