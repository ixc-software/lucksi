#ifndef __DOMAINTESTRUNNER__
#define __DOMAINTESTRUNNER__

#include "stdafx.h"

#include "Utils/IBasicInterface.h"
#include "Utils/ExeName.h"

#include "DomainRunner.h"
#include "DomainStartup.h"


namespace Domain
{

    // интерфейс воздействия на DomainTestRunner
    class IDomainTestRunnerControl : public Utils::IBasicInterface
    {
    public:
        // ...
    };

    class IDomainTestRunnerSupport : public Utils::IBasicInterface
    {
    public:
        virtual void SetDomainTestRunnerControl(IDomainTestRunnerControl &control) = 0;
    };

    // -----------------------------------------------------------------
	
    class DomainTestRunner : public IDomainTestRunnerControl
    {
        DomainStartup m_startup;

        // nothing, overloaded for type detection
        void BindControl(void const *pParams) {}

        void BindControl(IDomainTestRunnerSupport *pParams)
        {
            ESS_ASSERT(pParams);
            pParams->SetDomainTestRunnerControl(*this);
        }

        bool TryRun();

        void SetupParams(int timeout);
        
    // IDomainTestRunnerControl impl
    private:

        // ...

    public:

        DomainTestRunner(int timeout = -1);
        DomainTestRunner(const iLogW::LogSettings &sett, int timeout = -1);
        DomainTestRunner(const QStringList &specialParams, int timeout = -1);

        DomainStartup& Params() { return m_startup; }

        // TObject constructor must be TObject(DomainClass &domain, TParams &params)
        // if TParams implements IDomainTestRunnerSupport, TObject can get access to IDomainTestRunnerControl
        template<class TObject, class TParams>
        bool Run(TParams &params)
        {
            BindControl(&params);
            DomainInnerObject<TObject, TParams> inner(params);
            m_startup.DomainInnerCreate(&inner);

            return TryRun();
        }

    };
	
}  // namespace Domain

#endif

