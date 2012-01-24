#ifndef GENERALBOOTERCONST_H
#define GENERALBOOTERCONST_H

#include "BroadcastSourcePort.h"

namespace BfBootCore
{
    // Udp - порт на котором клиент прослушивает broadcast.
    const Platform::word CBroadcastClientPort = 64334;
    const Platform::dword CBroadcastSendPeriod = 2000;

    // размер блока данных при прокачке данных  
    const int CAditionalReservedSize = 50; // CMaxCmdNameSize ?
    const int CSplitSizeForSendToBooterByTcp = 1024 * 15; // нарезка на уровне Sbp пакетов в сторону booter
    const int CMaxChankSize = 1024 * 64;  // максимальный размер бинарных данных в параметре команды

    const int CComRate = 38400;


} // namespace BfBootCore

#endif
