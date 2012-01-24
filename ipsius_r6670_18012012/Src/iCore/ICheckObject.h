#ifndef __ICHECKOBJECT__

#define __ICHECKOBJECT__

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "MsgBase.h"
#include "Utils/AtomicTypes.h"
#include "Utils/IBasicInterface.h" 

namespace iCore
{

    class ICheckGenerator : public Utils::IBasicInterface
    {
    public:
        virtual int GenerateCheckSignature() const = 0;
    };

    class ICheckObject
    {
        // int m_checkSignature;
        Utils::AtomicInt m_checkSignature;

    public:

        ICheckObject(const ICheckGenerator &g)
        {
            while(true)
            {
                m_checkSignature.Set(g.GenerateCheckSignature());
                if (m_checkSignature.Get()) break;
            }
        }

        ~ICheckObject()
        {
            m_checkSignature.Set(0);
        }

        void SignatureCheck(int signature) const
        {
            ESS_ASSERT(signature == m_checkSignature.Get());
        }

        int GetSignature() const 
        { 
            return m_checkSignature.Get(); 
        }

    };


}  // namespace iCore


#endif


