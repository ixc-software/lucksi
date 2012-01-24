#ifndef __NOBJSECROOT__
#define __NOBJSECROOT__

#include "NObjSecProfile.h"
#include "NObjSecUser.h"
#include "SecUtils.h"

namespace DriSec
{

    // special class for geting all methods of NamedObject
    class NObjSecSpecial : public Domain::NamedObject, public DRI::INonCreatable
    {
        Q_OBJECT;

    public:

        NObjSecSpecial(Domain::IDomain *pDomain, Domain::ObjectName fullName) : 
          Domain::NamedObject(pDomain, fullName)
        {
        }

    };

    // ----------------------------------------------------

    class NObjSecProfiles : public NObjProfilesBase
    {
        Q_OBJECT;

        ISecContext &m_context;

        NObjSecProfile& CastToProfile(Domain::NamedObject *p)
        {
            ESS_ASSERT(p != 0);

            NObjSecProfile *pProfile = dynamic_cast<NObjSecProfile*>(p);
            ESS_ASSERT(pProfile != 0);
            return *pProfile;
        }

    public:

        NObjSecProfiles(NamedObject *pParent, ISecContext &context) : 
          NObjProfilesBase(pParent),
          m_context(context)
        {
        }

        Q_INVOKABLE void Add(QString name, 
            Mode rootMode, QString allowList = "", QString denyList = "", bool blockSecurity = true)
        {
            if (FindObject(name) != 0) ThrowRuntimeException("Dublicate name!");
            new NObjSecProfile(this, m_context, name, rootMode, allowList, denyList, blockSecurity);
        }

        NObjSecProfile* Find(QString name)
        {
            Domain::NamedObject *p = FindObject(name);
            if (p == 0) return 0;

            return &CastToProfile(p);
        }

    };

    // ----------------------------------------------------


    class NObjSecUsers : public Domain::NamedObject, public DRI::INonCreatable
    {
        Q_OBJECT;

        ISecContext &m_context;

    public:
        
        NObjSecUsers(NamedObject *pParent, ISecContext &context) : 
          Domain::NamedObject(&pParent->getDomain(), "Users", pParent),
          m_context(context)
        {
        }

        Q_INVOKABLE void Add(QString login, QString password, QString profile, int loginLimit = -1, QString desc = "")
        {
            if ( Find(login) ) ThrowRuntimeException("Dublicate user!");

            if ( m_context.FindProfile(profile) == 0) 
            {
                ThrowRuntimeException("Can't find profile!");
            }

            new NObjSecUser(this, m_context, login, password, profile, loginLimit, desc);
        }

        NObjSecUser* Find(QString name) const
        {
            NamedObject *p = FindObject(name);
            if (p == 0) return 0;

            NObjSecUser *pUser = dynamic_cast<NObjSecUser*>(p);
            ESS_ASSERT(pUser != 0);
            return pUser;
        }

        QStringList FindAllUsers(QString profileName) const
        {
            QStringList res;

            {
                QStringList children = GetAllChildrenNames();

                for(int i = 0; i < children.size(); ++i)
                {
                    NObjSecUser *pUser = Find( children.at(i) );
                    ESS_ASSERT(pUser != 0);
                    if (pUser->Profile().Name().GetShortName() == profileName)
                    {
                        res.append( children.at(i) );
                    }
                }
            }

            return res;
        }

    };


    // ----------------------------------------------------

    class NObjSecRoot : 
        public Domain::NamedObject, 
        public DRI::INonCreatable,
        public ISecContext
    {
        Q_OBJECT;

        NObjSecProfiles *m_pProfiles;  // child 
        NObjSecUsers    *m_pUsers;     // child 

        QStringList m_defaultAllowList;

        QStringList CreateDefaultAllowList();

        int LoggedCount(QString login);

    // ISecContext impl
    private:

        NObjSecProfile* FindProfile(QString name)
        {
            ESS_ASSERT(m_pProfiles);
            return m_pProfiles->Find(name);
        }

        void CheckCanDeleteProfile(QString name)
        {
            QStringList sl = m_pUsers->FindAllUsers(name);
            if (sl.empty()) return;

            QString msg = QString("Can't delete profile %1, it used by users %2").arg(name).arg(sl.join(", "));
            ThrowRuntimeException( msg.toStdString() );
        }

        void CheckCanDeleteUser(QString login)
        {
            int count = LoggedCount(login);
            if (count == 0) return;

            QString msg = QString("Can't delete user %1, it used %2 times").arg(login).arg(count);
            ThrowRuntimeException( msg.toStdString() );
        }

        QStringList GetDefaultAllowList() const 
        {
            return m_defaultAllowList;
        }
        
    public:

        NObjSecRoot(NamedObject *pParent) :
          Domain::NamedObject(&pParent->getDomain(), "Security", pParent),
          m_pProfiles( new NObjSecProfiles(this, *this) ),
          m_pUsers( new NObjSecUsers(this, *this) ),
          m_defaultAllowList( CreateDefaultAllowList() )
        {
            // register root
            m_pProfiles->Add( Const::RootProfile(), NObjProfilesBase::acFull, "", "", false );
            m_pUsers->Add( Const::RootLogin(), "", Const::RootProfile(), -1, "super-user");
        }

        NObjSecUser* FindUser(QString name)
        {
            return m_pUsers->Find(name);
        }

        Q_INVOKABLE void SwitchSecProfile(DRI::ISessionCmdContext *pContext, QString profileName) const
        {
            pContext->SwitchSecProfile(profileName);
        }
                
    };
    
    
}  // namespace DriSec

#endif