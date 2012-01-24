#include "stdafx.h"

#include "Platform/Platform.h"

#include "DriArgFactory.h"
#include "commanddriresult.h"

// ------------------------------------------------

using namespace DRI;

namespace
{

    template<class T>
    class ArgBase : public IArgFactoryType, public IQGenericArgument
    {
        T m_val;
        QGenericArgument m_arg;

    // IArgFactoryType partial impl
    private:

        const char* TypeName() const 
        { 
            static std::string name( Platform::FormatTypeidName( typeid(T).name() ) );
            return name.c_str(); 
        }

    // IQGenericArgument impl
    private:

        QGenericArgument& GetGenericArgument()
        {
            return m_arg;
        }

    public:

        ArgBase() : m_val() {}

        ArgBase(const T &val) :
            m_val(val),
            m_arg(TypeName(), static_cast<const void *>(&m_val))
        {
        }

    };

    // --------------------------------------------------------

    class ArgQString : public ArgBase<QString>
    {

    // IArgFactoryType impl
    private:

        IQGenericArgument* FromString(const QString &val) const
        {
            return new ArgQString(val);
        }

        IQGenericArgument* DefaultValue() const
        {
            return FromString("");
        }

    public:

        ArgQString() {}
        ArgQString(const QString &val) : ArgBase<QString>(val) {}

    };

    // --------------------------------------------------------

    class ArgBool : public ArgBase<bool>
    {

    // IArgFactoryType impl
    private:

        IQGenericArgument* FromString(const QString &val) const
        {
            std::string error;
            bool setValue = DriArgFactory::StringToBool(val, error);

            if (!error.empty())
            {
                ESS_THROW_MSG(DRI::DriArgException, error);
            }

            return new ArgBool(setValue);
        }

        IQGenericArgument* DefaultValue() const
        {
            return new ArgBool(false);
        }

    public:

        ArgBool() {}
        ArgBool(bool val) : ArgBase<bool>(val) {}

    };

    // --------------------------------------------------------

    class ArgInt : public ArgBase<int>
    {

    // IArgFactoryType impl
    private:

        IQGenericArgument* FromString(const QString &val) const
        {
            bool ok = false;
            int i = val.toInt(&ok);

            if (!ok) 
            {
                std::ostringstream ss;
                ss << "Bad int: " 
                    << val.toStdString();
                ESS_THROW_MSG(DRI::DriArgException, ss.str());
            }

            return new ArgInt(i);
        }

        IQGenericArgument* DefaultValue() const
        {
            return new ArgInt(0);
        }

    public:

        ArgInt() {}
        ArgInt(int val) : ArgBase<int>(val) {}

    };

}  // namespace

// ------------------------------------------------

namespace DRI
{
    
    DriArgFactory::DriArgFactory()
    {
        Add( new ArgQString() );
        Add( new ArgBool() );
        Add( new ArgInt() );
    }

    bool DriArgFactory::StringToBool( const QString &val, std::string &error )
    {
        error.clear();

        QString v = val.toUpper();

        bool setValue = false;

        if (v == "TRUE" || v == "T") setValue = true;
        else if (v == "FALSE" || v == "F") setValue = false;
        else 
        {
            std::ostringstream ss;
            ss << "Bad bool: " 
                << val.toStdString();
            error = ss.str();
            return false;  // dummi
        }

        return setValue;
    }

}  // namespace DRI

