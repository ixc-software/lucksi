#ifndef IL3CALL_H
#define IL3CALL_H

#include "Utils/IBasicInterface.h"
#include "DssUpIntf.h"



namespace ISDN
{    
    
    
    // Внутренний интерфейс для L3Call
    class IL3Call : public Utils::IBasicInterface
    {
    public:        
        
        virtual shared_ptr<const DssCallParams> GetOutCallParams() = 0;
        
        virtual const BChannelsWrapper& GetBCannels() const = 0;

        virtual void BChanBusyInSendSetup() = 0;        

        virtual void ReleaseConfirm() = 0;

        virtual void CloseCall( shared_ptr<const DssCause> pCause ) = 0; // инициатива удаления от фсм

        virtual BinderToDssCall GetBinder() = 0;

        virtual const ILoggable& getLogSession() = 0;

        virtual void AsyncSetupCompl() = 0;              

        virtual void LogCallInfo() = 0;

        virtual void SimpleLog(const char*, iLogW::LogRecordTag) = 0;
        
    };

} // ISDN

#endif

