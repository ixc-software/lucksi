#ifndef __REMOTECALLIO__

#define __REMOTECALLIO__

#include "stdafx.h"

namespace Domain
{

    // �������� �����, � ������� �������� RPC �����
    class RemoteCallIO : public QDataStream
    {
        QBuffer m_buff;

    public:
        RemoteCallIO()  
        {
            setDevice(&m_buff);
        }
    };


}  // namespace Domain


#endif

