#ifndef __QTENUMRESOLVER__
#define __QTENUMRESOLVER__

#include "Platform/Platform.h"
#include "Utils/IntToString.h"
#include "DelayInit.h"
#include "stdafx.h"
#include "ErrorsSubsystem.h"

namespace Utils
{

	class QtEnumResolverImpl
	{
	public:

		static std::string ResolveSet(const  QMetaEnum &e, int value);

		static const char* Resolve(const  QMetaEnum &e, int value);

		static bool Resolve(const  QMetaEnum &e, const char *inValue, int &outVal);
	};

    /* 
        Вспомогательный класс, для преобразования значения enum в строку
        Enum должен быть описан в потомке QObject и зарегестрирован с помощью Q_ENUMS()

        TQObject -- тип объекта, в котором описан enum
        TEnumType -- тип enum
    */
    template<class TQObject, class TEnumType>
    class QtEnumResolver
    {
		
        // const QMetaEnum *m_pMeta;
        int m_enumIndx;
        std::string m_name;

        std::string GetEnumName()
        {
            using std::string;

            std::string name = Platform::FormatTypeidName(typeid(TEnumType).name());
            size_t pos = name.rfind("::");
            ESS_ASSERT(pos != string::npos);

            return name.substr(pos + 2);
        }

        int SetEnumMeta() const
        {
            const QMetaObject &meta = TQObject::staticMetaObject;

            for(int i = 0; i < meta.enumeratorCount(); ++i)
            {
                if (m_name.compare(meta.enumerator(i).name()) == 0)
                {
                    return i;
                }
            } 

            return -1;
        }

    public:

        ESS_TYPEDEF(EnumMetaNotFound);
        ESS_TYPEDEF(EnumMetaNotResolved);

        QtEnumResolver()  // throw(EnumMetaNotFound)
        {
            m_name = GetEnumName();
            m_enumIndx = SetEnumMeta();
            if (m_enumIndx < 0) ESS_THROW(EnumMetaNotFound);
        }

        const std::string& getName() const
        {
            return m_name;
        }
		std::string ResolveSet(int value) const  // TEnumType value
		{

			const QMetaObject &meta = TQObject::staticMetaObject;
			QMetaEnum e = meta.enumerator(m_enumIndx);
			
			return QtEnumResolverImpl::ResolveSet(e, value);
		}

        std::string Resolve(int value) const  // TEnumType value
        {
            const QMetaObject &meta = TQObject::staticMetaObject;
            QMetaEnum e = meta.enumerator(m_enumIndx);
            const char *p = QtEnumResolverImpl::Resolve(e, value);

            if (p == 0) return "< Bad value " + Utils::IntToString(value) + ">";

            return p;
        }

		TEnumType Resolve(const char *value, bool *resultInd = 0) const  // TEnumType value
		{
			ESS_ASSERT(value != 0);

			const QMetaObject &meta = TQObject::staticMetaObject;
			QMetaEnum e = meta.enumerator(m_enumIndx);

			int  resultVal = -1;
			if(resultInd != 0) *resultInd = false;
			if(QtEnumResolverImpl::Resolve(e, value, resultVal)) 
			{
				if(resultInd != 0) *resultInd = true;
				return TEnumType(resultVal);
			}
			if(resultInd == 0)  ESS_THROW(EnumMetaNotResolved);
			return TEnumType(-1);
		}

    };

	template<class TQObject, class TEnumType>
	class QtEnumResolvHandler
	{
		typedef QtEnumResolvHandler T;

		typedef QtEnumResolver<TQObject, TEnumType> Resolver;

		boost::scoped_ptr<Resolver> m_impl;

		void Init()
		{
			ESS_ASSERT(m_impl == 0);
			
			m_impl.reset(new Resolver());
		}

	public:
		
		QtEnumResolvHandler()
		{
			DelayInit::Functor(boost::bind(&T::Init, this));
		}
		
		Resolver& Impl() 
		{
			return *m_impl;
		}


	};
	
} // namespace Utils


#endif
