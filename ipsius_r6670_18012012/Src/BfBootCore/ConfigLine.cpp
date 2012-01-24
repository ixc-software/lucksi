#include "stdafx.h"
#include "ConfigLine.h"
#include "UserResponseCode.h"



namespace 
{    
    const char CSeparator = '=';
    const char CTerminator = '\n';        
} // namespace 

namespace BfBootCore
{           

    ConfigLine::ConfigLine( const std::string& paramName, const std::string& value )
    {
        // assert vs throw?
        //ESS_ASSERT(paramName.find(CSeparator) == std::string::npos);        
        if (paramName.find(CSeparator) != std::string::npos) ESS_THROW_T(BoardException, errWrongParamName);

        m_arg = paramName;
        m_val = value;

        //m_raw.append(m_arg).push_back(CSeparator).push_back(m_val).push_back(CTerminator);
    }

    // ------------------------------------------------------------------------------------

    ConfigLine::ConfigLine( const std::string& str ) 
    {
        // todo trimm whitespace

        if ( str.empty() ) ESS_THROW(ParseError);
        if (str.at(str.size() - 1) != CTerminator) ESS_THROW(ParseError);

        size_t sep = str.find(CSeparator);

        if (sep == std::string::npos) ESS_THROW(ParseError);         
        
        m_arg = str.substr(0, sep); 
        m_val = str.substr(sep + 1, str.size() - sep - 2);
    }

    // ------------------------------------------------------------------------------------

    std::string ConfigLine::getRawString() const   // кэшировать строку
    {        
        std::ostringstream ss;
        ss << m_arg << CSeparator << m_val << CTerminator;
        return ss.str();
    }

    // ------------------------------------------------------------------------------------

    int ConfigLine::getRawSize() const
    {
        return m_arg.size() + m_val.size() + 2;
    }    
 
    
} // namespace BfBootSrv
