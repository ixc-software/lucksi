#ifndef __ENVVARSINPATH__
#define __ENVVARSINPATH__

#include "ErrorsSubsystem.h"

namespace Utils
{
    
    struct EnvVarsInPath
    {

        static QString Insert(QString path)  // can throw 
        {
            const char *CMarker = "~";  // "%" conflict with isc macro syntax

            QString res = path;

            while(true)
            {
                int posBegin = res.indexOf(CMarker);
                if (posBegin < 0) break;
                int posEnd = res.indexOf(CMarker, posBegin + 1);
                if (posEnd < 0) 
                {
                    ESS_THROW_MSG(ESS::Exception, "Bad env in path " + path.toStdString());
                }

                QString env = res.mid(posBegin + 1, (posEnd - posBegin) - 1);
                const char *envVal = std::getenv( env.toStdString().c_str() );
                if (envVal == 0)
                {
                    ESS_THROW_MSG(ESS::Exception, "Can't find env " + env.toStdString());
                }

                res = res.mid(0, posBegin) + envVal + res.mid(posEnd + 1);

            }

            return res;
        }

    };

}  // namespace Utils

#endif