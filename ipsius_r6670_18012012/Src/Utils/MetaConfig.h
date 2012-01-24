#ifndef __METACONFIG__
#define __METACONFIG__

#include "stdafx.h"
#include <QString>
#include "ErrorsSubsystem.h"
#include "IBasicInterface.h"
#include "StringList.h"
#include "Platform/Platform.h"


/*
    TODO
    - complete tests set

*/

namespace Utils
{

    // Reading data from file. 
    // Data format: "<name> = <value>  // comments "
    // Multi-line option's argument could be write using '\' at the end of each line,
    // non-quoted lines will be trimmed, to avoid this use '"'
    class MetaConfigLoader : boost::noncopyable
    {
        struct Pair
        {
            QString Name;
            QString Value;
        };

        QVector<Pair> m_pairs;

        void Load(StringList sl, bool namesIgnoreCase);

        bool NameExists(QString name, bool namesIgnoreCase)
        {
            if (namesIgnoreCase) name = name.toUpper();

            for(int i = 0; i < m_pairs.size(); ++i)
            {
                QString s = m_pairs.at(i).Name;
                if (namesIgnoreCase) s = s.toUpper();
                if (s == name) return true;
            }

            return false;
        }

    public:

        ESS_TYPEDEF(SyntaxError);
        ESS_TYPEDEF(DublicateName);

        MetaConfigLoader(QString fileName, bool namesIgnoreCase = true)  // can throw
        {
            StringList sl;
            sl.LoadFromFile(fileName);
            Load(sl, namesIgnoreCase);
        }

        MetaConfigLoader(StringList sl, bool namesIgnoreCase = true)  // can throw
        {
            Load(sl, namesIgnoreCase);
        }


        int getCount() const
        {
            return m_pairs.size();
        }

        QString NameAt(int indx) const
        {
            return m_pairs.at(indx).Name;
        }

        QString ValueAt(int indx) const
        {
            return m_pairs.at(indx).Value;
        }
        
    };

    // -------------------------------------------------------

    // преобразователь T <-> QString, работает через специализацию шаблона 
    template<class T>
    class FieldConv
    {
    public:
        static void fromString(QString s, T &field);
        static QString toString(const T &field);
        static QString getTypeName();
    };


    // -------------------------------------------------------

    // интерфейс доступа к ConfigField<T> -- отвязка от шаблонности, переход к полиморфизму
    class IConfigField : public IBasicInterface, boost::noncopyable
    {
    public:
        virtual QString getName() const = 0;
        virtual QString getDescription() const = 0;
        virtual QString getInfo() const = 0;
        virtual QString getInSaveFormat() const = 0;

        virtual QString get() const = 0;
        virtual void set(QString val) = 0;
    };

    // поле конфигурационного файла типа T
    template<class T>
    class ConfigField : public IConfigField
    {
        T &m_field; 
        QString m_name; 
        QString m_desc;

    // IConfigField impl
    private:

        QString getName() const
        {
            return m_name;
        }

        QString getDescription() const
        {
            return m_desc;
        }

        QString get() const
        {
            return FieldConv<T>::toString(m_field);
        }

        void set(QString val)
        {
            FieldConv<T>::fromString(val, m_field);
        }

        QString getInfo() const
        {
            QString s = m_name + " (" + FieldConv<T>::getTypeName() + ")";
            if (m_desc.size() > 0) s += " -- " + m_desc;
            return s;
        }

        QString getInSaveFormat() const
        {
            QString res = m_name + " = " + get();
            if (m_desc.size() > 0) res += " // " + m_desc;
            return res;
        }


    public:

        ConfigField(T &field, QString name, QString desc)
            : m_field(field), m_name(name), m_desc(desc)
        {
            ESS_ASSERT(name.size() > 0);
        }        

    };

    // -------------------------------------------------------

    class CommentField : public IConfigField
    {
        QString m_comment;

        // IConfigField impl
    private:

        QString getName() const
        {
            return "";
        }

        QString getDescription() const
        {
            return m_comment;
        }

        QString get() const
        {
            return "";
        }

        void set(QString val)
        {
            ;
        }

        QString getInfo() const
        {
            return "";
        }

        QString getInSaveFormat() const
        {
            return m_comment;
        }

    public:
        CommentField(QString comment)
            : m_comment(comment)
        {
            ESS_ASSERT(comment.size() > 0);
        }        
    };

    // -------------------------------------------------------

    // работа с файлами конфига
    class MetaConfig
    {
        typedef QVector<IConfigField*> List;

        List m_fields;
        bool m_completed;  // формирование полей в классе завершено
        bool m_namesIgnoreCase;

        void FreeFields()
        {
            for(int i = 0; i < m_fields.size(); ++i)
            {
                delete m_fields.at(i);
            }

            m_fields.clear();
        }

        IConfigField* FindByName(QString name, bool namesIgnoreCase)
        {
            if (namesIgnoreCase) name = name.toUpper();

            for(int i = 0; i < m_fields.size(); ++i)
            {
                QString s = m_fields.at(i)->getName();
                if (namesIgnoreCase) s = s.toUpper();

                if (s == name) return m_fields.at(i);
            }

            return 0;
        }

        void operator=(const MetaConfig& src);  // forbid

    public:

        ESS_TYPEDEF(BadConversion);
        ESS_TYPEDEF(UnknownFieldName);

        MetaConfig(bool namesIgnoreCase = true) : 
			m_fields(), m_completed(false), m_namesIgnoreCase(namesIgnoreCase)
        {
        }

        MetaConfig(const MetaConfig& src) : 
			m_fields(), m_completed(false), m_namesIgnoreCase(src.m_namesIgnoreCase)
        {
            // does nothing, new copy creates empty...
        }

        ~MetaConfig()
        {
            FreeFields();
        }

        template<class T>
        void Add(T &field, QString name, QString desc = "")
        {
            ESS_ASSERT(!m_completed);
            ESS_ASSERT(FindByName(name, m_namesIgnoreCase) == 0);
            m_fields.push_back( new ConfigField<T>(field, name, desc) );
        }

        void Add(QString comment)
        {
            ESS_ASSERT(!m_completed);
            m_fields.push_back( new CommentField(comment));
        }

        static void ThrowBadConversionFromString(QString s)
        {
            QString msg = "String: " + s;
            ESS_THROW_MSG(BadConversion, msg.toStdString());
        }

        QString ListAllFields() const
        {
            ESS_ASSERT(m_completed);
            QString msg;

            for(int i = 0; i < m_fields.size(); ++i)
            {
                msg += m_fields.at(i)->getInfo() + "\n";
            }

            return msg;
        }

        void LoadFrom(const MetaConfigLoader &loader); // can throw

        void LoadFromFile(QString fileName)  // can throw
        {
            MetaConfigLoader loader(fileName);
            LoadFrom(loader);
        }

        void SaveTo(StringList &sl) const; 

        void SaveToFile(QString name) // can throw
        {
            StringList sl;
            SaveTo(sl);
            sl.SaveToFile(name);
        }

        static void RunTest(bool silenceMode = false);

        bool getCompleted() const { return m_completed; }

        void Complete()
        {
            ESS_ASSERT(!m_completed);
            m_completed = true;
        }

    };

    // -------------------------------------------------------

    template<>
    class FieldConv<int>
    {
    public:
        static void fromString(QString s, int &field)
        {
            bool ok;
            field = s.toInt(&ok);
            if (!ok) MetaConfig::ThrowBadConversionFromString(s);
        }

        static QString toString(const int &field)
        {
            return QString::number(field);
        }

        static QString getTypeName()
        {
            return "integer";
        }

    };

    template<>
    class FieldConv<Platform::word>
    {
    public:
        static void fromString(QString s, Platform::word &field)
        {
            bool ok;
            field = s.toUInt(&ok);
            if (!ok) MetaConfig::ThrowBadConversionFromString(s);
        }

        static QString toString(const Platform::word &field)
        {
            return QString::number(field);
        }

        static QString getTypeName()
        {
            return "platform word";
        }

    };

    // -------------------------------------------------------

    template<>
    class FieldConv<QString>
    {
    public:
        static void fromString(QString s, QString &field)
        {
            field = s;
        }

        static QString toString(const QString &field)
        {
            return field;
        }

        static QString getTypeName()
        {
            return "string";
        }

    };

    template<>
    class FieldConv<bool>
    {
    public:
        static void fromString(QString s, bool &field)
        {
            s = s.toUpper();

            if ((s == "1") || (s == "TRUE") || (s == "T"))
            {
                field = true;
                return;
            }

            if ((s == "0") || (s == "FALSE") || (s == "F"))
            {
                field = false;
                return;
            }

            MetaConfig::ThrowBadConversionFromString(s);
        }

        static QString toString(const bool &field)
        {            
            return field ? "true" : "false";
        }

        static QString getTypeName()
        {
            return "boolean";
        }

    };

    template<>
    class FieldConv<std::string>
    {
    public:
        static void fromString(QString s, std::string &field)
        {
            field = s.toStdString () ;
        }

        static QString toString(const std::string &field)
        {            
            return QString::fromStdString(field);
        }

        static QString getTypeName()
        {
            return "std::string";
        }
    };

}  // namespace Utils


#endif
