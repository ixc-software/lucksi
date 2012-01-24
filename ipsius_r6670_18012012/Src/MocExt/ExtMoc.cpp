
#include <string>

#include "moc.h"
#include "ExtMoc.h"
#include "ExtProxyList.h"
#include "ExtNamedObjectList.h"
#include "ExtOptions.h"
#include "IMocExtension.h"
#include "ExtQObject.h"

namespace 
{
    MocExt::ExtentionMoc *GExtentionMoc = 0;
}

namespace MocExt
{

    class ExtentionMoc::Impl 
    {
        Moc &m_moc;

        /*
        InterfacesProxyList interfacesProxyList;
        NamedObjectList m_namedObject; */

        QList<IMocExtension*> m_ext;

    public:

		Impl(Moc &moc) : m_moc(moc)
        {
            m_ext.push_back( new NamedObjectList() );
            m_ext.push_back( new InterfacesProxyList() );
            m_ext.push_back( new ExtQObject() );
        }

        ~Impl()
        {
            for(int i = 0; i < m_ext.size(); ++i)
            {
                delete m_ext.at(i);
            }
        }

        void Append(const ClassDef &class_def, QString classNamespaces)
        {
            for(int i = 0; i < m_ext.size(); ++i)
            {
                std::string warn;
                QList<QByteArray> includes;
                m_ext.at(i)->ProcessClass(class_def, classNamespaces, warn, includes);

                if (!warn.empty()) warning( warn.c_str() );
                
                for(int j = 0; j < includes.size(); ++j)
                {
                    m_moc.includeFiles.push_back( includes.at(j) );
                }
            }
                        
        }

        void Generate(FILE *out) const
        {
            for(int i = 0; i < m_ext.size(); ++i)
            {
                m_ext.at(i)->GenerateFile(out);
            }
        }

        /*
        void Append(const ClassDef &class_def, QString classNamespaces)
        {   
			int prevNamedListSize = m_namedObject.Size();
			std::string res = m_namedObject.Append(class_def, classNamespaces);
            if(res.size() != 0) warning(res.c_str());

			if(prevNamedListSize == 0 && m_namedObject.Size() == 1 )
            {
                m_moc->includeFiles.push_back("DRI/NamedObjectFactory.h");
            }
            interfacesProxyList.Append(class_def);
        }
        
        void Generate(FILE *out) const
        {
            m_namedObject.Generate(out);
            interfacesProxyList.Generate(out);
        } */
        
        void warning(const char *msg)
        {
            m_moc.warning(msg);
        }
        
        void error(const char *msg)
        {
            m_moc.error(msg);
        }

    };

    //-----------------------------------------------------

    ExtentionMoc::ExtentionMoc(Moc &moc)
    {
        m_impl = new Impl(moc); 
        GExtentionMoc = this;
    }

    //-----------------------------------------------------
    
    ExtentionMoc::~ExtentionMoc()
    {
        GExtentionMoc = 0;
        delete m_impl;
        m_impl = 0;
    }

    //-----------------------------------------------------

    void ExtentionMoc::Append(const ClassDef &class_def, QString classNamespaces)
    {   
        if (!ExtOptionsEnabled()) return;
        
		m_impl->Append(class_def, classNamespaces);
    }

    //-----------------------------------------------------

    void ExtentionMoc::Generate(FILE *out) const
    {
        if (!ExtOptionsEnabled()) return;
        
        m_impl->Generate(out);
    }

    //-----------------------------------------------------

    void ExtentionMoc::warningImpl(const char *msg)
    {
        m_impl->warning(msg);
    }

    //-----------------------------------------------------

    void ExtentionMoc::errorImpl(const char *msg)
    {
        m_impl->error(msg);
    }

    //-----------------------------------------------------

    void ExtentionMoc::warning(const char *msg)
    {
		if(GExtentionMoc != 0) GExtentionMoc->warningImpl(msg);
    }

    //-----------------------------------------------------

    void ExtentionMoc::error(const char *msg)
    {
		if(GExtentionMoc != 0) GExtentionMoc->errorImpl(msg);
    }

    //-----------------------------------------------------

} // namespace MocExt

