
#ifndef __IUDPLOGCONTROLLERTOMODEL__
#define __IUDPLOGCONTROLLERTOMODEL__

#include "Utils/IBasicInterface.h"


namespace Ulv
{
    // class IModelToController;
    class CtrlToModelParams;
    // class UdpLogRecordSrc;
    
    class IControllerToModel : 
        public Utils::IBasicInterface
    {
    public:
        virtual void MsgRefreshView(boost::shared_ptr<CtrlToModelParams> params) = 0;
        virtual void MsgPauseView(bool state) = 0;
        virtual void MsgShutdown(/*Utils::SafeRef<IModelToController> controller*/) = 0;

        virtual void MsgCopyAllLogToClipboard() = 0;
        virtual void MsgCopyDisplayedLogToClipboard(int startRecord, int count) = 0;
        virtual void MsgSaveLogToFile(QString fileName) = 0;

        // virtual void MsgParamsChanged(boost::shared_ptr<CtrlToModelParams> params) = 0;
        virtual void MsgClearDB() = 0;
    };

} // namespace Ulv

#endif 


