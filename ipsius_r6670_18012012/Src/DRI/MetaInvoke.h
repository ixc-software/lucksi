#ifndef __METAINVOKE__
#define __METAINVOKE__

#include "Utils/IBasicInterface.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/ManagedList.h"

#include "DriArgFactory.h"
#include "driutils.h"
#include "commanddriresult.h"

namespace Domain
{
    class NamedObject;

}  // namespace Domain

namespace DRI
{
    
    class MetaInvoke 
    {       
        DriArgFactory &m_argFactory;
        Domain::NamedObject *m_pObj;
        QMetaMethod m_method;
        std::string m_methodName;
        Utils::ManagedList<IQGenericArgument> m_args;
        bool m_invoked;

        class SpecialParam : public IQGenericArgument
        {
            std::string m_typeName;
            const void* m_pData;

            QGenericArgument m_arg;

        // IQGenericArgument impl
        private:

            QGenericArgument& GetGenericArgument()
            {
                return m_arg;
            }

        public:

            SpecialParam(const std::string &typeName, const void *pData) :
              m_typeName(typeName),
                  m_pData(pData),
                  m_arg(m_typeName.c_str(), static_cast<const void *>(&m_pData))
              {
              }
        };

        class EmptyParam : public IQGenericArgument
        {
            QGenericArgument m_arg;

        // IQGenericArgument impl
        private:

            QGenericArgument& GetGenericArgument()
            {
                return m_arg;
            }

        };

        void ThrowArgTypeNotFound(const std::string &typeName)
        {
            std::string msg = "Type not found -- ";
            msg += typeName;
            ESS_THROW_MSG(DRI::DriArgException, msg);
        }

        void AddDefaultParams()
        {
            while(true)
            {
                std::string typeName = GetTopTypename();
                if (typeName.size() == 0) break;

                IArgFactoryType *pType = m_argFactory.Find(typeName);
                if (pType == 0) ThrowArgTypeNotFound(typeName);

                m_args.Add( pType->DefaultValue() );
            }
        }

        std::string GetTopTypename()
        {
            int indx = m_args.Size();

            QList<QByteArray> argTypes = m_method.parameterTypes();
            if (indx >= argTypes.size()) return "";

            return argTypes.at(indx).constData();
        }

        void AddParam(const QString &paramValue);

    public:

        MetaInvoke(DriArgFactory &argFactory, Domain::NamedObject *pObj, const QMetaMethod &method) : 
            m_argFactory(argFactory),
            m_pObj(pObj),
            m_method(method),
            m_invoked(false)
        {
            m_methodName = DriUtils::MethodName(m_method);
        }

        bool Invoked() const { return m_invoked; }

          template<class T>
          void AddSpecialParam(T p)
          {
              std::string typeName = DriUtils::QtTypename(p).toStdString();
              ESS_ASSERT(GetTopTypename() == typeName);

              m_args.Add( new SpecialParam(typeName, p) );

              //QGenericArgument arg(typeName.c_str(), static_cast<const void *>(&p));
              //m_args.push_back(arg);
          }

          void AddParams(const QStringList &params)
          {
              for(int i = 0; i < params.size(); ++i)
              {
                  AddParam( params.at(i) );
              }

              // AddDefaultParams();
          }

          void Invoke();

          QString InvokeInfo() const;

          // without root
          QString ObjName() const;

          const QMetaMethod& Method() const { return m_method; }

    };

    
    
}  // namespace DRI

#endif
