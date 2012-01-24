#include "stdafx.h"
#include "Utils/BinaryWriter.h"
#include "Utils/BinaryReader.h"
#include "ConfigLineList.h"
#include "CfgRestoreError.h"

namespace BfBootSrv
{      

    bool ConfigLineList::AddLine( const BfBootCore::ConfigLine& line )
    {
        // поиск дубликатов
        for (Lines::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
        {   
            if ( i->Name() == line.Name() ) return false;
        }

        m_lines.push_back(line);
        return true;
    }

    // ------------------------------------------------------------------------------------

    const BfBootCore::ConfigLine& ConfigLineList::FindLineByName( const std::string& name ) const /* can throw ErrCfgNoParam */
    {
        // todo use memFind            
        for (Lines::const_iterator i = m_lines.begin(); i != m_lines.end(); ++i)
        {   
            if ( i->Name() == name ) return *i;
        }

        ESS_THROW(ErrCfgNoParam);
        return m_lines.front(); // suppress warning
    }

    // ------------------------------------------------------------------------------------

    ConfigLineList::ConfigLineList( Utils::IBinaryReader& reader )
    {
        Platform::word lineCount = reader.ReadWord();
        BfBootCore::CRC crc;
        std::string strLine;
        for (int i = 0; i < lineCount; ++i)
        {                

            // todo когда сериализация будет перенесена в Core использовать
            // единую иерархию исключений для всех ошибок восстановления.
            try
            {
                Platform::word size = reader.ReadWord();
                if (size == 0) ESS_THROW(ErrCfgParse);
                reader.ReadString(strLine, size);
                m_lines.push_back( BfBootCore::ConfigLine(strLine) );
            }
            catch(BfBootCore::ConfigLine::ParseError&)
            {
                ESS_THROW(ErrCfgParse);
            }
            catch(Utils::NotEnoughMemory&) // ?
            {
                ESS_THROW(ErrCfgParse);
            }

            crc.ProcessBlock(&strLine.at(0), strLine.size());
        }
        if (crc.Release() != reader.ReadDword()) ESS_THROW(ErrCfgCRC);
    }

    // ------------------------------------------------------------------------------------

    void ConfigLineList::Save( Utils::IBinaryWriter& writer ) const
    {       
        writer.WriteWord(m_lines.size()); // line count

        BfBootCore::CRC crc;
        std::string str;
        for (Lines::const_iterator i = m_lines.begin(); i != m_lines.end(); ++i)
        {
            str = i->getRawString();
            ESS_ASSERT(!str.empty());

            crc.ProcessBlock(&str.at(0), str.size());
            writer.WriteWord(str.size());
            writer.WriteString(str);
        }
        writer.WriteDword(crc.Release());
    }
   
} // namespace BfBootSrv

