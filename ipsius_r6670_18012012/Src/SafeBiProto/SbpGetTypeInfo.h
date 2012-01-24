
#ifndef __SAFEBIPROTOTYPES__
#define __SAFEBIPROTOTYPES__

#include "SbpTypes.h"
#include "Utils/ErrorsSubsystem.h"

namespace SBProto
{
    enum TypeInfoDesc
    {
        TypeBool        = 0,                // size == 1
        TypeByte        = 1,                // 1
        TypeWord        = 2,                // 2 (uint16) 
        TypeInt32       = 3,                // 4 (int)
        TypeDword       = 4,                // 4 (uint32) 
        TypeFloat       = 5,                // 4
        TypeDouble      = 6,                // 8
        TypeInt64       = 7,                // 8 

        TypeString      = 128,              // D
        TypeWstring     = 129,              // D
        TypeBinary      = 130,              // D
    };

    // -------------------------------------------

    bool IsValidTypeInfoDesc(byte b);

    // -------------------------------------------
    
    // SafeBiProto types' params
    struct TypeInfo
    {
        TypeInfoDesc TypeID;
        size_t TypeSize;
        bool IsDynamic;
        std::string TypeName;
        
        typedef void (*FnPtr)(const void*, size_t, std::string&);
        FnPtr FnToStrConverter;

        void ToStrConverter(const void *p, size_t size, std::string& s) const
        {
            ESS_ASSERT(FnToStrConverter != 0);

            s.clear();
            if (size == 0) return;

            ESS_ASSERT(p != 0);
            FnToStrConverter(p, size, s);
        }
       
        TypeInfo(TypeInfoDesc typeID, size_t typeSize, 
            const std::string &typeName, FnPtr pConverter);
    };

    // -------------------------------------------
    
    class TypeInfoTable
    {
        std::vector<TypeInfo> m_types;
        
        template<class TType> void RegisterType();

    public:
        TypeInfoTable();

        const TypeInfo& Find(TypeInfoDesc typeID) const;
    };
    
} // namespace SBProto

#endif
