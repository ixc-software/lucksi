#pragma once

#include "stdafx.h"
#include "ModArmIoDetail.h"
#include "Utils/ErrorsSubsystem.h"

namespace ModArmIo
{

    class DevControl
    {
        int m_fd;

    public:       
        ESS_TYPEDEF(Err);

        DevControl(const std::string &devFile, bool write);
        ~DevControl();

        void RunCmd(Detail::UserData &data);
    };
    

} // namespace ModArmIo

