#ifndef __IDENTIFICATOR__

#define __IDENTIFICATOR__

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"

namespace Domain
{

    class Identificator
    {

        static bool Check(const QString &name)
        {
            if (name.isEmpty()) return false;

            for(int i = 0; i < name.size(); i++)
            {
                QChar c = name.at(i);
                bool insideLo = (c >= 'a') && (c <= 'z');
                bool insideHi = (c >= 'A') && (c <= 'Z');
                bool special = (c == '_');
                bool number = (i > 0) ? ((c >= '0') && (c <= '9')) : false;
                if (!(insideLo || insideHi || special || number)) return false;
            }

            return true;
        }

        /*
        static bool Check(const std::string &name)
        {
            return Check(QString(name.c_str()));
        } */

    public:

        // extract full name like "Domain:Name.Name"
        static bool ExtractFullName(const QString &name, 
            QString &domain, QString &objectName)
        {
            // extract domain
            int pos = name.indexOf(CDomainSeparator);
            if (pos < 0) return false;

            domain = name.left(pos);
            if (!CheckDomainName(domain)) return false;

            // extract name
            objectName = name.mid(pos + 1);
            if (!CheckObjectName(objectName)) return false;

            return true;
        }

        static bool CheckObjectName(const QString &name)
        {
            if (name.isEmpty()) return true;

            QStringList list = name.split( QChar(CNameSeparator) );

            for(int i = 0; i < list.size(); i++)
            {
                if (!Check(list.at(i))) return false;
            }

            return true;
        }

        static bool CheckDomainName(const QString &name)
        {
            return Check(name);
        }

        static int ObjectNameLevel(const QString &name)
        {
            if (name.isEmpty()) return 0;  // root
            return 1 + name.count( QChar(CNameSeparator) );
        }

        static QString GetObjectParent(const QString &name)
        {
            int pos = name.lastIndexOf(CNameSeparator);
            if (pos < 0) return "";
            return name.left(pos);
        }

        enum
        {
            CDomainSeparator = ':',
            CNameSeparator = '.',
        };

    };
}

#endif

