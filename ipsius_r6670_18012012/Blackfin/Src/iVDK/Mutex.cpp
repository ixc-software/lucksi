#include "VDK.h"
#include "stdafx.h"  // STL must be after VDK.h :-/
#include "Utils/ErrorsSubsystem.h"
#include "iVDK/VdkCheckInit.h"

namespace
{
	VDK::MutexID CastID(Platform::dword id)
	{
		return static_cast<VDK::MutexID>(id);
	}
}

namespace iVDK
{
    BOOST_STATIC_ASSERT(sizeof(Platform::dword) == sizeof(VDK::MutexID));
	
    Mutex::Mutex()
    {
		VdkCheckInit();
    	
        m_id = VDK::CreateMutex(); 
        ESS_ASSERT(m_id != UINT_MAX);
    }

    Mutex::~Mutex()
    {
        VDK::DestroyMutex( CastID(m_id) );
    }

    void Mutex::Lock()
    {
        VDK::AcquireMutex( CastID(m_id) );
    }

    void Mutex::Unlock()
    {
        VDK::ReleaseMutex( CastID(m_id) );
    }
    
}  // namespace iVDK

