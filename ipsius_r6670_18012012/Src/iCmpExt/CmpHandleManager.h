#ifndef _CMP_HANDLE_MANAGER_H_
#define _CMP_HANDLE_MANAGER_H_

#include "Utils/IntSet.h"
#include "Utils/WeakRef.h"

namespace iCmpExt
{
	class CmpHandleManager : boost::noncopyable
	{
		typedef Utils::IntSet List;
		
		Utils::WeakRefHost m_selfRefHost;

		List m_freeHandle;
		List m_busyHandle;
	public:
		Utils::WeakRef<CmpHandleManager&> SelfRef()
		{
			return m_selfRefHost.Create<CmpHandleManager&>(*this);
		}
		int AllocHandle();
		void FreeHandle(int handle);
		std::string ToString(const std::string &sep = "\n") const
		{
			return "Free: " + m_freeHandle.ToString() + sep + "Busy: " + m_busyHandle.ToString();
		}
	};

	class CmpHandleWrapper : boost::noncopyable
	{
	public:
		CmpHandleWrapper(CmpHandleManager &handleList) : m_handleList(handleList.SelfRef())
		{
			m_handle = m_handleList.Value().AllocHandle();
		}
		CmpHandleWrapper(CmpHandleWrapper &handle)
		{
			m_handleList = handle.m_handleList;
			handle.m_handleList = Utils::WeakRef<CmpHandleManager&>();
			m_handle = handle.m_handle;
		}
		~CmpHandleWrapper()
		{
			if(m_handleList.Valid()) m_handleList.Value().FreeHandle(m_handle);
		}
		int Handle() const 
		{	
			return m_handle; 
		}

	private:
		Utils::WeakRef<CmpHandleManager&> m_handleList;
		int m_handle;
	};

};

#endif




