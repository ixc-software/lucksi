#ifndef __MAKESECURERESULT__
#define __MAKESECURERESULT__

#include "Platform/Platform.h"
#include "Utils/ManagedList.h"

namespace ChipSecure
{
    using Platform::dword;

    class SecureSingleItem;

    class SecureResults : boost::noncopyable
    {
        Utils::ManagedList<SecureSingleItem> m_list;
        dword m_mcsPerTransform;

    public:

        SecureResults();        
        ~SecureResults();

        void Clear();
        void Add(SecureSingleItem *item);  // get ownership

        dword CRC() const;
        dword McsPerTransform() const { return m_mcsPerTransform; }
        void McsPerTransform(dword val) { m_mcsPerTransform = val; }

        std::string Dump() const;

    };

    // TODO -- use more complex random + seed
    void MakeSecureResult(dword seed, dword count, bool verify, SecureResults &result);

    void MakeSecureDump();
    
}  // namespace ChipSecure

#endif
