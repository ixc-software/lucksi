
#ifndef __PROGRAMOPTIONSSETTERS__
#define __PROGRAMOPTIONSSETTERS__

// ProgramOptionsSetters.h

#include "stdafx.h"
#include "Utils/ProgramOptions.h"
#include "Utils/IntToString.h"
#include "Utils/HostInf.h"
#include "Utils/StringUtils.h"

namespace Utils
{
    /*
        Set given field 
    */

    // Base class for all setters
    class SetOptBase: public OptionBase
    {
    public:
        enum ParamCount
        {
            PCNone = 0,
            PCOne = 1,
            PCOneOrNone = 2, // for bool
        };
        
        SetOptBase(const std::vector<std::string> &names,
                   const std::string &desc,
                   ParamCount paramCount = PCOne) : 
            OptionBase(names, desc), m_paramCount(paramCount)
        {
        }

        virtual ~SetOptBase()
        {
        }

    protected:
        virtual void SetOption(const std::string &val) = 0;
        
        void OptValueErr(const std::string &optType, const std::string &val)
        {
            std::ostringstream err;
            err << "Invalid '" << Names("") << "' " 
                << "option (" << optType  << "-type) value: '" << val << "'";
            
            ProgramOptionsExceptions::Error(err.str());
        }

    private:
        ParamCount m_paramCount;

        void Exec(const std::vector<std::string> &data) // override
        {
            if ((data.size() == 0) 
                && ((m_paramCount == PCNone) || (m_paramCount == PCOneOrNone)))
            {
                SetOption("");
                return; 
            }

            if ((data.size() == 1) 
                && ((m_paramCount == PCOne) || (m_paramCount == PCOneOrNone)))
            {
                SetOption(data.at(0));
                return; 
            }

            std::ostringstream err;
            err << "Invalid '" << Names("") << "' option paramaters count: " << data.size();
            
            ProgramOptionsExceptions::Error(err.str());
        }
    };

    // --------------------------------------------

    class SetOptString : public SetOptBase
    {
        std::string &m_field;

        void SetOption(const std::string &val) // override
        {
            m_field = val;
        }

    public:
        SetOptString(std::string &field,
                     const std::vector<std::string> &names, 
                     const std::string &desc) :
        SetOptBase(names, desc),  m_field(field)
        {
        }
    };
    
    // --------------------------------------------

    class SetOptQString : public SetOptBase
    {
        QString &m_field;

        void SetOption(const std::string &val) // override
        {
            m_field = QString::fromStdString(val);
        }
        
    public:
        SetOptQString(QString &field,
                      const std::vector<std::string> &names,
                      const std::string &desc) :
            SetOptBase(names, desc),  m_field(field)
        {
        }
    };

    // --------------------------------------------

    class SetOptInt : public SetOptBase
    {
        int &m_field;

        void SetOption(const std::string &val) // override
        {
            if (StringToInt(val, m_field)) return;

            OptValueErr("int", val);
        }
        
    public:
        SetOptInt(int &field,
                  const std::vector<std::string> &names,
                  const std::string &desc) :
            SetOptBase(names, desc),  m_field(field)
        {
        }
    };

    // --------------------------------------------

    class SetOptBool : public SetOptBase
    {
        bool &m_field;

        void SetOption(const std::string &val) // override
        {
            if (val.empty())
            {
                m_field = true;
                return;
            }

            // if ((val != "0") && (val != "1")) OptValueErr("bool", val);
            // m_field = (val == "0")? false : true;
             
            if ((val == "0") 
                || (Utils::StringUpCase(val) == "FALSE")
                || (Utils::StringUpCase(val) == "F"))
            {
                m_field = false;
                return;
            }

            if ((val == "1") 
                || (Utils::StringUpCase(val) == "TRUE")
                || (Utils::StringUpCase(val) == "T"))
            {
                m_field = true;
                return;
            }

            OptValueErr("bool", val);
        }
        
    public:
        SetOptBool(bool &field,
                   const std::vector<std::string> &names,
                   const std::string &desc) :
            SetOptBase(names, desc, SetOptBase::PCOneOrNone),  m_field(field)
        {
        }
    };
    
    // --------------------------------------------

    class SetOptHostInf : public SetOptBase
    {
        HostInf &m_field;

        void SetOption(const std::string &val) // override
        {
            if (HostInf::FromString(val, m_field, HostInf::HostAndPort)) return;
            if (HostInf::FromString(val, m_field, HostInf::PortOnly))
            {
                // m_field.Address("127.0.0.1");
                return;
            }

            OptValueErr("HostInf", val);
        }
        
    public:
        SetOptHostInf(Utils::HostInf &field,
                      const std::vector<std::string> &names,
                      const std::string &desc) :
            SetOptBase(names, desc),  m_field(field)
        {
        }
    };

    // ---

    class SetOptStringList : public OptionBase
    {
        QStringList &m_field;
        
    private:
        void Exec(const std::vector<std::string> &data) // override
        {
            if (data.empty())
            {
                std::ostringstream err;
                err << "Invalid '" << Names("") << "' option paramaters count: 0";
            
                ProgramOptionsExceptions::Error(err.str());
            }
            
            for (size_t i = 0; i < data.size(); ++i)
            {
                m_field.push_back(data.at(i).c_str());
            }
        }

    public:
        SetOptStringList(QStringList &field,
                         const std::vector<std::string> &names,
                         const std::string &desc) : 
            OptionBase(names, desc), m_field(field)
        {
            field.clear();
        }
    };
    
} // namespace Utils

#endif
