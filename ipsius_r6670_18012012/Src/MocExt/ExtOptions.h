#ifndef __EXTOPTIONS__

#define __EXTOPTIONS__

    #include <QList>
    #include <QByteArray>
    #include "ExtMoc.h"

    namespace MocExt
    {
        bool ExtOptionsEnabled();

        void ProcessIncludes(QList<QByteArray> &includes);

    } // namespace MocExt

		
#endif



