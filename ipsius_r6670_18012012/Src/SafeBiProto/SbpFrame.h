
#ifndef __SBP_FRAME__
#define __SBP_FRAME__

// SbpFrame.h

#include "Utils/ManagedList.h"

#include "SbpException.h"
#include "SbpGetTypeInfo.h"

namespace SBProto
{
    class Frame :
        boost::noncopyable
    {
        // TypeInfoDesc m_type;
        TypeInfo m_type;
        const void *m_pData;
        dword m_length;

    public:
        Frame(TypeInfoDesc type, const void *pData, dword length);

        const TypeInfo& Type() const { return m_type; }
        const void* DataPtr() const { return m_pData; }
        dword Length() const { return m_length; }

        // with type check
        byte AsByte() const;
        bool AsBool() const ;
        // char AsChar() const;
        word AsWord() const;
        int32 AsInt32() const;
        int64 AsInt64() const;
        dword AsDword() const;
        float AsFloat() const;
        double AsDouble() const;
        std::string AsString() const;
        std::wstring AsWstring() const;
        std::vector<byte> AsBinary() const;
        
        void AsString(std::string &data) const;
        void AsWstring(std::wstring &data) const;
        void AsBinary(std::vector<byte> &data) const;
        int AsBinary(void *pData, size_t size) const;

        std::string ConvertToString() const;
        std::string Value() const;
        std::string TypeAsString() const;

        // returns true if no parsing errors occurs
        static bool SplitDataToFrames(const byte *pData, size_t size, 
                                      Utils::ManagedList<Frame> &outList);
    };
    
} // namespace SBProto

#endif
