#ifndef __ISOCKETDATATRANSFORMER__
#define __ISOCKETDATATRANSFORMER__

#include "Utils/IBasicInterface.h"
#include "iNet/SocketData.h"


namespace Ulv
{
    // To convert QByteArray to QString
    class ISocketDataTransformer : public Utils::IBasicInterface
    {
    public:
        virtual QString Transform(boost::shared_ptr<iNet::SocketData> data, bool *pOk = 0) = 0;
    };


} // namespace Ulv

#endif
