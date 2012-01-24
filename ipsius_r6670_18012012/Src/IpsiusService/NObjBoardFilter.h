#ifndef NOBJBOARDFILTER_H
#define NOBJBOARDFILTER_H

#include "Utils/RangeList.h"
#include "Domain/NamedObject.h"
#include "BfBootCore/BroadcastMsg.h"

namespace IpsiusService
{   
    class NObjBoardFilter 
        : public Domain::NamedObject,
        public DRI::INonCreatable
    {
        Q_OBJECT;

        boost::scoped_ptr<Utils::RangeList> m_include;
        boost::scoped_ptr<Utils::RangeList> m_exclude;
        
        bool m_locked;

    private:
        
        QString getExclude() const;
        QString getInclude() const;
        void ThrowIfLocked()
        {
            if (m_locked) ThrowRuntimeException("Disable HwFinder first");
        }

    //DRI interface:
    public:
        Q_INVOKABLE void Include(QString rangeList); // обработка исключений парсинга?
        Q_INVOKABLE void Exclude(QString rangeList); // обработка исключений парсинга?
        Q_INVOKABLE void Clear();

        //Q_PROPERTY(bool Enabled WRITE Activate READ m_active);
        Q_PROPERTY(QString Exclude READ getExclude); 
        Q_PROPERTY(QString Include READ getInclude);
    
    // owner interface
    public:
        NObjBoardFilter(Domain::NamedObject *pParent, const Domain::ObjectName &name);

        bool AcceptedBoard(const BfBootCore::BroadcastMsg& msg);
        void LockChanges(bool lock);
    };

    
} // namespace IpsiusService

#endif
