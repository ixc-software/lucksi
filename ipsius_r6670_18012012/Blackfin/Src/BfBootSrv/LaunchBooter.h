#ifndef LAUNCHBOOTER_H
#define LAUNCHBOOTER_H

#include "BooterStartupParams.h"

// ������ ���������� ���������� ������

namespace BfBootSrv
{   
	class BoardSetup;

    // �������� ������.
    void LaunchBooter(const BooterStartupParams& params);

    // ������ ��� ���������. ������� ����� ������������� �������� ���� � �������������� BoardSetup � �����.
	void LaunchBooter(const BooterStartupParams& params, BoardSetup &boardSetup, bool initNetwork = false);	

} // namespace BfBootSrv

#endif
