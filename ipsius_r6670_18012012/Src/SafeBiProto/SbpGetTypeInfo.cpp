
#include "stdafx.h"
#include "SbpGetTypeInfo.h"

#include "Utils/MemReaderStream.h"
#include "Utils/BinaryReader.h"
#include "Utils/IntToString.h"


namespace
{
    using namespace SBProto;

    template<class T>
    void TypeToString(const void *pData, size_t size, std::string &outStr)
    {
        ESS_ASSERT(size == sizeof(T));
        
        // std::cout << (Platform::dword)pData << std::endl;  // debug

        // copy data for memory align - !
        T val;
        std::memcpy(&val, pData, size);
        
        std::ostringstream os;
        os << val;

        outStr = os.str();
    }

    // ---------------------------------------------------------------------------

    void BoolToString(const void *pData, size_t size, std::string &outStr)
    {
        ESS_ASSERT(size == sizeof(bool));

        bool val = *(( const bool*)pData);
        outStr = val ? "true" : "false";
    }


    // ---------------------------------------------------------------------------

    void ByteToString(const void *pData, size_t size, std::string &outStr)
    {
        ESS_ASSERT(size == sizeof(byte));

        int val = *(( const byte*)pData);
        TypeToString<int>(&val, sizeof(val), outStr);
    }

    // ---------------------------------------------------------------------------

    void StringToString(const void *pData, size_t size, std::string &outStr)
    {
        ESS_ASSERT(size > 0);

        outStr.resize(size);
        std::memcpy(&outStr[0], pData, size);
    }

    // ---------------------------------------------------------------------------

    void WstringToString(const void *pData, size_t size, std::string &outStr)
    {
        ESS_ASSERT(size > 0);
        ESS_ASSERT((size & 1) == 0);  // must be even

        outStr.clear();
        outStr.reserve(size >> 1);  // div by 2

        const wchar_t *p = (const wchar_t *)pData;

        while(size--)
        {
            outStr.push_back(*p++);
        }
    }

    // ---------------------------------------------------------------------------

    void BinaryToString(const void *pData, size_t size, std::string &outStr)
    {
        ESS_ASSERT(size > 0);

        Utils::DumpToHexString((void*)pData, size, outStr);
    }

    // ---------------------------------------------------------------------------

    #define GET_TYPEINFO(_type, _desc, _typeSize, _typeStr, _fn)                            \
            TypeInfo GetTypeInfo(_type &dummi)                                              \
            {                                                                               \
                return TypeInfo(_desc, _typeSize, _typeStr, &_fn);                          \
            }                                                                               \

    #define GET_STATIC_TYPEINFO(_type, _desc, _fn)   GET_TYPEINFO(_type, _desc, sizeof(_type), #_type, _fn)
            
    // ---------------------------------------------------------------------------
    
    GET_STATIC_TYPEINFO(bool,   TypeBool,   BoolToString);  
    GET_STATIC_TYPEINFO(byte,   TypeByte,   ByteToString);
    GET_STATIC_TYPEINFO(word,   TypeWord,   TypeToString<word>);
    GET_STATIC_TYPEINFO(int32,  TypeInt32,  TypeToString<int32>);
    GET_STATIC_TYPEINFO(dword,  TypeDword,  TypeToString<dword>);
    GET_STATIC_TYPEINFO(float,  TypeFloat,  TypeToString<float>);
    GET_STATIC_TYPEINFO(double, TypeDouble, TypeToString<double>);
    GET_STATIC_TYPEINFO(int64,  TypeInt64,  TypeToString<int64>);

    GET_TYPEINFO(std::string,       TypeString,  0, "string",  StringToString);
    GET_TYPEINFO(std::wstring,      TypeWstring, 0, "wstring", WstringToString);
    GET_TYPEINFO(std::vector<byte>, TypeBinary,  0, "binary",  BinaryToString);

    BOOST_STATIC_ASSERT(sizeof(bool) == 1);
    
} // namespace

// ---------------------------------------------------------------------------

namespace SBProto
{
    TypeInfo::TypeInfo(TypeInfoDesc typeID, size_t typeSize, 
                       const std::string &typeName, FnPtr pConverter) :
        TypeID(typeID), 
        TypeSize(typeSize), 
        IsDynamic(typeSize <= 0 ? true : false), 
        TypeName(typeName),
        FnToStrConverter(pConverter)
    {
        TUT_ASSERT(pConverter != 0);
    }
    
    // ---------------------------------------------------------------------------
    // TypeInfoTable impl
     
    TypeInfoTable::TypeInfoTable()
    {
        RegisterType<bool>();
        RegisterType<byte>();
        RegisterType<word>();
        RegisterType<int32>();
        RegisterType<dword>();
        RegisterType<float>();
        RegisterType<double>();
        RegisterType<int64>();

        RegisterType<std::string>();
        RegisterType<std::wstring>();
        RegisterType<std::vector<byte> >();
    }

    // ---------------------------------------------------------------------------

    template<class TType> 
    void TypeInfoTable::RegisterType()
    {
        TType var;
        m_types.push_back(GetTypeInfo(var));
    }

    // ---------------------------------------------------------------------------

    const TypeInfo& TypeInfoTable::Find(TypeInfoDesc typeID) const
    {
        for (size_t i = 0; i < m_types.size(); ++i)
        {
            if (m_types.at(i).TypeID == typeID) return m_types.at(i);
        }

        std::ostringstream msg;
        msg << "Invalid TypeID " << (int)typeID;
        ESS_HALT( msg.str() );
        
        return m_types.at(0);  // dummi
    }

    // ---------------------------------------------------------------------------
    
    bool IsValidTypeInfoDesc(byte b)
    {
        return ((b == TypeBool) ||
                (b == TypeByte) || 
                (b == TypeWord) ||
                (b == TypeInt32) ||
                (b == TypeDword) ||
                (b == TypeFloat) ||
                (b == TypeDouble) ||
                (b == TypeInt64) ||
                (b == TypeString) ||
                (b == TypeWstring ) ||
                (b == TypeBinary));    
    }
    
} // namespace SBProto
