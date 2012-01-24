#ifndef __LOGSESSIONBODY__
#define __LOGSESSIONBODY__

#include "Utils/IVirtualDestroy.h"
#include "Utils/ManagedList.h"

namespace iLog
{

    class ISessionBodyDestroy : public Utils::IBasicInterface
    {
    public:
        virtual void SafeSessionDestroy(Utils::IVirtualDestroy *pObject) = 0;
        virtual void UnsafeSessionDestroy(Utils::IVirtualDestroy *pObject) = 0;
    };

    // ----------------------------------------------

	// тело "сессии", хранящееся по подсчитываемым ссылкам в записях и сессии	
	template<class TData>
    class LogSessionBody : public Utils::IVirtualDestroy, boost::noncopyable
	{
        ISessionBodyDestroy &m_destroy;
        int m_useCount;
        const TData m_sessionName;
        Utils::ManagedList<TData> m_kinds;  // record kinds

	public:

        LogSessionBody(ISessionBodyDestroy &destroy, const TData &sessionName)
            : m_destroy(destroy), m_sessionName(sessionName)
        {
            m_useCount = 1;
        }

        ~LogSessionBody()
        {
            ESS_ASSERT(m_useCount == 0);
        }

        const TData& Name() const { return m_sessionName; }

        void IncUsage()
        {
            m_useCount++;
        }

        void DecUsage(bool safeContext)
        {
            ESS_ASSERT(m_useCount > 0);

            m_useCount--;

            if (m_useCount == 0) 
            {
                if (safeContext) m_destroy.SafeSessionDestroy(this);
                            else m_destroy.UnsafeSessionDestroy(this);
            }
        }

        TData* RegisterRecordKind(const TData &data)
        {
            ESS_ASSERT(data.size() > 0);

            // dublicates check
            for(int i = 0; i < m_kinds.Size(); ++i)
            {
                if (*(m_kinds[i]) == data) ESS_HALT("Dublicate records kind!");
            }

            // register
            TData *pResult = new TData(data);
            m_kinds.Add(pResult);

            return pResult;
        }
				
	};

    // ----------------------------------------------

    template<class TData>
    class RecordKind
    {
        TData *m_pData;
		bool m_isActive;
        LogSessionBody<TData> *m_pSession;

    public:

        RecordKind() : m_pData(0), m_pSession(0), m_isActive(false)
        {
        }

        RecordKind(TData *pData, LogSessionBody<TData> *pSession)
            : m_pData(pData), m_pSession(pSession), m_isActive(false)

        {
            ESS_ASSERT( !Empty() );
        }

        bool Empty() const
        {
            return (m_pData == 0) || (m_pSession == 0);
        }
		bool IsActive() const	{	return m_isActive; };
		void Activate(bool par)	{	m_isActive = par;  }
        bool SessionEqual(LogSessionBody<TData> *pSession)
        {
            return (pSession == m_pSession);
        }

        const TData& Data() const
        {
            ESS_ASSERT( !Empty() );
            return *m_pData;
        }

    };

	
}  // namespace iLog

#endif
