#include "moc.h"
#include "ExtOptions.h"

namespace
{
    QByteArray TrimString(const QByteArray &s)
    {
        QByteArray res = s;

        // from begin
        while (!res.isEmpty())
        {
            char c = res.at(0);
            if (c > ' ') break;
            res.remove(0, 1);
        }

        // from end
        while (!res.isEmpty())
        {
            char c = res.at(res.size() - 1);
            if (c > ' ') break;
            res.remove(res.size() - 1, 1);
        }

        return res;
    }

    QByteArray GetEnvVar(const char *p)
    {
        QByteArray val = qgetenv(p);
        return TrimString(val);
    }

    QList<QByteArray> ListTrimAndRemoveEmpty(const QList<QByteArray> &list)
    {
        QList<QByteArray> res;

        for(int i = 0; i < list.size(); i++)
        {
            QByteArray s = TrimString(list.at(i));
            if (s.size()) res.append(s);
        }

        return res;
    }

}

namespace MocExt
{

    bool ExtOptionsEnabled()
    {
        return GetEnvVar("QTMOCADVANCED").toInt() != 0;
    }

    // ------------------------------------------


    void ProcessIncludes(QList<QByteArray> &includes)
    {
        if (!ExtOptionsEnabled()) return;

        QByteArray val = GetEnvVar("QTMOCINCLUDES");
        if (val.isEmpty()) return;

        QList<QByteArray> list = val.split(';');
        list = ListTrimAndRemoveEmpty(list);

        for(int i = 0; i < list.size(); i++)
        {
            includes.insert(0 + i, list.at(i));
        }
    }

}  // namespace MocExt



