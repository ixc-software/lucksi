#ifndef __REMOTECALLIO__

#define __REMOTECALLIO__

#include "stdafx.h"

namespace Domain
{

    // Двоичный поток, в котором хранится RPC вызов
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

