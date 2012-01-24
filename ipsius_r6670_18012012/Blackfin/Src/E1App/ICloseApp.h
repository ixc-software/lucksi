#ifndef ICLOSEAPP_H
#define ICLOSEAPP_H

namespace E1App
{
    class ICloseApp : Utils::IBasicInterface
    {
    public:
        virtual void Reboot(Utils::AtomicBool &breakFlag) = 0;        
    };
} // namespace E1App

#endif
