#ifndef __OBJECTLINKBINDER__
#define __OBJECTLINKBINDER__

#include "stdafx.h"
#include "Utils/AtomicTypes.h"
#include "ObjectLink.h"

namespace ObjLink
{
    using boost::shared_ptr;

    class ObjLinkStoreBinder;

    // базовый класс для ObjLinkBinder, нужен для полиморфного хранения в ObjLinkStoreBinder
    class ObjLinkBinderBase
    {
    public:
        virtual void Clear() = 0;
        virtual ~ObjLinkBinderBase() {}
    };

    /* Класс передается через shared_ptr<> в сообщении для подключения к 
     интрефейсу типа TIntf, т.е. хранит указатель на интерфейс + хост, плюс
     контроль времени жизни этих указателей с помощью класса ObjLinkStoreBinder */
    template<class TIntf>
    class ObjLinkBinder : public ObjLinkBinderBase, boost::noncopyable
    {
        // check TIntf must be IObjectLinkInterface
        enum { CCheckBase = boost::is_base_of<IObjectLinkInterface, TIntf>::value };      
        BOOST_STATIC_ASSERT(CCheckBase);

        IObjectLinksHost* m_pHost;
        TIntf* m_pIntf;

        friend class ObjLinkStoreBinder;

        template<class T>
        ObjLinkBinder(T* p) : m_pHost(p), m_pIntf(p)
        {}

        void Clear()  // override
        {
            m_pHost = 0;
            m_pIntf = 0;
        }

    public:
		bool IsEmpty() const 
		{
			return m_pIntf == 0;
		}
        bool Connect(ObjectLink<TIntf> &link) const
        {
            if (m_pIntf == 0) return false;

            link.Connect(m_pHost, m_pIntf);
            return true;
        }

    };

    //----------------------------------------------------------------

    /* 
       Создание и хранение списка объектов типа ObjLinkBinder<TIntf>, 
     (храним как shared_ptr<ObjLinkBinderBase>). Деструктор очищает все созданные 
     в getBinder() ссылки, т.к. они могут находитья в очереди сообщений в момент 
     удаления класса-сервера.     
    */
    class ObjLinkStoreBinder
        : boost::noncopyable // тк владелец предположительно некопируемый
    {
        typedef shared_ptr<ObjLinkBinderBase> Item;
        typedef std::vector<Item> List;

        enum { CCleanupMask = 0x0F };  // must be binary with all 1 at low digits

        Utils::AtomicInt m_counter;
        List m_list;

        void DoCleanup()
        {
            List::iterator i = m_list.begin();

            while(i != m_list.end())
            {
                if ((*i).unique())
                {
                    i = m_list.erase(i);
                    continue;
                }

                ++i;
            }
        }

    public:

        ObjLinkStoreBinder() 
        {}

        ~ObjLinkStoreBinder()
        {
            for(List::iterator i = m_list.begin(); i != m_list.end(); ++i)
            {
                (*i)->Clear();
            }
        }

        template<class TIntf, class TOwner>
        shared_ptr< ObjLinkBinder<TIntf> > getBinder(TOwner *pOwner)
        {
            shared_ptr< ObjLinkBinder<TIntf> > result(new ObjLinkBinder<TIntf>(pOwner));
            m_list.push_back(result);

            m_counter.Inc();
            if ((m_counter.Get() & CCleanupMask) == 0)
            {
                DoCleanup();
            }

            return result;
        }
    };


	
}  // namespace ObjLink


#endif



