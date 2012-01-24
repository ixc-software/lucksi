#include "stdafx.h"
#include "IsdnUtils.h"
#include "Utils/QtHelpers.h"
#include "Utils/StringUtils.h"

namespace ISDN
{
    namespace IsdnUtils
    {

        QString ShortName(const QString& isxStr)
        {            
            int i = isxStr.lastIndexOf("::", isxStr.lastIndexOf('<'));
            if (i == -1) return isxStr; // "::" not found. Nothing to do.            
            return isxStr.mid(i + 2);
        }

        std::string ShortName(std::string& isxStr)
        {
            int i = isxStr.find_last_of("::");
            return isxStr.substr(i + 1);
        }

        LogString ToLogString(const QString &s)
        {
            return Utils::QStringToWString(s);
        }

        LogString ToLogString(const std::string &s)
        {
            return Utils::StringToWString(s);
        }

        void CorrectSet::Add( int val )
        {
            ESS_ASSERT( m_setValue.insert(val).second );
        }

        void CorrectSet::Reset()
        {
            m_setValue.clear();
        }

    	
    }  // namespace IsdnUtils
    
    
}  // namespace ISDN
