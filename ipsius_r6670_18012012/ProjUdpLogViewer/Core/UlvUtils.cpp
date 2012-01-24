
#include "stdafx.h"

#include "UlvUtils.h"
#include "Utils/ExeName.h"
#include "Utils/ErrorsSubsystem.h"

namespace
{
    const QString COpenError = "Can't open file '%1'";
    const QString CResizeError = "Can't resize file '%1'";
    const QString CWriteError = "Writing to file '%1' failed";

} // namespace

// ---------------------------------------------------------------------------

namespace UlvUtils
{
    QString GenAbsFileName(const QString &exeRelativeDir,
                           const QString &namePrefix,
                           const QString &extention,
                           QString &err)
    {
        QString dt = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
        QString dirStr = QDir::cleanPath(Utils::ExeName::GetExeDir().c_str() + exeRelativeDir);

        QDir dir(dirStr);
        if (!dir.exists())
        {
            if (!dir.mkpath(dirStr)) err = QString("Failed to create dir '%1'").arg(dirStr);
        }

        QString name = QString("%1/%2_%3.%4").arg(dir.absolutePath()).arg(namePrefix).arg(dt).arg(extention);

        return name;
    }

    // ---------------------------------------------------------------------------

    void SaveToFile(const QString &file, const QString &data, bool append, QString &err)
    {
        err.clear();
        QFile f(file);

        if (append) ESS_ASSERT(f.exists());

        QIODevice::OpenMode openMode = append
                                       ? (QIODevice::Append | QIODevice::Text)
                                       : (QIODevice::WriteOnly | QIODevice::Text);
        if (!f.open(openMode))
        {
            err = COpenError.arg(file);
            return;
        }

        if (!append)
        {
            if (!f.resize(0))
            {
                err = CResizeError.arg(file);
                return;
            }
        }

        if (f.write(data.toAscii()) == data.size()) return;

        err = CWriteError.arg(file);
    }

} // namespace UlvUtils
