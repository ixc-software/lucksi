#ifndef __QSYNCDELETE__
#define __QSYNCDELETE__

#include "stdafx.h"
#include "AtomicTypes.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/ThreadSyncEvent.h"

namespace Utils
{

    // класс синхронного удаления QObject
    class QSyncDelete
    {
        typedef ThreadSyncEvent Event;

        bool m_allowAsyncDelete;
        boost::shared_ptr<Event> m_event;

    protected:

        virtual ~QSyncDelete() 
        {
            if (m_event.get() != 0)
            {
                // send signal -- we are destroyed!
                m_event->Ready();
            }
            else
            {
                // проверить, что объект допускает удаление не через функцию DoSyncDelete()
                ESS_ASSERT(m_allowAsyncDelete);
            }

        }

    public:

        QSyncDelete(bool allowAsyncDelete = false) : m_allowAsyncDelete(allowAsyncDelete)
        {
        }

        void DoSyncDelete()
        {
            ESS_ASSERT(m_event.get() == 0);  // assert -- first call

            QObject *pObject = dynamic_cast<QObject*>(this);
            ESS_ASSERT(pObject != 0);

            m_event.reset(new Event());
            pObject->deleteLater();            
            m_event->Wait();           // опасно, т.к. сюда мы вернемся после вызова деструктора ?
        }

    };

}  // namespace Utils

#endif
