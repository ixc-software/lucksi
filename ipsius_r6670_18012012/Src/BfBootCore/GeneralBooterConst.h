#ifndef GENERALBOOTERCONST_H
#define GENERALBOOTERCONST_H

#include "BroadcastSourcePort.h"

namespace BfBootCore
{
    // Udp - ���� �� ������� ������ ������������ broadcast.
    const Platform::word CBroadcastClientPort = 64334;
    const Platform::dword CBroadcastSendPeriod = 2000;

    // ������ ����� ������ ��� �������� ������  
    const int CAditionalReservedSize = 50; // CMaxCmdNameSize ?
    const int CSplitSizeForSendToBooterByTcp = 1024 * 15; // ������� �� ������ Sbp ������� � ������� booter
    const int CMaxChankSize = 1024 * 64;  // ������������ ������ �������� ������ � ��������� �������

    const int CComRate = 38400;


} // namespace BfBootCore

#endif
