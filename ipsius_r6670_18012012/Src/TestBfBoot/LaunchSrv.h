#ifndef LAUNCHSRV_H
#define LAUNCHSRV_H

#include "BfBootSrv/LaunchBooter.h"

namespace TestBfBoot
{
    class LaunchSrv // ��� ������� ������� � ������ ���� � ������ ThreadTaskRunner
    {
    public:
        LaunchSrv(const boost::shared_ptr<BfBootSrv::BooterStartupParams> param)
        {
            BfBootSrv::LaunchBooter(*param);
        }
    };   
} // namespace TestBfBoot

#endif
