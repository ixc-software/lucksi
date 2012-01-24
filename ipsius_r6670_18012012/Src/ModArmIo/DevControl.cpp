#include "stdafx.h"
#include "DevControl.h"

#include <fcntl.h>
#include <sys/ioctl.h>

namespace
{
    const bool CDebugNoIoctl = false;
}


namespace ModArmIo
{
    DevControl::DevControl( const std::string &devFile, bool write )
    {
        m_fd = open(devFile.c_str(), write ? O_RDWR : O_RDONLY);

        if (m_fd <= 0) ESS_THROW_MSG(Err, "Can`t open driver file");
    }

    void DevControl::RunCmd( Detail::UserData &data )
    {
        int rc = CDebugNoIoctl ? Detail::retOk : ioctl(m_fd, 0, &data);

        if (rc != Detail::retOk)
        {
            std::stringstream ss;
            ss << "Device driver return error code " << rc;
            ESS_THROW_MSG(Err, ss.str());
        }

        ESS_ASSERT( data.m_magic == Detail::CMagic );
    }

    DevControl::~DevControl()
    {
        close(m_fd);
    }
} // namespace ModArmIo
