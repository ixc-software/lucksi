
#ifndef __MSGOBJECT_T_
#define __MSGOBJECT_T_

#include "stdafx.h"

#include "MsgObject.h"

namespace iCore
{

	/*
			������� ��� ��� �������, �������� �������� ���������

	*/
    template <class T> 
	class MsgObjectT : public MsgObject	
    {
	protected:
		void PutMsg(void (T::*fn)(void))
		{
			MsgObject::PutMsg(static_cast<T*>(this), fn);
		}

		template<class Par>
		void PutMsg(void (T::*fn)(Par), Par param, bool keepParam = false)
		{
			MsgObject::PutMsg(static_cast<T*>(this), fn, param, keepParam);
		}

	public:
		MsgObjectT(MsgThread &thread) : MsgObject(thread)
		{
		}
	};
};

#endif


