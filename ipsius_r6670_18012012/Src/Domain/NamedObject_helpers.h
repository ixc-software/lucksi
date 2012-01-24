#ifndef __NAMEDOBJECT_HELPERS__
#define __NAMEDOBJECT_HELPERS__

// WARNING! This file must be included in NamedObject.cpp only

namespace
{
    using namespace Domain;
    using DRI::DriPropertyFactory;
    using DRI::DriArgFactory;
    using DRI::ExecutiveDRI;


    /*
        This marker used for
            - read-only properties
            - non delete instances
            - non create types */
    const QChar CReadOnlyMarker = '#';

    // -------------------------------------------------------

    const QChar CIndentChar = ' ';
    const int CIndentSize = 4;
    
    // -------------------------------------------------------

    bool ObjectIsNonCreatable(const NamedObject *pObj)
    {
        ESS_ASSERT(pObj != 0);

        const DRI::INonCreatable *p = dynamic_cast<const DRI::INonCreatable*>(pObj);
        return (p != 0);
    }

    // -------------------------------------------------------

    void AddUniqStringToList(Utils::StringList &sl, const QString &s)
    {
        ESS_ASSERT( !s.isEmpty() );

        if (sl.indexOf(s) >= 0) return;

        sl.append(s);
    }

    // -------------------------------------------------------

    bool ReadRawStringProperty(QObject *pObj, const QString &propertyName, QString &result)
    {
        const std::string propName = propertyName.toStdString();

        const QMetaObject *pMeta = pObj->metaObject();

        int indx = pMeta->indexOfProperty(propName.c_str());
        if (indx < 0) return false;

        QMetaProperty prop = pMeta->property(indx);
        if (prop.typeName() != std::string("QString")) return false;
        if (!prop.isReadable()) return false;

        result = pObj->property(propName.c_str()).toString();
        return true;
    }

    // -------------------------------------------------------
    
    int FindMethodByName(const QMetaObject *pMeta, QString methodName)
    {
        for(int i = 0; i < pMeta->methodCount(); ++i)
        {
            QString currName = DRI::DriUtils::MethodName(pMeta->method(i)).c_str();
            if (currName == methodName) return i;
        }

        return -1;
    }

    // -------------------------------------------------------

    QString MakeIndent(int level)
    {
        return QString(level * CIndentSize, CIndentChar);
    }

}  // namespace


#endif
