#ifndef __NOBJSECUSER__
#define __NOBJSECUSER__

#include "SecUtils.h"

namespace DriSec
{
    class NObjSecProfile;
    
    class NObjSecUser : public Domain::NamedObject, public DRI::INonCreatable
    {
        Q_OBJECT;

        ISecContext &m_context;

        QString m_login; 
        QString m_desc; 
        QString m_password; 
        QString m_profile; 
        int m_loginLimit;

        QString LoginLimitToStr() const
        {
            if (m_loginLimit == 0) return "login disabled";
            if (m_loginLimit < 0)  return "login unlimited";
            if (m_loginLimit == 1) return "login only for one user";

            return "login limit " + QString::number(m_loginLimit);
        }

        QString GetInfo() const
        {
            QString s;

            s += m_login;
            if (m_desc.length()) s += " [" + m_desc + "]";
            s += " password length " + QString::number(m_password.length());
            s += "; profile " + m_profile;
            s += "; " + LoginLimitToStr();

            return s;
        }

    public:
        
        NObjSecUser(NamedObject *pParent, ISecContext &context,
            QString login, QString password, QString profile, int loginLimit, QString desc) :
            Domain::NamedObject(&pParent->getDomain(), login, pParent),
            m_context(context),
            m_login(login),
            m_desc(desc),
            m_password(password),
            m_profile(profile),
            m_loginLimit(loginLimit)
        {
        }

        Q_PROPERTY(QString Info READ GetInfo);

        Q_INVOKABLE void Delete()
        {
            if ( m_login == Const::RootLogin() ) ThrowRuntimeException("Can't delete ROOT!");
            m_context.CheckCanDeleteUser(m_login);

            AsyncDeleteSelf();
        }

        Q_INVOKABLE void SetLoginLimit(int value)
        {
            m_loginLimit = value;
        }

        Q_INVOKABLE void SetPassword(QString value)
        {
            m_password = value;
        }

        bool VerifyPassword(QString password)
        {
            return (m_password == password);
        }

        NObjSecProfile& Profile() const
        {
            NObjSecProfile *p = m_context.FindProfile(m_profile);
            if (p == 0)
            {
                ThrowRuntimeException("Can't find profile " + m_profile + " for user " + m_login);
            }

            return *p;
        }

        bool VerifyLimit(int currUsers)
        {
            if (m_loginLimit < 0) return true;  // unlimited

            return (currUsers < m_loginLimit);
        }

    };
    
    
}  // namespace DriSec

#endif