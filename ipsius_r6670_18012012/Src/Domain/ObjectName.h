#ifndef __OBJECTNAME__
#define __OBJECTNAME__

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/ComparableT.h"
#include "Identificator.h"

namespace Domain
{


    /*
    Полное имя объекта представлено строкой в формате: 
    "DomainName:ObjectNameParent.ObjectName"  // пример 

    Рутовый объект в имени НЕ указываеться, т.к. он избыточен

    Разновидности имен
    1. Имя домена (класс DomainName), т.е. "DomainName" из примера
    2. Имя объекта (класс ObjectName), т.е. "ObjectNameParent.ObjectName" из примера
    3. Короткое имя объекта (класс ObjectName.GetShortName()), т.е. "ObjectName" из примера
    4. Имя родителя (класс ObjectName.GetParent()), т.е. "ObjectNameParent" из примера
    5. Полное имя объекта (класс FullObjectName, фактически DomainName + ObjectName), т.е. пример целиком

    */


    // ----------------------------------------------------------

    ESS_TYPEDEF(ObjectNameError);

    // имя домена
    class DomainName : public Utils::ComparableT<DomainName>
    {
        QString m_name;

        void Init(const QString &name)
        {
            if (!Identificator::CheckDomainName(name)) 
            {
                ESS_THROW_MSG(ObjectNameError, name.toStdString());
            }

            m_name = name;
        }

    public:
        DomainName(const QString &name)
        {
            Init(name);
        }

        DomainName(const char *pName)
        {
            Init(pName);
        }

        int Compare(const DomainName &other) const
        {
            return m_name.compare( other.m_name );
        }

        const QString& Name() const { return m_name; }
    };



    // ----------------------------------------------------------

    // имя объекта в домене 
    class ObjectName : public Utils::ComparableT<ObjectName>
    {
        QString m_name;

        void Init(const QString &name)
        {
            if (!Identificator::CheckObjectName(name)) 
            {
                ESS_THROW_MSG(ObjectNameError, name.toStdString());
            }

            m_name = name;
        }

    public:

        ObjectName(const char *pName) 
        {
            Init(pName);
        }

        ObjectName(const QString &name)
        {
            Init(name);
        }

        int Compare(const ObjectName &other) const
        {
            return m_name.compare( other.m_name );
        }

        QString Name(bool withRootFix = false) const 
        { 
            if ((withRootFix) && IsRoot()) return "ROOT";
            return m_name; 
        }

        bool IsEmpty() const { return m_name.isEmpty(); }
        bool IsRoot() const  { return IsEmpty(); }

        bool IsShort() const
        {
            return (ObjectLevel() == 1);
        }

        // уровень объекта, фактически -- количество коротких имен в имени объекта
        // для короткого имени уровень равен 1
        // для рутового объекта -- 0
        int ObjectLevel() const
        {
            return Identificator::ObjectNameLevel(m_name);
        }

        // return full parent name, i.e. return "Name1.Name2" for "Name1.Name2.Name3" 
        QString Parent() const
        {
            if (ObjectLevel() == 0) ESS_THROW_MSG(ObjectNameError, m_name.toStdString());

            return Identificator::GetObjectParent(m_name);
        }

        // return first object name, i.e. return "Name1" for "Name1.Name2.Name3" 
        QString GetHeadName() const
        {
            if (ObjectLevel() == 0) ESS_THROW_MSG(ObjectNameError, m_name.toStdString());

            int pos = m_name.indexOf(Identificator::CNameSeparator);

            QString s = (pos < 0) ? 
                m_name : m_name.left(pos);

            return s;
        }

        // return object name beyond first, i.e. return "Name2.Name3" for "Name1.Name2.Name3" 
        QString GetBeyondHead() const
        {
            int pos = m_name.indexOf(Identificator::CNameSeparator);
            if (pos < 0) return "";

            return m_name.mid(pos + 1);
        }

        // return short object name, i.e. return "Name3" for "Name1.Name2.Name3" 
        QString GetShortName() const
        {
            int pos = m_name.lastIndexOf(Identificator::CNameSeparator);

            if (pos < 0) return m_name;

            return m_name.mid(pos + 1);
        }

        // add parameter string before current name
        QString AddParent(const QString &parent) const 
        {
            if (IsRoot()) ESS_THROW_MSG(ObjectNameError, m_name.toStdString());

            QString result = parent;
            if (!result.isEmpty()) result += Identificator::CNameSeparator;
            result += m_name;

            return result;
        }

        QString AddParent(const ObjectName &parent) const 
        {
            return AddParent(parent.Name());
        }

        QString AddChild(const QString &child) const
        {
            QString s = m_name;

            if (child.size() > 0)
            {
                if (s.size() > 0) s += Identificator::CNameSeparator;
                s += child;
            }

            return s;
        }

        QStringList ToStringList() const
        {
            ObjectName n(m_name);

            QStringList result;
            while(true)
            {
                if (n.ObjectLevel() == 0)
                {
                    result.prepend("");
                    break;
                }

                result.prepend( n.GetShortName() );
                n = ObjectName( n.Parent() );
            }

            ESS_ASSERT( result.size() == (ObjectLevel() + 1) );
            return result;
        }

        
    };


    // ----------------------------------------------------------

    // полное имя объекта, включая имя домена
    class FullObjectName
    {
        DomainName m_domainName; 
        ObjectName m_objectName;

    public:

        /*
        FullObjectName(const char *pName)
            : m_domainName(ExtractDomain(pName)), m_objectName(ExtractName(pName))
        {
        } */

        FullObjectName(const DomainName &domainName, const ObjectName &objectName)
            : m_domainName(domainName), m_objectName(objectName)
        {
        }

        const ObjectName& Name() const
        {
            return m_objectName;
        }

        const DomainName& Domain() const
        {
            return m_domainName;
        }

    };


} // namespace Domain


#endif



