#include "stdafx.h"
#include "DevControl.h"

namespace
{
    void Halt()
    {
        ESS_HALT("Can`t use this ModArmIo device on PC!");
    }
}

namespace ModArmIo
{
    DevControl::DevControl( const std::string &devFile, bool write )
    {
        Halt();
    }

    void DevControl::RunCmd( Detail::UserData &data )
    {
        Halt(); 
    }

    DevControl::~DevControl()
    {        
    }

} // namespace ModArmIo