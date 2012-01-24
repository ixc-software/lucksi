#ifndef __MSGTEMPLATES__
#define __MSGTEMPLATES__

#include "stdafx.h"
#include "Utils/DeleteIfPointer.h"
#include "MsgBase.h"
#include "ICheckObject.h"

namespace iCore
{
    namespace mpl = boost::mpl; 

    class MsgObject;

    // Сообщение без параметров
	template<typename TOwner, typename TFn>
	class MsgSimple : public MsgBase
	{
		TOwner m_owner;
		TFn m_fn;

	protected:

		void Execute()  // override for MsgBase
		{
            CheckDestination();
            SignatureCheck();
			(m_owner->*m_fn)(); // (std::mem_fun(m_fn))(m_owner);
		}

	public:
		
		MsgSimple(TOwner owner, TFn fn) : MsgBase( ConvertToMsgObject(owner) ), 
            m_owner(owner), m_fn(fn)
		{
		} 

        MsgBase* Clone()  // override
        {
            return new MsgSimple<TOwner, TFn>(m_owner, m_fn);
        }

        std::string TypeName()  // override
        {
            std::ostringstream oss;
            oss << "Msg " << typeid(TOwner).name();
            return oss.str();
        }

	};

	// ------------------------------------------------------

	/* 
        Сообщение с одним параметром

        Если TParam является указателем, то он удаляется в деструкторе (если keepParam = false)
        Для обмена данными между объектами через указатели использовать boost::shared_ptr<>
        Параметр не удаляется, если в конструктор передали параметр keepParam = true
        или TParam является потомком Utils::INonDestructable
    */

	template<typename TOwner, typename TFn, typename TParam>
	class MsgParam : public MsgBase
	{
		TOwner m_owner;
		TFn m_fn;
		TParam m_param;
        bool m_keepParam;

        // TParam can't be (char*, const char*, wchar_t*, const wchar_t) 'couse auto delete
        BOOST_STATIC_ASSERT( !(boost::is_same<TParam, char*>::value) ); 
        BOOST_STATIC_ASSERT( !(boost::is_same<TParam, const char*>::value) ); 
        BOOST_STATIC_ASSERT( !(boost::is_same<TParam, wchar_t*>::value) ); 
        BOOST_STATIC_ASSERT( !(boost::is_same<TParam, const wchar_t*>::value) ); 

	protected:

		void Execute()  // override for MsgBase
		{
            CheckDestination();
            SignatureCheck();
			(m_owner->*m_fn)(m_param); // (std::bind2nd(std::mem_fun(m_fn), m_param))(m_owner);
		}

	public:

		MsgParam(TOwner owner, TFn fn, TParam param, bool keepParam = false) 
			: MsgBase( ConvertToMsgObject(owner) ), 
            m_owner(owner), m_fn(fn), m_param(param), m_keepParam(keepParam)
		{
		} 

        MsgBase* Clone()  // override
        {
            ESS_ASSERT(m_keepParam);  // защита от повторного удаления параметра

            return new MsgParam<TOwner, TFn, TParam>(m_owner, m_fn, m_param, m_keepParam);
        }

        std::string TypeName()  // override
        {
            std::ostringstream oss;
            oss << "Msg " << typeid(TOwner).name() << " param " << typeid(TParam).name();
            return oss.str();
        }

        ~MsgParam()
        {
            if (m_keepParam) return;
            Utils::DeleteIfPointer(m_param);
        }

	};


} // namespace iCore


#endif



