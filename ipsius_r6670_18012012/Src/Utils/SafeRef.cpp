#include "stdafx.h"
#include "ProjConfigLocal.h"
#include "Platform/Platform.h"
#include "Utils/SafeRef.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/ILockable.h"

/*

    ��������� �� ����������� ������

    1. � GCC �� �������� ����������� ����� �����������, �.�. typeid() �� 
    �������� ��� �������������� ������������ �����.
       ����� ��������� typeid � runtime, �� ��� ���������� "�������" 
    ��������� � ���� �������, ��� �� �������� ����������

    2. ���������� ���� ������� ����������� �� ������� ������������������
    (�����������, ��� ����������� ����� ������). 
      ��������, ��� ~2.0M, ����� ~1.95M

    3. ����� �������� hooking ��� ����������� ������� ������ � �����������.
    ������ �� ����� ����, �.�. ���������� ��� ������-������ ��� �����������
    ��������� ��������. �������������� ������� -- ������ � ��� SafeRefServer
    ����� tagging.

    4. ���� CEnableDebug ������ � .cpp, ����� �� ������������� ��������� 
    ��������������

    5. ��������� ������ CCacheServerType � CRawNamesMode ����� �����������
    �������� ������ � ������ ������� (� ���� ������ ������� ����� 
    ��������� ���-�� � ��� ����)


*/


// ------------------------------------------------

namespace Utils
{
    using namespace ProjConfig;

    class SafeRefServerDebug
    {
        class Item
        {
            BaseSafeRef *m_pRef; 
            std::string m_serverType;

        public:
            Item(BaseSafeRef *pRef, const std::string &serverType)
                : m_pRef(pRef), m_serverType(serverType)
            {
            }

            bool Equal(BaseSafeRef *pRef)
            {
                return (m_pRef == pRef);
            }

            std::string GetInfo()
            {
                std::string s;
                s = "Server " + m_serverType + "; Ref " + m_pRef->GetTypeInfo();
                return s;
            }

            
        };

        class List : public std::vector<Item*>, public Utils::ILockable {};

        List m_list;
        std::string m_serverType;

        List::iterator Find(BaseSafeRef *pRef)
        {
            for(List::iterator i = m_list.begin(); i != m_list.end(); ++i)
            {
                if ((*i)->Equal(pRef)) return i;
            }

            return m_list.end();
        }

        void GetTypeName(const SafeRefServer *pServer, /* out */ std::string &name)
        {
            name = typeid(*pServer).name();
            if (!CfgSafeRef::CRawNamesMode) name = Platform::FormatTypeidName(name.c_str());
        }

    public:

        void Add(BaseSafeRef *pRef, const SafeRefServer *pServer)
        {
            Locker locker(m_list);

            ESS_ASSERT( Find(pRef) == m_list.end() );

            Item *pItem = 0; 

            // ��������� �����������, ������� Item ��������� � ���� ������ ������
            if (CfgSafeRef::CCacheServerType)
            {
                if (m_serverType.empty()) GetTypeName(pServer, m_serverType);
                pItem = new Item(pRef, m_serverType);
            }
            else
            {
                std::string typeName;
                GetTypeName(pServer, typeName);
                pItem = new Item(pRef, typeName);
            }

            m_list.push_back(pItem);
        }

        void Remove(BaseSafeRef *pRef)
        {
            Locker locker(m_list);

            List::iterator i = Find(pRef);
            ESS_ASSERT( i != m_list.end() );

            delete (*i);
            m_list.erase(i);
        }

        std::string GetItemsInfo()
        {
            Locker locker(m_list);

            ESS_ASSERT( m_list.size() > 0);

            std::ostringstream ss;

            ss << "~SafeRef list" << "\n";
            
            for(List::iterator i = m_list.begin(); i != m_list.end(); ++i)
            {
                ss << (*i)->GetInfo() << "\n";
            }
            
            return ss.str();
        }

        ~SafeRefServerDebug()
        {
            // �������������, ����� ��� ������ ����� Utils::RunSafeRefTest()
            // ESS_ASSERT( m_list.empty() );  
        }

    };

    // ------------------------------------------------

	
    SafeRefServer::SafeRefServer()
    {
        Init();
    }

    SafeRefServer::SafeRefServer(const std::string &name) : m_objectName(name)
    {
        Init();
    }

    SafeRefServer::~SafeRefServer()
    {          
        if (m_ref.Get() == 0) return;  // OK

        // halt with debug info 
        std::ostringstream oss;

        oss << "~SafeRefServer " << m_ref.Get();

        if ( m_typeInfo.size() )   oss << " TYPE {" << m_typeInfo << "}";
        if ( m_objectName.size() ) oss << " NAME " << m_objectName;
        
        if (CfgSafeRef::CEnableDebug)
        {
            std::string info = m_debug->GetItemsInfo();
            if ( info.size() ) oss << " EXTRA " << info;
        }

        ESS_HALT( oss.str() );        
    }

    void SafeRefServer::IncSafeRef(BaseSafeRef *pRef) const  
    { 
        int prevVal = m_ref.FetchAndAdd(1);
        
        if ( (prevVal == 0) && m_typeInfo.empty() )  // init type info
        {
            m_typeInfo = typeid(*this).name();
        }

        if (CfgSafeRef::CEnableDebug) 
        {
            // std::string typeName = FormatTypeidName( typeid(*this).name() );
            m_debug->Add(pRef, this);
        }
    }      

    void SafeRefServer::DecSafeRef(BaseSafeRef *pRef) const  
    { 
        m_ref.Dec(); 

        if (CfgSafeRef::CEnableDebug) m_debug->Remove(pRef);
    }

    bool SafeRefDebugModeEnabled()
    {
        return CfgSafeRef::CEnableDebug;
    }

    void SafeRefServer::Init()
    {
        if (CfgSafeRef::CEnableDebug)
        {
            m_debug.reset( new SafeRefServerDebug() );
        }
    }
	
}  // namespace Utils


