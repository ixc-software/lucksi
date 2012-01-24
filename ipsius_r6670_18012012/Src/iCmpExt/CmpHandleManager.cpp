#include "stdafx.h"
#include "CmpHandleManager.h"


namespace iCmpExt
{

	int CmpHandleManager::AllocHandle()
	{
		int handle = 0;
		List::Iterator i = m_freeHandle.Begin();
		if(i != m_freeHandle.End())
		{
			handle = *i;
			m_freeHandle.Clear(i);
		}
		else if(m_busyHandle.IsEmpty())
		{
			handle = 1;
		}
		else
		{
			handle = 1 + m_busyHandle.MaxValue();
		}

		m_busyHandle.Set(handle);
		return handle;
	}

	void CmpHandleManager::FreeHandle(int handle)
	{
		ESS_ASSERT(m_busyHandle.IsExist(handle));
		ESS_ASSERT(!m_freeHandle.IsExist(handle));

		m_busyHandle.Clear(handle);
		m_freeHandle.Set(handle);
	}

};





