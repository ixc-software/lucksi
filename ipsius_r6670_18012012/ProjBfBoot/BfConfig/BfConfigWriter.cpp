#include "stdafx.h"
#include "BfUtils/StringConvertions.h"
#include "BfConfigWriter.h"


namespace BfConfig
{

    BfConfigWriter::BfConfigWriter(IBfConfig &textConfig) : m_textConfig(textConfig)
    {

    }


    //-------------------------------------------------------------------------


    bool BfConfigWriter::SetString(const String &key, const String &value, bool allowCreation)
    {
        return m_textConfig.SetString(key, value, allowCreation);
    }


    //-------------------------------------------------------------------------


    bool BfConfigWriter::SetInteger(const String &key, const int &value, bool allowCreation)
    {
        return m_textConfig.SetString(key, BfUtils::to_string(value), allowCreation);
    }


    //-------------------------------------------------------------------------


    bool BfConfigWriter::SetDescription(const String &key, const String &description)
    {
        return m_textConfig.SetDescription(key, description);
    }

} // namespace BfConfig
