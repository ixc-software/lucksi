
#include "stdafx.h"

#include "ProgramOptions.h"


namespace 
{
    const int CScreenWidthCh = 80; // chars

    // --------------------------------------------------------------
    
    std::vector<std::string> HelpOptList()
    {
        std::vector<std::string> res;
        res.push_back("h");
        res.push_back("help");
        return res;
    }

    // --------------------------------------------------------------

    void ScreenWidthLines(std::string text, int offset, std::stringstream &output)
    {
        std::string textOffsetStr(offset, ' ');
        int i = 0;
        while (i < text.size())
        {
            std::string textPart(text.substr(i, CScreenWidthCh - textOffsetStr.size()));

            output << textOffsetStr << textPart << std::endl;

            i += textPart.size();
        }
    }

    
} // namespace 

// --------------------------------------------------------------

namespace Utils
{
    // --------------------------------------------------------------
    // OptionBase impl

    int OptionBase::Find(const std::string &name) const
    {
        for (size_t i = 0; i < m_names.size(); ++i)
        {
            if (m_names.at(i) == name) return i;
        }

        return -1;
    }

    // --------------------------------------------------------------

    void OptionBase::CheckDublicates(const std::vector<std::string> &names) const // can throw
    {
        std::string sameList;
        for (int i = 0; i < names.size(); ++i)
        {
            if (Find(names.at(i)) >= 0)
            {
                sameList += names.at(i);
                if (i < (names.size() - 1)) sameList += ", ";
            }
        }

        if (sameList.empty()) return;

        PE::DublicateNameError(sameList);
    }

    // --------------------------------------------------------------

    void OptionBase::CheckDublicates(const OptionBase &other) const // can throw
    {
        std::string sameList;
        for (int i = 0; i < m_names.size(); ++i)
        {
            if (other.Find(m_names.at(i)) >= 0)
            {
                sameList += m_names.at(i);
                if (i < (m_names.size() - 1)) sameList += ", ";
            }
        }

        if (sameList.empty()) return;

        PE::DublicateNameError(sameList);
    }

    // --------------------------------------------------------------
    
    std::string OptionBase::Names(const std::string &key) const
    {
        std::string res;
        for (size_t i = 0; i < m_names.size(); ++i)
        {
            res += key;
            res += m_names.at(i);
            if (i != (m_names.size() - 1)) res += ", ";
        }

        return res;
    }
    
    // --------------------------------------------------------------
    // ProgramOptionsExceptions impl
    
    void ProgramOptionsExceptions::DublicateNameError(const std::string &src)
    {
        std::string msg("Dublicate name(s)"); 
        if (!src.empty())
        {
            msg += ": ";
            msg += src;
        }
        
        ESS_THROW_MSG(DublicateName, msg);
    }

    // --------------------------------------------------------------
    
    void ProgramOptionsExceptions::Error(const std::string &desc, const std::string &src)
    {
        std::string msg = desc; 
        if (!src.empty())
        {
            msg += ": ";
            msg += src;
        }
        
        ESS_THROW_MSG(ProcessError, msg);
    }

    // --------------------------------------------------------------
    // OptionExecHelp impl
    
    void OptionExecHelp::Exec(const std::vector<std::string> &data) // override
    {
        typedef ProgramOptionsExceptions PE;
        
        if (!data.empty()) PE::Error("Value for option without value");

        // ESS_THROW_MSG(PE::HelpRequest, m_src.HelpInfo());

        m_helpRequested = true;
    }

    // --------------------------------------------------------------

    OptionExecHelp::OptionExecHelp(bool &helpRequested) :
        OptionBase(HelpOptList(), "list of program's command line options"),  
        m_helpRequested(helpRequested)
    {
    }

    // --------------------------------------------------------------
    // OptionList impl
    
    OptionList::OptionList(const std::string &key) :
        m_opt(true), m_key(key)
    {
    }

    // --------------------------------------------------------------

    OptionBase* OptionList::Find(const std::string &name) const
    {
        
        ESS_ASSERT(!name.empty());

        for (size_t i = 0; i < m_opt.Size(); ++i)
        {
            if (m_opt[i]->Find(name) >= 0) return m_opt[i];
        }

        return 0;
    }

    // --------------------------------------------------------------

    void OptionList::AllNames(std::vector<std::string> &res, size_t &longestSize) const
    {
        res.clear();
        longestSize = 0;
        for (int i = 0; i < m_opt.Size(); ++i)
        {
            res.push_back(m_opt[i]->Names(m_key));
            if (res.at(i).size() > longestSize) longestSize = res.at(i).size();
        }
    }

    // --------------------------------------------------------------

    void OptionList::AllDescriptions(std::vector<std::string> &res) const
    {
        res.clear();
        for (int i = 0; i < m_opt.Size(); ++i)
        {
            res.push_back(m_opt[i]->Description());
        }
    }

    // --------------------------------------------------------------

    /*std::string OptionList::OptionHelpInfo(const std::string &names,
                                           const std::string &desc,
                                           int descMinOffset)
    {
        ESS_ASSERT(names.size() <= descMinOffset);

        std::string sep = " -- ";
        int descOffset = descMinOffset + sep.size();

        std::stringstream res;
        res << names << std::string(descMinOffset - names.size(), ' ') << sep;
        
        int i = 0;
        while (i < desc.size())
        {
            std::string descPart(desc.substr(i, CScreenWidthCh - descOffset));

            if (i > 0) res << std::string(descOffset, ' ');
            res << descPart << std::endl;

            i += descPart.size();
        }
        
        return res.str();
    }*/

    std::string OptionList::OptionHelpInfo(const std::string &names,
                                           const std::string &desc,
                                           int descMinOffset)
    {
        ESS_ASSERT(names.size() <= descMinOffset);

        std::string sep = " -- ";
        int descOffset = descMinOffset + sep.size();

        // names
        std::stringstream res;
        res << names << std::string(descMinOffset - names.size(), ' ') << sep;

        // description
        std::vector<std::string> descList;

        // split to lines
        {
            int i = 0;
            while (i < desc.size())
            {
                int lfPos = desc.find_first_of('\n', i);

                descList.push_back(desc.substr(i, lfPos - 1));

                if (lfPos < 0) break;

                i += lfPos + 1;
            }
        }

        // add desc to res
        for (size_t i = 0; i < descList.size(); ++i)
        {
            int offset = (i == 0)? 0 : descOffset;
            ScreenWidthLines(descList.at(i), offset, res);
        }

        return res.str();
    }

    // --------------------------------------------------------------
    
    std::string OptionList::HelpInfo() const
    {
        std::string res("Command line options:\n");

        std::vector<std::string> names;
        size_t longestNameSise = 0;
        AllNames(names, longestNameSise);

        std::vector<std::string> descs;
        AllDescriptions(descs);

        ESS_ASSERT(names.size() == descs.size());
        
        for (size_t i = 0; i < names.size(); ++i)
        {
            std::string desc = descs.at(i);
            desc += (i == names.size() - 1)? "." : ";";
            
            res += OptionHelpInfo(names.at(i), desc, longestNameSise);
        }

        return res;
    }

    // --------------------------------------------------------------
    
    std::string OptionList::HelpKeys() const
    {
        bool dummi;
        OptionExecHelp opt(dummi);

        return opt.Names(m_key);
    }

    // --------------------------------------------------------------

        
} // namespace Utils
