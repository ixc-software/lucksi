#ifndef CONFIGLINE_H
#define CONFIGLINE_H

#include "stdafx.h"
#include "Platform/PlatformTypes.h"
#include "Utils/ErrorsSubsystem.h"

#include "Utils/MemReaderStream.h"
#include "Utils/MemWriterStream.h"


namespace BfBootCore
{
    class FlashIStream;
    class FlashOStream;
    using Platform::dword;    
    using Platform::byte;  

    // структурированное представление строки конфигурационого файла
    class ConfigLine
    {                
        std::string m_arg;
        std::string m_val;
        //std::string m_raw;        

    public:        

        ESS_TYPEDEF(ParseError);

        //bool ReadFromStream(Utils::MemReaderStream& stream);                     // конструктор ?

        // return read byte count        
        //int Read(const void* pData, int size);
        //int Read(const std::vector<byte> rawData, int offs);
        
                
        std::string getRawString() const;    

        int getRawSize() const;                

        // Конструируется с сырой строки. Синтаксис строки strName = strValue
        ConfigLine(const std::string& str); 


        // конструируется из исходной пары парметр, значение (исходящий)
        ConfigLine(const std::string& paramName, const std::string& value);

        ConfigLine(){}
                
        const std::string& Name() const 
        {            
            return m_arg;
        }
        const std::string& Value() const 
        {            
            return m_val;
        }

        bool IsEqual(const ConfigLine& other) const
        {
            return other.m_arg == m_arg && other.m_val == m_val;
        }
       
    };  

} // namespace BfBootCore

#endif
