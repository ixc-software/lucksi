
#include "stdafx.h"
#include "UdpDataTransformer.h"
#include "UdpLogExceptions.h"

namespace Ulv
{
    QString UdpDataTransformer::TransformFromWSting(const QByteArray &data, bool *pOk)
    {
        QString res;

        // std::cout << "rb = " << data.size() << std::endl;

        int size = data.size();
        int step =  sizeof(wchar_t);
        if ((size % step) != 0)
        {
            // ESS_THROW(OddWStringBytesCount);
            if (pOk != 0) *pOk = false;

            return res;
        }

        int i = 0;
        while (i < size)
        {
            wchar_t ch = 0;
            for (int chI = (step - 1); chI >= 0; --chI)
            {
                wchar_t tmp = data.at(i);
                tmp <<= (chI * 8);
                ch |= tmp;

                ++i;
            }

            res += QChar(ch);
        }

        if (pOk != 0) *pOk = true;

        return res;
    }

    // -----------------------------------------------------------------------

    QString UdpDataTransformer::Transform(boost::shared_ptr<iNet::SocketData> data, bool *pOk)
    {
        switch (m_dataType)
        {
        case PTString:
            if (pOk != 0) *pOk = true;
            return QString(data->getData());

        case PTWstring:            
            return TransformFromWSting(data->getData(), pOk);

        default:
            ESS_HALT("Invalid UDP port data type");
        }

        return QString(); // never happens
    }

    // -----------------------------------------------------------------------


} // namespace Ulv
