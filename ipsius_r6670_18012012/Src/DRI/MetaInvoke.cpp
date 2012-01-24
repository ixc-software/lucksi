#include "stdafx.h"

#include "Domain/NamedObject.h"

#include "MetaInvoke.h"
#include "NamedObjectFactory.h"
#include "driutils.h"

#include "Utils/QObjFactory.h"

// ---------------------------------------

namespace
{
    using namespace DRI;

	bool GSearchEnumInGlobalMetaObjects= true;

    class EnumArg 
    {

        static QMetaEnum GetMetaEnum(const std::string &typeName, Domain::NamedObject *pObj)
        {
            std::string enumType = typeName;

            // try to setup pMeta
            const QMetaObject *pMeta = 0;

            size_t i = typeName.rfind("::");
			if (i != std::string::npos) 
            {
                std::string classType = typeName.substr(0, i);                
                INamedObjectType *pN = NamedObjectFactory::Instance().Find(classType);

                if (pN != 0)
					pMeta = &pN->MetaObject();
				else if (GSearchEnumInGlobalMetaObjects) 
					pMeta = Utils::QObjFactory::Instance().FindQMetaObject(classType);

	            if (pMeta == 0)
				{
					std::string msg = "For arg type " + typeName;
					msg += " enum holder class " + classType + " not found";
					ESS_THROW_MSG(DRI::DriArgException, msg);                    
				}

                enumType = typeName.substr(i + 2); 
            }
            else
            {
                ESS_ASSERT(pObj != 0);
                pMeta = pObj->metaObject();
            }

            ESS_ASSERT(pMeta != 0);

            int enumIndex = pMeta->indexOfEnumerator( enumType.c_str() );

            if (enumIndex < 0) 
			{
                std::string msg = "For arg type " + typeName;
                msg += " enum " + enumType + " not found";
                ESS_THROW_MSG(DRI::DriArgException, msg);                    
            }

			return pMeta->enumerator(enumIndex);
        }

        class EnumParam : public IQGenericArgument
        {
            enum Any { anyValue, };

            std::string m_typeName;
            Any m_enum;

            QGenericArgument m_arg;

        // IQGenericArgument impl
        private:

            QGenericArgument& GetGenericArgument()
            {
                return m_arg;
            }

        public:

            EnumParam(const std::string &typeName, int value) :
                m_typeName(typeName),
                m_enum( (Any)value ),
                m_arg(m_typeName.c_str(), static_cast<const void *>(&m_enum))
              {
              }
        };

    public:

        static IQGenericArgument* TryCreate(DriArgFactory &argFactory,
            const std::string &typeName, const std::string &value, Domain::NamedObject *pObj)
        {
            QMetaEnum e = GetMetaEnum(typeName, pObj);

            if ( !DriUtils::CorrectEnumKey(e, value.c_str()) )
            {
                std::string msg = "For arg type " + typeName;
                msg += " bad enum value " + value;
                ESS_THROW_MSG(DRI::DriArgException, msg);                    
            }

            IArgFactoryType *pT = argFactory.Find("int");
            ESS_ASSERT(pT != 0);

            int i = e.keyToValue( value.c_str() );
            return new EnumParam(typeName, i);
        }

    };


}  // namespace


// ---------------------------------------

namespace DRI
{
            
    void MetaInvoke::Invoke()
    {
        const int CMaxParams = 10;

        ESS_ASSERT(!m_invoked);

        // add empty params
        while(m_args.Size() < CMaxParams) m_args.Add( new EmptyParam() );
        ESS_ASSERT(m_args.Size() == CMaxParams);

        // check destination
        m_pObj->BeforeInvoke();

        // exec            
        bool res = QMetaObject::invokeMethod(m_pObj, m_methodName.c_str(), 
            Qt::DirectConnection, QGenericReturnArgument(), 
            m_args[0]->GetGenericArgument(), 
            m_args[1]->GetGenericArgument(), 
            m_args[2]->GetGenericArgument(), 
            m_args[3]->GetGenericArgument(), 
            m_args[4]->GetGenericArgument(), 
            m_args[5]->GetGenericArgument(), 
            m_args[6]->GetGenericArgument(), 
            m_args[7]->GetGenericArgument(), 
            m_args[8]->GetGenericArgument(), 
            m_args[9]->GetGenericArgument() );

        if (!res) 
        {
            std::string msg = "Can't invoke for ";
            msg += m_methodName;
            ESS_THROW_MSG(DRI::InvokeException, msg);
        }

        m_invoked = true;
    }

    // -----------------------------------------------------------

    QString MetaInvoke::InvokeInfo() const
    {
        ESS_ASSERT(m_pObj != 0);

        return QString(" @ %1.%2")
            .arg(m_pObj->Name().Name(true))
            .arg(m_methodName.c_str());
    }

    // -----------------------------------------------------------

    QString MetaInvoke::ObjName() const
    {
        ESS_ASSERT(m_pObj != 0);
        
        return m_pObj->Name().Name();
    }

    // -----------------------------------------------------------

    void MetaInvoke::AddParam( const QString &paramValue )
    {
        std::string typeName = GetTopTypename();
        if (typeName.empty())
        {
            ESS_THROW(DriMethodExtraParams);
        }

        IArgFactoryType *pType = m_argFactory.Find(typeName);

        if (pType == 0) 
        {
            IQGenericArgument *pArg = 
                EnumArg::TryCreate(m_argFactory, typeName, paramValue.toStdString(), m_pObj);

            if (pArg == 0) ThrowArgTypeNotFound(typeName);

            m_args.Add(pArg);

            return;
        }

        m_args.Add( pType->FromString(paramValue) );
    }

}  // namespace DRI

