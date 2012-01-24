#include "stdafx.h"
#include "BroadcastMsg.h"
//#include "SbpSerialTranslator.h"
#include "Utils/IniFile.h"
#include "BfBootCore/StringToTypeConverter.h"

#include "Utils/Random.h"

namespace 
{
	using Platform::dword;
    using Platform::word;
    using Platform::byte;    
    
        	
    const Platform::dword CProtoVersion = 1;        
        
    const dword CVer = 2;
    const dword CMaxVer = 20;

    const char CSeparator = '=';
    const char CTerminator = '\n';
    const char CStringChar = '\"';
    
    std::string ValToStr(const int val)
    {
        return Utils::IntToString(val);
    }
    const std::string& ValToStr(const std::string& val)
    {
        return val;
    }    

    void WriteLine(const std::string& name, const std::string& val, std::vector<byte>& out)
    {        
        // create line
        ESS_ASSERT(name.find(CSeparator) == std::string::npos);
        ESS_ASSERT(val.find(CSeparator) == std::string::npos);
        const std::string line = name + CSeparator + CStringChar + val + CStringChar + CTerminator;

        // add line to binary
        ESS_ASSERT(!line.empty());
        /*
        int oldSize = out.size();
        out.resize(oldSize + line.size());
        std::memcpy(&out.at(oldSize), &line.at(0), line.size());
        */
        
        //std::copy(line.begin(), line.end(), std::inserter(out, out.end())); // нет std::inserter под Bf
        for (int i = 0; i < line.size(); ++i)        
        {
        	out.push_back(line[i]);
        }
    
        
    }  

    template<class T>
    void WriteLine(const std::string& name, const T& val, std::vector<byte>& out)
    {
        WriteLine(name, ValToStr(val), out);
    }     

    bool ItemToVal( const Utils::IniFileItem& src, std::string& val)
    {
        val = src.AsString();
        return true;
    }

    bool ItemToVal( const Utils::IniFileItem& src, int& val)
    {
        bool err;
        val = src.AsInt(&err);
        return !err;
    }

    
    bool ItemToVal(const Utils::IniFileItem& src, dword& val)
    {
        bool err;
        int i = src.AsInt(&err);
        val = i;
        return !err && i >= 0;
    }
    
    
    template<class T>
    bool ReadToVal(const Utils::IniFileSection& src, const std::string& name, T& out)
    {        
        const Utils::IniFileItem* p = src.FindItem(name);
        if (p == 0) return false;
        return ItemToVal(*p, out);
    }

    class ParserImpl : public Utils::IniFile
    {
    public:
        ParserImpl(Utils::IIniFileDataSource &src, const Utils::IniFileConfig &cfg) : Utils::IniFile(src, cfg)
        {}
    };


    class Parser : public Utils::IIniFileDataSource
    {   
        // todo create line here
        
        const Platform::byte* m_data;
        const int m_size;

        int m_index;

        boost::scoped_ptr<ParserImpl> m_parser;

        int Find(int from, byte what)
        {
            for (int i = from; i < m_size; ++i)
            {
                if (m_data[i] == what) return i;
            }
            return -1;
        }

    private:
        bool Eof()
        {
            return m_index >= m_size;
        }
        std::string ReadNextLine()
        {
            ESS_ASSERT(!Eof());
            int term = Find(m_index, CTerminator);                 
            ESS_ASSERT(term < m_size);
            
            if (term == -1)
            {
                m_index = m_size;
                return "";
            }

            std::string str;            
            str.resize(term - m_index);
            if (term != m_index) std::memcpy(&str.at(0), m_data + m_index, term - m_index);
            m_index = term + 1;
            return str;
        }

    public:
        Parser(const Platform::byte* data, int size) : m_data(data), m_size(size), m_index(0)
        {
            Utils::IniFileConfig cfg;
            cfg.StringChar = CStringChar;
            m_parser.reset( new ParserImpl(*this, cfg) );
        }

        ParserImpl& getImpl()
        {
            return *m_parser;
        }

    };
    

} // namespace 

namespace BfBootCore
{          

    BroadcastMsg::BroadcastMsg() 
        : BroadcastVersion(CVer),
        BooterVersionNum(0),
        HwNumber(0),
        HwType(0),
        BooterPort(0), 
        CmpPort(0),
        SoftRelNumber(NoRelNum),
        ExtraFieldExist(false)
    {
    }    

    // ------------------------------------------------------------------------------------

    bool BroadcastMsg::Assign( const Utils::HostInf& src, const Platform::byte* data, int size)
    {          
        if (size == 0) return false;
        boost::scoped_ptr<Parser> parser;
        try
        {
            parser.reset( new Parser(data, size) );
        }
        catch(Utils::IniFileParseError& err)        
        {            
            return false;
        }        

        if ( parser->getImpl().SectionsCount() == 0 ) return false;       
        const Utils::IniFileSection& frame = (parser->getImpl())[0];
        if ( frame.ItemsCount() <  CItemCount ) return false;       
        
        // Read        
        if (!ReadToVal(frame, "BroadcastVersion", BroadcastVersion)) return false;        
        if (BroadcastVersion > CMaxVer) return false;
        if (!ReadToVal(frame, "BooterVersionInfo", BooterVersionInfo)) return false;          

        if (!ReadToVal(frame, "BooterVersionNum", BooterVersionNum)) return false;        
        if (!ReadToVal(frame, "HwNumber", HwNumber)) return false;  
        if (!ReadToVal(frame, "HwType", HwType)) return false;        
        if (!ReadToVal(frame, "BooterPort", BooterPort)) return false;          
        if (!ReadToVal(frame, "CmpPort", CmpPort)) return false;          
        if (!ReadToVal(frame, "OwnerAddress", OwnerAddress)) return false;  
        if (!ReadToVal(frame, "Mac", Mac)) return false;  
        if (!ReadToVal(frame, "SoftRelNumber", SoftRelNumber)) return false;  

        std::string descrList;
        if (!ReadToVal(frame, "ImgDescrList", descrList) ) return false;

        try
        {
            StringToTypeConverter::fromString(descrList, SoftReleaseInfo);
        }
        catch(StringToTypeConverter::CantConvert&)
        {
            return false;
        }
       
        CbpAddress.Set(src.Address(), BooterPort < 0 ? 0 : BooterPort);
        SrcAddress = src;

        if (!Consistency()) return false;        

        ExtraFieldExist = (CItemCount < frame.ItemsCount());
        return true;
    }

    // ------------------------------------------------------------------------------------

    void BroadcastMsg::ToBin( std::vector<Platform::byte>& out )const /* ”добно ли vector<byte> -- ? */
    {        
        ESS_ASSERT(Consistency());                
        out.clear();

        WriteLine("BroadcastVersion", CVer, out);        
        WriteLine("BooterVersionInfo", BooterVersionInfo, out);
        WriteLine("BooterVersionNum", BooterVersionNum, out);
        WriteLine("HwNumber", HwNumber, out);
        WriteLine("HwType", HwType, out);
        WriteLine("BooterPort", BooterPort, out);
        WriteLine("CmpPort", CmpPort, out);
        WriteLine("OwnerAddress", OwnerAddress, out);
        WriteLine("Mac", Mac, out);
        WriteLine("SoftRelNumber", SoftRelNumber, out);

        WriteLine("ImgDescrList", StringToTypeConverter::toString( SoftReleaseInfo ), out);
    }   

    // ------------------------------------------------------------------------------------

    std::string BroadcastMsg::Info() const
    {        

        std::ostringstream ss;
        ss  << "BroadcastRevision = " << BroadcastVersion
            << "\nBooterRevision = " << BooterVersionNum
            << "\nhwNumber = " << HwNumber
            << "\nhwType = " << HwType
            << "\ndevPort = " << BooterPort            
            << "\nOwnerInfo: " << (OwnerAddress.empty() ? "DeviceFree" : OwnerAddress)
            << "\nExtraFieldExist = " << ExtraFieldExist
            << "\nSwReleaseNum = ";

        if (IsExistSoftRelNum()) ss << SoftRelNumber;
        else ss << "NotExist";

        ss << "\nSorce is " << (IsBooter() ? "booter" : "application");


        if (!CbpAddress.Empty()) ss << "\nSorce address: " << SrcAddress.ToString();
        return ss.str();

    }

    // -------------------------------------------------------------------------------------

    bool BroadcastMsg::Consistency() const
    {
        return 
            (CmpPort <= 0) == (BooterPort > 0)//  "Only one port must be assigned"
         && (SoftReleaseInfo.Size() == 0) == (SoftRelNumber == NoRelNum);         
    }        

    
} // namespace BfBootCore

