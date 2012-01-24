
#include "stdafx.h"
#include "DriIncludeDirList.h"
#include "Utils/QtHelpers.h"
#include "Utils/ExeName.h"
#include "Platform/Platform.h"


namespace DRI
{

    DriIncludeDirList::DriIncludeDirList(const QString &pathList)
    {
        m_debug = false;
        
        m_exePath = QString::fromStdString(Utils::ExeName::GetExeDir());
        if (m_debug) std::cout << "Exe path = " << m_exePath.toStdString() << std::endl;
        
        if (m_debug) std::cout << "DriIncludeDirList(" << pathList << ")" << std::endl;
        
        if (!QDir::isAbsolutePath(m_exePath)) ESS_HALT(m_exePath.toStdString().c_str());
        AddSinglePath(m_exePath);
        
        Add(pathList);
    }

    // -----------------------------------------------------------------

    void DriIncludeDirList::ThrowException(const QString &desc, const QString &src)
    {
        QString msg = QString("%1: \"%2\"").arg(desc).arg(src);
        
        ESS_THROW_MSG(InvalidPath, msg.toStdString());
    }
    
    // -----------------------------------------------------------------

    void DriIncludeDirList::AddSinglePath(QString path) // can throw
    {
        path = path.trimmed();
        if (path.isEmpty()) return;

        if (QDir::isRelativePath(path)) path.prepend(m_exePath);
        path = QDir::cleanPath(path);
        path.append(Platform::PathSeparator());

        // if (!QDir(path).exists()) ThrowException("Dirrectory doesn't exists", path);

        // if not in list already
        if (!PathInList(path))
        {
            if (m_debug) std::cout << "Add path: " << path << std::endl;
            
            m_list.append(path); 
        }
    }
    
    // -----------------------------------------------------------------

    void DriIncludeDirList::Add(const QString &pathList) // can throw
    {
        if (m_debug) std::cout << "Add(" << pathList << ")" << std::endl;
        
        if (pathList.isEmpty()) return;

        QStringList sl = pathList.split(PathSeparator(), QString::SkipEmptyParts);
        
        for (int i = 0; i < sl.size(); ++i)
        {
            AddSinglePath(sl.at(i));
        }
    }

    // -----------------------------------------------------------------

    bool DriIncludeDirList::PathInList(const QString &path)
    {
        for (int i = 0; i < m_list.size(); ++i)
        {
            if (m_list.at(i) == path) return true;
        }

        return false;
    }
    
    // -----------------------------------------------------------------
    
    /*QString DriIncludeDirList::Find(const QString &file, bool throwException) const
    {
        if ((QFileInfo(file).isAbsolute()) && (QFile::exists(file))) return file;

        if (m_debug) std::cout << "List size = " << m_list.size() << std::endl;
        
        // try to find in list
        for (int i = 0; i < m_list.size(); ++i)
        {
            QString absPath(m_list.at(i));
            absPath.append(file);
            absPath = QDir::cleanPath(absPath);

            if (m_debug) std::cout << "Absolute path: " << absPath << std::endl;

            if (QFile::exists(absPath)) return absPath;
        }

        if (throwException) 
        {
            ThrowException("Path wasn't found in list or file doesn't exist", file);
        }

        return QString();
    }*/

    // -----------------------------------------------------------------

    QString DriIncludeDirList::Find(const QString &file, bool throwException) const
    {
        if (m_debug) std::cout << "Find(" << file << ")" << std::endl;
        if (m_debug) std::cout << "List size = " << m_list.size() << std::endl;
        
        if ((QFileInfo(file).isAbsolute()) && (QFile::exists(file))) 
        {
            // this path can't have dublicates
            return file;
        }

        // try to find in list
        QStringList found;
        QStringList notFound;
        for (int i = 0; i < m_list.size(); ++i)
        {
            QString absPath(m_list.at(i));
            absPath.append(file);
            absPath = QDir::cleanPath(absPath);
            
            if (m_debug) std::cout << "Processed path: " << m_list.at(i) << std::endl;
            if (m_debug) std::cout << "Absolute path: " << absPath << std::endl;

            // dir/dir1/ + ../dir2/file.txt -> dir/dir2/file.txt
            // dir/dir2/ + ../dir2/file.txt -> dir/dir2/file.txt
            if (QFile::exists(absPath))
            {
                 if (!found.contains(absPath)) found.push_back(absPath);
            }
            else notFound.push_back(absPath);
        }

        if (throwException) 
        {
            if (found.isEmpty())
            {
                QString src("Path wasn't found in list or file doesn't exist");
                ThrowException(src, notFound.join("; "));
            }

            if (found.size() > 1)
            {
                QString src = QString("Found more than one path for file '%1'").arg(file);
                ThrowException(src, found.join("; "));
            }
        }

        // return first found
        return (found.isEmpty())? QString() : found.at(0);
    }
    
    // -----------------------------------------------------------------
    
    bool DriIncludeDirList::operator==(const DriIncludeDirList &other) const
    {
        return ((m_list == other.m_list) && (m_exePath == other.m_exePath));
    }

    // -----------------------------------------------------------------

    char DriIncludeDirList::PathSeparator()
    {
        return ';';
    }

    // -----------------------------------------------------------------

    QString DriIncludeDirList::ToString() const
    {
        return QString(m_list.join(QChar(PathSeparator())));
    }

    // -----------------------------------------------------------------

    bool DriIncludeDirList::Verify(QStringList *pInvalidPathes) const
    {
        if (pInvalidPathes != 0) pInvalidPathes->clear();

        bool res = true;
        for (int i = 0; i < m_list.size(); ++i)
        {
            if (!QDir(m_list.at(i)).exists())
            {
                res = false;
                if (pInvalidPathes == 0) return res;

                QString shortPath(m_list.at(i));
                shortPath.remove(m_exePath);
                
                QString str = QString("%1 (%2)").arg(shortPath)
                                .arg(QDir::cleanPath(m_list.at(i)));
                
                pInvalidPathes->push_back(str);
            }
        }

        return res;
    }
   
} // namespace DRI
