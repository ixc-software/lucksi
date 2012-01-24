#ifndef __IRTP__
#define __IRTP__

#include "Utils/IBasicInterface.h"

namespace Rtp
{
    // User receiver callbak interface
    class IRtpUserReceiver : public Utils::IBasicInterface
    {
    public:
        virtual void ReceiveData(const QByteArray &data, bool marker, unsigned long tStamp) = 0;
        virtual void ReceiveEvent(int event, char key, int duration, 
            int volume, unsigned int timestamp) = 0;
    };

} // namespace

#endif
