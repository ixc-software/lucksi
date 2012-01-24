#ifndef __SECUTILS__
#define __SECUTILS__

#include "Utils/IBasicInterface.h"

#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "Domain/IDomain.h"

namespace DriSec
{
    class NObjSecProfile;

    class ISecContext : public Utils::IBasicInterface
    {
    public:

        virtual NObjSecProfile* FindProfile(QString name) = 0;

        virtual void CheckCanDeleteProfile(QString name) = 0;  // can't if profile references by user; throw if can't 
        virtual void CheckCanDeleteUser(QString login) = 0;    // can't if it uses by logged user; throw if can't 

        virtual QStringList GetDefaultAllowList() const = 0;
        
    };

    // --------------------------------------------------

    class NObjProfilesBase : public Domain::NamedObject, public DRI::INonCreatable
    {
        Q_OBJECT;

    public:

        enum Mode
        {
            acDeny,
            acReadOnly,
            acFull,
        };

        Q_ENUMS(Mode);

        NObjProfilesBase(NamedObject *pParent) : 
            Domain::NamedObject(&pParent->getDomain(), "Profiles", pParent)
        {
        }

    };

    // --------------------------------------------------

    struct Const
    {
        static QString RootLogin() 
        {
            return "root";
        }

        static QString RootProfile() 
        {
            return RootLogin();
        }
    };

    // --------------------------------------------------

    class ISecurity : public Utils::IBasicInterface
    {
    public:

        enum ActionKind
        {
            akExecute,
            akPropRead,
            akPropWrite,
        };

        static QString ActionKindToStr(ActionKind kind)
        {
            if (kind == akExecute)   return "Execute";
            if (kind == akPropRead)  return "PropRead";
            if (kind == akPropWrite) return "PropWrite";
            
            ESS_HALT( "Bad ActionKind " + Utils::IntToString(kind) );
            return "(none)";
        }

        virtual bool VerifyAction(const Domain::NamedObject *pObj, const QString &attrName, 
            ActionKind kind, /* out */ QString &info) = 0;


    };
    
}  // namespace DriSec

#endif