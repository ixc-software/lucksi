
#ifndef __DRITESTHELPERS__
#define __DRITESTHELPERS__

// DRITestHelpers.h

#include "Utils/SafeRef.h"
#include "Domain/DomainTestRunner.h"
#include "Domain/DomainClass.h"
#include "iCore/MsgObject.h"
#include "iLog/LogManager.h"

#include "DRI/FileDRI.h"
#include "DRI/Diff.h"
#include "DRI/drifileload.h"

#include "DriTestsParams.h"


namespace DriTestHelpers
{
    using namespace Domain;
    using namespace DRI;
    
    using boost::scoped_ptr;
    
    struct ScriptRunParams
    {
        QString ScpFile;
        QString ScpRefOutput;
        bool SilenceMode;

        // output
        QString Diffs;
        // bool OK;

        ScriptRunParams() : SilenceMode(true)/*, OK(true)*/ {}
    };

    // -----------------------------------------------------------------

    class ScriptRun : 
        public iCore::MsgObject,
        public IFileExecuteDone
    {
        DomainClass &m_domain;
        ScriptRunParams &m_params;
        scoped_ptr<FileDRI> m_file;

        void LoadScript();
        static QString LoadFile(QString fileName);
        
        class Comparator;

     // IFileExecuteDone impl
    private:
        void OnFileExecuteDone(shared_ptr<FileExecuteResult> result);

    public:
        ScriptRun(DomainClass &domain, ScriptRunParams &params);
        ~ScriptRun();

    };
    
    // -----------------------------------------------------------------

    void RunScript(const DriTests::DriTestsParams &params, 
                      QString scriptName, 
                      QString scriptRefOutput, 
                      const QStringList &specParams = QStringList(),
                      /* out */ QString *pError = 0);

    // -----------------------------------------------------------------
    
    void SetupLog(DomainClass &domain);

    
} // namespace DriTestHelpers


#endif
