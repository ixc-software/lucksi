#include "stdafx.h"

#include "Domain/NamedObject.h"

#include "DriPropertyFactory.h"
#include "MetaObjectsInfo.h"
#include "DriArgFactory.h"
#include "driutils.h"

// ------------------------------------------------

namespace
{
    using namespace DRI;

    ESS_TYPEDEF(PropertyError);
    ESS_TYPEDEF_FULL(BadEnum, PropertyError);
    ESS_TYPEDEF_FULL(BadTime, PropertyError);
    ESS_TYPEDEF_FULL(BadDate, PropertyError);
    ESS_TYPEDEF_FULL(BadDateTime, PropertyError);
    ESS_TYPEDEF_FULL(PropertyNotFound, PropertyError);
    ESS_TYPEDEF_FULL(PropertyReadOnly, PropertyError);

    QString CDateFormat("dd.MM.yyyy");
    QString CTimeFormat("hh:mm:ss");
    QString CDateTimeFormat = QString("%1 %2").arg(CDateFormat).arg(CTimeFormat);

    // -----------------------------------------------
            
    class EnumHandle : public IPropertyType
    {

    // IPropertyType impl
    private:

        bool Handle(const QMetaProperty &prop) const
        {
            return (prop.isEnumType()) && (!prop.enumerator().isFlag());
        }

        QVariant Set(const QMetaProperty &prop, const QString &val) const
        {
            QMetaEnum e = prop.enumerator();

            // int indx = e.keyToValue( val.toStdString().c_str() );
            if ( !DriUtils::CorrectEnumKey(e, val.toStdString().c_str()) ) 
            {                
                QString info = QString("For property %1 %2 can't found %3")
                    .arg(prop.name()).arg(MetaObjectsInfo::EnumToString(e)).arg(val);
                ESS_THROW_MSG(BadEnum, info.toStdString());
            }

            return QVariant(val);
        }

        QString Get(const QMetaProperty &prop, const QVariant &varValue) const
        {
            QMetaEnum e = prop.enumerator();

            int val = varValue.toInt();

            std::string result;
            const char *pCaption = e.valueToKey(val);
            if (pCaption == 0) result = "*BAD ENUM*";
            else result = pCaption;

            return result.c_str();            
        }

    };

    // -----------------------------------------------

    class EnumFlagsHandle : public IPropertyType
    {

    // IPropertyType impl
    private:

        bool Handle(const QMetaProperty &prop) const
        {
            return (prop.isEnumType()) && (prop.enumerator().isFlag());
        }

        QVariant Set(const QMetaProperty &prop, const QString &val) const
        {
            ESS_UNIMPLEMENTED;

            return QVariant();
        }

        QString Get(const QMetaProperty &prop, const QVariant &varValue) const
        {
            QMetaEnum e = prop.enumerator();

            int val = varValue.toInt();

            std::string result;

            QByteArray arr = e.valueToKeys(val);
            if (arr.isEmpty()) result = "[]";
            else result = arr.constData();

            return result.c_str();            
        }

    };

    // -----------------------------------------------

    // TType -- QDate/QTime/QDateTime
    template<class TType, class TException>
    QVariant CheckAndConvert(const QString &val,
                             const QString &format, 
                             const QString ttypeDesc)
    {
        TType t= TType::fromString(val, format);

        if (!t.isValid())
        {
            QString msg = QString("Invalid %1 or invalid %1 format(have to be '%2')")
                                   .arg(ttypeDesc).arg(format);
            ESS_THROW_MSG(TException, msg.toStdString());
        }

        return QVariant(t);
    }
    
    // -----------------------------------------------
    
    class TimeHandle : public IPropertyType
    {

    // IPropertyType impl
    private:

        bool Handle(const QMetaProperty &prop) const
        {
            return IsType(prop, "QTime");
        }

        QVariant Set(const QMetaProperty &prop, const QString &val) const
        {
            return CheckAndConvert<QTime, BadTime>(val, CTimeFormat, "time");
        }

        QString Get(const QMetaProperty &prop, const QVariant &varValue) const
        {
            QTime t = varValue.toTime();
            return t.toString(CTimeFormat);
        }

    };

    // -----------------------------------------------
    
    class DateHandle : public IPropertyType
    {

    // IPropertyType impl
    private:

        bool Handle(const QMetaProperty &prop) const
        {
            return IsType(prop, "QDate");
        }

        QVariant Set(const QMetaProperty &prop, const QString &val) const
        {
            return CheckAndConvert<QDate, BadDate>(val, CDateFormat, "date");
        }

        QString Get(const QMetaProperty &prop, const QVariant &varValue) const
        {
            QDate d = varValue.toDate();
            return d.toString(CDateFormat);
        }

    };

    // -----------------------------------------------
    
    class DateTimeHandle : public IPropertyType
    {

    // IPropertyType impl
    private:

        bool Handle(const QMetaProperty &prop) const
        {
            return IsType(prop, "QDateTime");
        }

        QVariant Set(const QMetaProperty &prop, const QString &val) const
        {
            return CheckAndConvert<QDateTime, BadDateTime>(val, CDateTimeFormat, 
                                                           "datetime");
        }

        QString Get(const QMetaProperty &prop, const QVariant &varValue) const
        {
            QDateTime dt = varValue.toDateTime();
            return dt.toString(CDateTimeFormat);
        }

    };

    // -----------------------------------------------

    class BoolHandle : public IPropertyType
    {

    // IPropertyType impl
    private:

        bool Handle(const QMetaProperty &prop) const
        {
            return IsType(prop, "bool");
        }

        QVariant Set(const QMetaProperty &prop, const QString &val) const
        {
            std::string error;
            bool setValue = DriArgFactory::StringToBool(val, error);

            if (!error.empty())
            {
                ESS_THROW_MSG(PropertyError, error);
            }

            return QVariant(setValue);
        }

        QString Get(const QMetaProperty &prop, const QVariant &varValue) const
        {
            bool v = varValue.toBool();
            return v ? "true" : "false";
        }

    };

    // -----------------------------------------------

    class DefaultHandle : public IPropertyType
    {

    // IPropertyType impl
    private:

        bool Handle(const QMetaProperty &prop) const
        {
            return true;
        }

        QVariant Set(const QMetaProperty &prop, const QString &val) const
        {
            return QVariant(val);
        }

        QString Get(const QMetaProperty &prop, const QVariant &varValue) const
        {
            QString s;
            s = varValue.toString();

            // quote string
            if (IsType(prop, "QString"))
            {
                s = "\"" + s + "\"";
            }

            return s;
        }

    };

    // -----------------------------------------------

    QMetaProperty FindProperty(const NamedObject *pObj, const QString &propName) // can throw
    {
        std::string name = propName.toStdString();

        const QMetaObject *pMeta = pObj->metaObject();
        int indx = pMeta->indexOfProperty(name.c_str());
        if (indx < 0) ESS_THROW(PropertyNotFound);

        return pMeta->property(indx);
    }

}  // namespace

// ------------------------------------------------

namespace DRI
{
        
    DriPropertyFactory::DriPropertyFactory()
    {
        Add( new EnumHandle() );            // enum
        Add( new EnumFlagsHandle() );       // flags
        Add (new BoolHandle() );            // bool

        Add( new TimeHandle() );            // QTime
        Add( new DateHandle() );            // QDate
        Add( new DateTimeHandle() );        // QDateTime

        Add( new DefaultHandle() );         // default, must be last
    }

    // ---------------------------------------------------------

    void DriPropertyFactory::Set( const QString &propName, NamedObject *pObj, const QString &val ) const
    {        
        try
        {
            QMetaProperty prop = FindProperty(pObj, propName);
            IPropertyType *p = Find(prop);
            ESS_ASSERT(p != 0);

            if (!prop.isWritable()) ESS_THROW(PropertyReadOnly);

            QVariant v = p->Set(prop, val);
            pObj->setProperty(prop.name(), v);
        }
        catch(const PropertyError &e)
        {
            throw;
        }
    }

    // ---------------------------------------------------------

    QString DriPropertyFactory::Get( const QString &propName, const NamedObject *pObj ) const
    {
        try
        {
            QMetaProperty prop = FindProperty(pObj, propName);
            IPropertyType *p = Find(prop);
            ESS_ASSERT(p != 0);

            QVariant v = prop.read(pObj);
            return  p->Get(prop, v);
        }
        catch(const PropertyError &e)
        {
            throw;
        }
        
    }

}  // namespace DRI

