#ifndef COMMON_IE
#define COMMON_IE

#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "Utils/BaseSpecAllocated.h"
#include "Utils/ComparableT.h"

#include "DssWarning.h"
#include "IIsdnAlloc.h"
#include "IsdnUtils.h"

#include "IeId.h"
#include "ICommonIe.h"
#include "RawData.h"

namespace ISDN
{
    using IsdnUtils::ShortName;            
    
    template<byte id, byte codSet>
    class BaseIe :
        boost::noncopyable,
        public Utils::BaseSpecAllocated<IIsdnAlloc>,
        public ICommonIe
    {
        mutable std::string m_myNameCash;

    public:

        enum
        {
            CIeID = id,
            CIeCodeSet = codSet,
        };       

        IeId GetIeId()const { return GetId(); } // override
        static IeId GetId() { return IeId(id, codSet); }

    // ICommonIe
    private:
        const std::string& GetName()const
        {
            if ( m_myNameCash.empty() )  
            {                
                std::string name = Platform::FormatTypeidName( typeid(*this).name() );
                m_myNameCash = ShortName(name);
            }
            return m_myNameCash;
        }

        const std::vector<byte>& Get() const
        {
            if (!m_RawIe.empty()) return m_RawIe;
            m_mode = readOnly;
            Serialize(m_RawIe);
            return m_RawIe;
        }

    protected:   
       
        enum Modes
        {
            readOnly,
            writeOnly,
        };

        BaseIe(Modes mode) : m_mode(mode){}

        void FixLen() const
        {
            //добавить проверки
            m_RawIe.at(1) = (m_RawIe.size()-2);
        }

        static byte ExtractValue(byte src, byte loBit, byte hiBit )
        {
            ESS_ASSERT(/*loBit >= 0 &&*/ loBit < 8 /*&& hiBit >= 0*/ && hiBit < 8 /*&& hiBit >= loBit*/);
            src <<= 7 - hiBit;
            src >>= 7 - hiBit + loBit;
            return src;
        }

        //Assert len field, raw size and minimal fullSize are coordinated.
        static void AssertSizeAndId(const RawData& raw, int fullSize, IIsdnAlloc& alloc)
        {
            ESS_ASSERT(fullSize >= 2);
            int len = raw[1];
            if (raw.Size() < fullSize || len < fullSize - 2 || len + 2 > raw.Size())
            {
                ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(alloc) ));
            }
            ESS_ASSERT( (raw[0] & 0x7f) == GetId().Id() ); // ѕроверка соответстви€ IeId
        }        
        

        Modes Mode() const
        {
            return m_mode;
        }

        void AssertReadOnlyMode() const
        {
            ESS_ASSERT(m_mode == readOnly);
        }                

    private:
        virtual void Serialize(std::vector<byte>& raw) const = 0;         

    private:        
        mutable Modes m_mode;
        mutable std::vector<byte> m_RawIe;
    };

} // namespace ISDN

#endif

