#include "stdafx.h"
#include "ScriptCmdList.h"

#include "Utils/StringUtils.h"
#include "Utils/StringParser.h"


namespace 
{    
    const char* CCmdSeparator = ";";
    const char CLeftPar = '(';
    const char CRightPar = ')';
    const char* CArgSep = ",";

    const std::string CIntfMain = "MAIN";
    const std::string CIntfExt = "SPI";        

    
} // namespace
 
namespace BfBootCore
{
    class ScriptCmdList::CmdLoad : public ICmd
    {        
        std::string m_intf;            
        ImgId m_id;            

        static std::string& Name() 
        {
            static std::string name("Load");
            return name;
        }

    // ICmd impl
    public:
        std::string getIntf() const
        {
            return m_intf;
        }
        ImgId getImgId() const
        {
            return m_id;
        }

    public:
        CmdLoad(std::string& inStr, bool last)
        {
            if(inStr.empty()) ESS_THROW_MSG(ScriptError, "CmdEmpty: " + inStr);
            inStr = Utils::TrimBlanks(inStr);      

            // Find parenthesis
            size_t beginParam = inStr.find(CLeftPar);
            size_t endParam = inStr.find(CRightPar);
            if (beginParam == std::string::npos ||
                endParam == std::string::npos ) ESS_THROW_MSG(ScriptError, "NoParenthesis: " + inStr);     
            if (endParam != inStr.size() - 1) ESS_THROW_MSG(ScriptError, "UnexpectedParantesis: " + inStr);   

            // Extract cmd name:
            std::string cmdName;                            
            cmdName = inStr.substr(0, beginParam);
            cmdName = Utils::TrimBlanks(cmdName);
            if(cmdName != Name()) ESS_THROW_MSG(ScriptError, "UnknownCmdName: " + cmdName); 

            // Extract params:
            std::string args = inStr.substr(beginParam + 1, endParam - beginParam - 1);

            // Parse params:
            Utils::ManagedList<std::string> argList;        
            Utils::StringParser(args, CArgSep, argList);
            if (2 != argList.Size()) ESS_THROW_MSG(ScriptError, "WrongArgCount in " + inStr);

            // Read args:
            m_intf = Utils::TrimBlanks(*argList[0]);
            if (m_intf != CIntfMain && m_intf != CIntfExt) ESS_THROW_MSG(ScriptError, "UnknownIntfName: " + m_intf); 
            
            std::istringstream ss; 
            ss.str(Utils::TrimBlanks(*argList[1]));
            ss >> m_id;            
            if (ss.fail()) ESS_THROW_MSG(ScriptError, "Can`t convert to ImgId: " + *argList[1]);

            // last command check
            if (!last)
            {
                if (m_intf == CIntfMain) ESS_THROW_MSG(ScriptError, "Main intf not last.");
            }
        }           
    };

    // ------------------------------------------------------------------------------------

    ScriptCmdList::ScriptCmdList(const std::string& script)
    {
        std::string in = Utils::TrimBlanks(script);

        Utils::ManagedList<std::string> strCmdList;        
        Utils::StringParser(in, CCmdSeparator, strCmdList);
        //if (!strCmdList.Back()->empty()) ESS_THROW(NoCmdSeparator);         
        
        if (strCmdList.Back()->empty()) strCmdList.Delete(strCmdList.Size() - 1);        
        
        for (int i = 0; i < strCmdList.Size(); ++i)
        {
            bool last = (i == strCmdList.Size() - 1);
            m_list.Add( new CmdLoad(*strCmdList[i], last) );                        
        }                        
    }

    // ------------------------------------------------------------------------------------

    const ICmd& ScriptCmdList::operator[]( int index )
    {
        ESS_ASSERT(index < m_list.Size());
        return *m_list[index];
    }

    // ------------------------------------------------------------------------------------

    int ScriptCmdList::Size() const
    {
        return m_list.Size();
    }

    // ------------------------------------------------------------------------------------

    bool ScriptCmdList::Validate( const std::string& scriptString, int imgCount, std::string* pErr) // static
    {        
        try
        {
            ScriptCmdList parsed(scriptString);

            for (int i = 0; i < parsed.Size(); ++i)
            {
                if (parsed[i].getImgId() >= imgCount) 
                {
                    if (pErr) *pErr = "Wrong ImgId";                    
                    return false;
                }
            }            
        }
        catch(ScriptError &e)
        {
            if (pErr) *pErr = e.getTextMessage();
            return false;
        }

        if (pErr && !pErr->empty()) pErr->clear();
        return true;
    }

    // ------------------------------------------------------------------------------------

    const std::string& ScriptCmdList::MainIntf() // static
    {
        return CIntfMain;
    }

    // ------------------------------------------------------------------------------------

    const std::string& ScriptCmdList::ExtIntf() // static
    {
        return CIntfExt;
    }

    
} // namespace BfBootCore

