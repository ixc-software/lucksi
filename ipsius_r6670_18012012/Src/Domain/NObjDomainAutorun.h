
#ifndef __NOBJDOMAINAUTORUN__
#define __NOBJDOMAINAUTORUN__

// NObjDomainAutorun.h

#include "IDomainAutorun.h"

#include "DRI/IDriFileExecuteDone.h"
#include "DRI/INonCreatable.h"
#include "DRI/FileDRI.h"

#include "Utils/ErrorsSubsystem.h"
#include "Utils/SafeRef.h"

#include "NamedObject.h"
#include "DomainCoutFormatter.h"

namespace DRI
{
    class FileDRI;

}  // namespace DRI

namespace Domain
{
    // Using to run Domain startup script. 
    // Log trace can be enabled from within script.
    class NObjDomainAutorun : 
        public NamedObject,
        public virtual Utils::SafeRefServer,
        public DRI::IFileExecuteDone,
        public DRI::IFileDRICoutTracer,
        public DRI::INonCreatable
    {
        Q_OBJECT
        
        IDomain &m_domain;
        Utils::SafeRef<IDomainAutorunToOwner> m_owner;
        boost::scoped_ptr<DRI::FileDRI> m_file;
        DomainCoutFormatter m_coutFormatter;
        bool m_enableTrace;
        
        int m_trimLineCharCount;
        int m_canPrintCharCount;
        
        void AsyncExit(const QString &err);
        QString TrimData(const QString &data, bool isFullLine);

    // IFileExecuteDone impl
    private:
        void OnFileExecuteDone(boost::shared_ptr<DRI::FileExecuteResult> result);

    // DRI setter/getters
    private:
        bool TraceToCout() const { return m_enableTrace; }
        void setTraceToCout(bool val) { m_enableTrace = val; }

    // IFileDRICoutTracer impl
    private:
        // can be on/off depend on m_enableTrace flag
        // void AddToOutput(const QString &data, bool trimIfNeed);
        void AddToOutput(const QString &data, bool lineFeed);

    // DRI interface
    public:
        Q_PROPERTY(bool TraceToCout READ TraceToCout WRITE setTraceToCout);
        
    public:
        NObjDomainAutorun(NamedObject *pParent, 
                          const ObjectName &name,
                          Utils::SafeRef<IDomainAutorunToOwner> owner,
                          bool stopOnFirstErr);
        
    };

    
} // namespace Domain

#endif
