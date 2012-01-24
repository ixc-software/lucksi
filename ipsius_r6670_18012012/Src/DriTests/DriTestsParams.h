
#ifndef __DRITESTSPARAMS__
#define __DRITESTSPARAMS__

// DriTestsParams.h

namespace DriTests
{
    struct DriTestsParams
    {
        bool SilentMode;
        QString ScriptDirs;
        int MaxDiffLines;

        DriTestsParams(bool silentMode = true, 
                       const QString &scriptDirs = "../isc/",
                       int maxDiffLines = 8) : 
            SilentMode(silentMode), ScriptDirs(scriptDirs), MaxDiffLines(maxDiffLines)
        {
        }
    };
    
} // namespace DriTests

#endif
