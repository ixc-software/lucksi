#include "stdafx.h"
#include "isdninfr.h"

#include "Platform/Platform.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/Random.h"

namespace ISDN
{

    IsdnAllocDefault::~IsdnAllocDefault()
    {
        if (alloc_count != 0)
            ESS_ASSERT(  alloc_count == 0  &&  "Memory outflow!" );
    }

    // ------------------------------------------------------------------------------------

    void IsdnInfraDefault::free( void* ptr )
    {
        allocator.free(ptr);
    }

    // ------------------------------------------------------------------------------------

    void* IsdnAllocDefault::alloc( size_t size )
    {
        ++alloc_count;
        return new( byte [size]);
    }

    // ------------------------------------------------------------------------------------

    void IsdnAllocDefault::free( void* p )
    {
        byte* pp = static_cast<byte*>(p);
        delete[] pp;
        --alloc_count;
    }

    // ------------------------------------------------------------------------------------

    IsdnInfraDefault::IsdnInfraDefault( Utils::Random* pRnd, iLogW::ILogSessionToLogStore& storeIntf, iLogW::LogSessionProfile& logSessionProf ) : m_pRnd(pRnd),
        m_storeIntf(storeIntf),
        m_logSessionProf(logSessionProf)
    {
    }

    // ------------------------------------------------------------------------------------

    ISDN::dword IsdnInfraDefault::GetTick() const
    {
        return Platform::GetSystemTickCount();
    }

    // ------------------------------------------------------------------------------------

    ISDN::dword IsdnInfraDefault::GetSeed()
    {
        return m_pRnd->Next();
    }

    // ------------------------------------------------------------------------------------

    void* IsdnInfraDefault::alloc( size_t size )
    {
        return allocator.alloc(size);
    }

    // ------------------------------------------------------------------------------------
    
    iLogW::ILogSessionToLogStore& IsdnInfraDefault::getLogStoreIntf()
    {
        return m_storeIntf;
    }

    // ------------------------------------------------------------------------------------

    iLogW::LogSessionProfile& IsdnInfraDefault::getLogSessionProfile()
    {
        return m_logSessionProf;
    }

    
} // namespace ISDN

