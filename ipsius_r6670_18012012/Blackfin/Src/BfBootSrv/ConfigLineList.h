#ifndef CONFIGLINELIST_H
#define CONFIGLINELIST_H

#include "stdafx.h"
#include "BfBootCore/ConfigLine.h"
#include "Utils/IBinaryReader.h"
#include "Utils/IBinaryWriter.h"

// cpp
#include "BfBootCore/CRC.h"
#include "CfgRestoreError.h"

namespace BfBootSrv
{    
    
    using Platform::dword;    
    using Platform::byte;    

    //class FlashOStream;
    //class FlashIStream;

    class ConfigLineList : boost::noncopyable
    {
        typedef std::list<BfBootCore::ConfigLine> Lines;
        Lines m_lines;                
        
        //void WriteUsefulData(std::vector<byte>& buff) const;

    public:        

        bool AddLine(const BfBootCore::ConfigLine& line);
        const BfBootCore::ConfigLine& FindLineByName(const std::string& name) const; // can throw ErrCfgNoParam;
        
        ConfigLineList(){}                               

        ConfigLineList(Utils::IBinaryReader& reader);

        void Save(Utils::IBinaryWriter& writer) const;
    };

} // namespace BfBootSrv


#endif
