
#ifndef __IUDPLOGMODELTOVIEW__
#define __IUDPLOGMODELTOVIEW__


#include "Utils/IBasicInterface.h"

namespace Ulv
{
    class Error;
    class UdpLogDBStatus;
    class ModelToViewParams;
    
    class IModelToView :
        public Utils::IBasicInterface
    {
    public:
        virtual void Update(boost::shared_ptr<ModelToViewParams> params) = 0;
        virtual void UpdateStatus(UdpLogDBStatus status) = 0;
        virtual void SaveToClipboard(QString data) = 0;

        virtual void DisplayError(Error error) = 0;
    };

} // namespace Ulv

#endif
