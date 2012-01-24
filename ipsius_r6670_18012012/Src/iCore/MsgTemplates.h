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

    // ��������� ��� ����������
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
        ��������� � ����� ����������

        ���� TParam �������� ����������, �� �� ��������� � ����������� (���� keepParam = false)
        ��� ������ ������� ����� ��������� ����� ��������� ������������ boost::shared_ptr<>
        �������� �� ���������, ���� � ����������� �������� �������� keepParam = true
        ��� TParam �������� �������� Utils::INonDestructable
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
            ESS_ASSERT(m_keepParam);  // ������ �� ���������� �������� ���������

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



