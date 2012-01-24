#ifndef __PROXYTEST_INTERFACE__

#define __PROXYTEST_INTERFACE__

#include "stdafx.h"
#include "Utils/IBasicInterface.h"

namespace CallModel
{

    // force moc to process this file
    // class ForceMoc_ProxyTestInterface : QObject { Q_OBJECT };

    // use as ProxyInterface
    class ITestInterface : public Utils::IBasicInterface 
        // public Domain::IRemoteInterface
    {
    public:
        virtual void Ping() = 0; 
        virtual void SetName(boost::shared_ptr<QString> name) = 0;
        virtual void SetNameVal(QString name) = 0;        	
    };

    /*
    namespace Detail
    {
        class IBasicTestIntf : public Domain::IRemoteInterface
        {
        public:
            virtual void BasicPrint() = 0;
        };
    }

    class IAdvTestIntf : public Detail::IBasicTestIntf
    {
    public:
        virtual void AdvancedPrint() = 0;
    }; */

}  // namespace CallModel


#endif
