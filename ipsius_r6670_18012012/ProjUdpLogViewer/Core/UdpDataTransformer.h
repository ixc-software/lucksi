#ifndef __UDPDATATRANSFORMER__
#define __UDPDATATRANSFORMER__

#include "ISocketDataTransformer.h"
#include "UlvTypes.h"

namespace Ulv
{
    // Using for converting received from UDP socket data to QString
    class UdpDataTransformer :
        public ISocketDataTransformer
    {
        UdpPortDataType m_dataType;

        static QString TransformFromWSting(const QByteArray &data, bool *pOk);

    // ISocketDataTransformer impl
    private:
        QString Transform(boost::shared_ptr<iNet::SocketData> data, bool *pOk = 0);

    public:
        UdpDataTransformer(UdpPortDataType dataType) : m_dataType(dataType)
        {
        }
    };


    /*
    // -------------------------------------------------------------------

    // Using for converting received from UDP socket ASCII data to QString
    class AsciiDataTransformer :
        public ISocketDataTransformer
    {
    // ISocketDataTransformer impl
    private:
        QString Transform(boost::shared_ptr<iNet::SocketData> data)
        {
            return QString(data->getData());
        }

    public:
        AsciiDataTransformer()
        {
        }

    };

    // -------------------------------------------------------------------

    // Using for converting received from UDP socket UNICODE data to QString
    class UnicodeDataTransformer
    {
    // ISocketDataTransformer impl
    private:
        QString Transform(boost::shared_ptr<iNet::SocketData> data);

    public:
        UnicodeDataTransformer()
        {
        }
    };
    */

} // namespace Ulv


#endif
