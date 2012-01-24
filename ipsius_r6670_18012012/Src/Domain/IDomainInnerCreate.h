#ifndef __IDOMAININNERCREATE__
#define __IDOMAININNERCREATE__

#include "stdafx.h"

namespace Domain
{
    class DomainClass;
	
    /* 
        ��������� ��� �������� ����� �������� ������ ������
        ���������� ��� ������� �� ������� DomainInnerObject � ������������ 
    */
    class IDomainInnerCreate : public Utils::IBasicInterface
    {
    public:
        virtual void Create(DomainClass &domain) = 0;
        virtual void Destroy() = 0;
    };

    // RAII ������� ��� IDomainInnerCreate
    class DomainInnerCreator
    {
        IDomainInnerCreate &m_intf;

    public:

        DomainInnerCreator(IDomainInnerCreate &intf, DomainClass &domain) : m_intf(intf)
        {
            m_intf.Create(domain);
        }

        ~DomainInnerCreator()
        {
            m_intf.Destroy();
        }
    };

    // ������, ����������� ��������� ����� ��� ����� ��������� IDomainInnerCreate
    // ����������� TObject ������ ���� TObject(DomainClass &domain, TParams &params)
    template<class TObject, class TParams>
    class DomainInnerObject : public IDomainInnerCreate
    {
        TObject *m_pObject;
        TParams &m_params;

    // IDomainInnerCreate impl
    private:

        virtual void Create(DomainClass &domain)
        {
            ESS_ASSERT(m_pObject == 0);
            m_pObject = new TObject(domain, m_params);
        }

        virtual void Destroy()
        {
            ESS_ASSERT(m_pObject != 0);
            delete m_pObject;
            m_pObject = 0;
        }

    public:
        DomainInnerObject(TParams &params) : 
          m_pObject(0), m_params(params)
        {
        }

        ~DomainInnerObject()
        {
            ESS_ASSERT(m_pObject == 0);
        }
    };

	
}  // namespace Domain

#endif

