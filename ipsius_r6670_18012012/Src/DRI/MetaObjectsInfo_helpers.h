#ifndef __METAOBJETCSINFO_HELPERS__
#define __METAOBJETCSINFO_HELPERS__

// WARNING! This files must be included in MetaObjectsInfo.cpp only

#include "Domain/NamedObject_helpers.h"

// Property helpers
namespace
{
    const char *CPropertyTypeSep = " : ";

    // --------------------------------------------------------------------

    QString GetPropertyEnumValue(const QObject *p, QMetaProperty prop)
    {
        QString s;

        ESS_ASSERT(prop.isEnumType());

        QMetaEnum e = prop.enumerator();

        int val = prop.read(p).toInt();

        std::string result;

        if (e.isFlag())
        {
            QByteArray arr = e.valueToKeys(val);
            if (arr.isEmpty()) result = "[]";
                          else result = arr.constData();
        }
        else
        {
            const char *pCaption = e.valueToKey(val);
            if (pCaption == 0) result = "*BAD ENUM*";
                          else result = pCaption;
        }

        return result.c_str();
    }

    // --------------------------------------------------------------------

    QString GetPropertyValue(const QObject *p, QMetaProperty prop)
    {
        QString s;

        s = prop.read(p).toString();

        // quote string
        if (prop.typeName() == std::string("QString"))
        {
            s = "\"" + s + "\"";
        }

        return s;
    }

    // --------------------------------------------------------------------

    QString GetPropertyDateTime(const QObject *p, QMetaProperty prop)
    {
        static QString CDateFormat("dd.MM.yyyy");
        static QString CTimeFormat("hh:mm:ss");

        std::string type = prop.typeName();

        if (type == "QDateTime") 
        {
            QDateTime dt = prop.read(p).toDateTime();
            return dt.toString(CDateFormat + " " + CTimeFormat);
        }

        return GetPropertyValue(p, prop);
    }

    // --------------------------------------------------------------------

    bool TypeIsDateTime(const std::string &type)
    {
        return (type == "QDateTime") || (type == "QDate") || (type == "QTime");
    }

    // --------------------------------------------------------------------

    QString SimplifyPropertyTypename(QString typeName)
    {
        int pos = typeName.lastIndexOf("::");

        if (pos >= 0)
        {
            typeName = typeName.mid(pos + 2);
        }

        return typeName;
    }
    
} // namespace

// --------------------------------------------------------------------

// Enum helpers
namespace 
{
    bool EnumExternType(QString className, QString typeName)
    {
        return (typeName.indexOf("::") >= 0);
    }

    // --------------------------------------------------------------------
    
    void AddEnumValues(Utils::StringList &sl, const QMetaObject &meta, bool addExternThruPropertySearch = true)
    {
        struct Inner
        {
            static void AddEnum(Utils::StringList &sl, const QMetaEnum &e)
            {
                if (e.isFlag()) return;

                for(int j = 0; j < e.keyCount(); ++j)
                {
                    AddUniqStringToList(sl, e.key(j));
                }
            }
        };

        int count = meta.enumeratorCount();

        for(int i = 0; i < count; ++i)
        {
            QMetaEnum e = meta.enumerator(i);
            Inner::AddEnum(sl, e);
        }

        if (!addExternThruPropertySearch) return;

        for(int i = 0; i < meta.propertyCount(); ++i)
        {
            QMetaProperty prop = meta.property(i);

            if (!EnumExternType(meta.className(), prop.typeName())) continue;

            if (prop.isFlagType() || prop.isEnumType())
            {
                Inner::AddEnum(sl, prop.enumerator());
            }

        }
    }

}  // namespace

// --------------------------------------------------------------------

// Other helpers
namespace 
{

    bool TypeIsNonCreatable(QString typeName)
    {
        const DRI::INamedObjectType *pMeta = 
            DRI::NamedObjectFactory::Instance().Find( typeName.toStdString() );
        if (pMeta == 0) return true;

        return !(pMeta->AllowCreate());
    }
    
} // namespace

#endif

