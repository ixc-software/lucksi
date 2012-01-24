#ifndef ULVUTILS_H
#define ULVUTILS_H

namespace UlvUtils
{

    // name format: name_number.extention
    // for absolute name use .exe path
    QString GenAbsFileName(const QString &exeRelativeDir,
                           const QString &namePrefix,
                           const QString &extention,
                           QString &err);

    void SaveToFile(const QString &file, const QString &data, bool append, QString &err);

} // namespace UlvUtils


#endif // ULVUTILS_H
