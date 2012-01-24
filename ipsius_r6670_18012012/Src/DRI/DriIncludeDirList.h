#ifndef __DRIINCLUDEDIRLIST__
#define __DRIINCLUDEDIRLIST__

// DriIncludeDirList.h

#include "Utils/ErrorsSubsystem.h"

namespace DRI
{
    /*
        TODO: sort pathes in list?
    */
    
    // List of pathes. 
    // Allow to add invalid pathes, but they can verified lated by using Verify()
    class DriIncludeDirList
    {
        QString m_exePath;
        QStringList m_list;
        bool m_debug;

        static void ThrowException(const QString &desc, const QString &src);
        void AddSinglePath(QString path); // can throw
        bool PathInList(const QString &path); // true if list contains this path

    public:
        ESS_TYPEDEF(InvalidPath);

        // pathList - list of pathes separeted by ';'
        // Default: .exe path
        // Do not check if path is valid!
        DriIncludeDirList(const QString &pathList = "");

        void Clear()
        {
            m_list.clear();
        }

        // Path is absolute or .exe-relative.
        // Do not check if path is valid(!)
        void Add(const QString &pathList); // can throw

        bool Verify(QStringList *pInvalidPathes = 0) const;
        
        // Path is absolute or relative to the pathes in list 
        QString Find(const QString &file, bool throwException = true) const;

        bool operator==(const DriIncludeDirList &other) const; 
        bool operator!=(const DriIncludeDirList &other) const { return !operator==(other); }

        // Added pathes + exe-path
        int Size() const { return m_list.size(); }

        QString ToString() const;

        static char PathSeparator();
    };

    
} // namespace DRI

#endif
