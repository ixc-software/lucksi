#ifndef __NOBJSECPROFILE__
#define __NOBJSECPROFILE__

#include "Utils/ErrorsSubsystem.h"
#include "Utils/FullEnumResolver.h"

#include "DRI/driutils.h"
#include "Domain/NamedObject.h"

#include "SecUtils.h"

namespace DriSec
{

    ESS_TYPEDEF(SecException);

    // -----------------------------------------------------

    class ProfileAttrList
    {
        struct Pair
        {
            QString Attr, Type;
            bool Plused;

            Pair(QString attr, QString type, bool plused) : 
                Attr(attr), Type(type), Plused(plused)
            { 
                ESS_ASSERT( Attr.length() );
            }

            QString ToString() const
            {
                QString res = Attr;

                if (Type.length()) res = Type + "." + res;
                if (Plused) res += "+";

                return res;
            }

            // for empty Type, equality of attr is enough 
            bool Equal(const QString &attr, const QString &type) const
            {
                ESS_ASSERT( attr.length() );
                ESS_ASSERT( type.length() );

                if (attr != Attr) return false;

                if (Type.length())
                {
                    if (type != Type) return false;
                }

                return true;
            }
        };

        std::vector<Pair> m_list;
        int m_firstSysItem;

    public:

        ProfileAttrList(const QString &list, const QStringList &sysList = QStringList())
        {
            QStringList sl = list.split( QRegExp("( |,|;)+"), QString::SkipEmptyParts );
            m_firstSysItem = sl.size();
            sl.append(sysList);

            for(int i = 0; i < sl.size(); ++i)
            {
                QString type = "";
                QString attr = sl.at(i);
                bool plused = false;

                int n = attr.indexOf(".");

                if (n >= 0)
                {
                    type = attr.left(n);
                    attr = attr.mid(n + 1);
                }

                if ( attr.endsWith("+") )
                {
                    plused = true;
                    attr = attr.left(attr.length() - 1);
                }

                m_list.push_back( Pair(attr, type, plused) );
            }
        }

        bool Empty() const
        {
            return m_list.empty();
        }

        QString ToString(bool withSysItems) const
        {
            QStringList sl;

            int size = withSysItems ? m_list.size() : m_firstSysItem;

            for(int i = 0; i < size; ++i)
            {
                sl.push_back( m_list.at(i).ToString() );
            }

            return sl.join("; ");
        }

        bool Inside(const QString &attr, const QString &type, /* out */ bool &plused)
        {
            for(int i = 0; i < m_list.size(); ++i)
            {
                if ( m_list.at(i).Equal(attr, type) ) 
                {
                    plused = m_list.at(i).Plused;
                    return true;
                }
            }

            return false;
        }
    };

    // -----------------------------------------------------

    class ProfileRecord
    {
        // all type fields are const 
        Domain::ObjectName m_path; 
        bool m_recursive; 
        NObjProfilesBase::Mode m_mode;
        QString m_modeAsStr;   // cached
        ProfileAttrList m_allowList, m_denyList; 

        static int CompareNames(const Domain::ObjectName &name0, 
            const Domain::ObjectName &name1)
        {
            QStringList list0 = name0.ToStringList();
            QStringList list1 = name1.ToStringList();

            int size = std::min(list0.size(), list1.size());

            for(int i = 0; i < size; ++i)
            {
                if (list0.at(i) != list1.at(i)) return i;
            }

            return size;
        }
       
        static QString ActionInfo(const Domain::NamedObject *pObj, const QString &attrName, 
            ISecurity::ActionKind kind, int recIndex)
        {
            ESS_ASSERT(pObj != 0);

            QString s = pObj->Name().Name(true);
            s += " action " + ISecurity::ActionKindToStr(kind);
            s += " for '" + attrName + "'";
            s += " by rec #" + QString::number(recIndex);

            return s;
        }

        static QString FullActionInfo(const QString &action, bool allow, const QString &reason = "")
        {
            QString s;

            s = action;
            s += " // ";
            s += (allow ? "ALLOW" : "DENY");

            if ( reason.length() ) s += " " + reason;

            return s;
        }

        static bool ActionByModeAllowed(NObjProfilesBase::Mode mode, ISecurity::ActionKind kind)
        {
            if (mode == NObjProfilesBase::acDeny) return false;

            if (mode == NObjProfilesBase::acReadOnly)
            {
                return (kind == ISecurity::akPropRead) ? true : false;
            }

            ESS_ASSERT( mode == NObjProfilesBase::acFull );
            return true;
        }

        static QString ListToStr(const ProfileAttrList &list, QString prefix, bool withSysItems)
        {
            QString val = list.ToString(withSysItems);
            if (val.isEmpty()) return "";
            return " " + prefix + ": " + val;
        }
            
    public:

        ProfileRecord(QString path, bool recursive, NObjProfilesBase::Mode mode,
            QString allowList, QString denyList, const QStringList &sysAllowItems) :
            m_path(path),
            m_recursive(recursive),
            m_mode(mode),
            m_modeAsStr( Utils::EnumResolve(mode).c_str() ),
            m_allowList(allowList, sysAllowItems),
            m_denyList(denyList)
        {
        }

        bool IsRoot() const
        {
            return m_path.IsRoot() && m_recursive;
        }

        QString ToString(bool withSysItems) const
        {
            QString s;

            s += (m_path.ObjectLevel() == 0) ? "[root]" : m_path.Name();
            if (m_recursive) s += "+";

            s += " " + QString(Utils::EnumResolve(m_mode).c_str());

            s += ListToStr(m_allowList, "a", withSysItems);
            s += ListToStr(m_denyList,  "d", withSysItems);

            return s;
        }

        bool Simular(const ProfileRecord &another) const
        {
            return (m_path == another.m_path);
        }

        /*
            Возвращает m_path.ObjectLevel() + 1 если
                -- запись нерекурсивна и (name == m_path)
                -- запись рекурсивна и (name.left(x) == m_path)
        */
        int NameCompare(const Domain::ObjectName &name, bool &exactMatch) const 
        {
            exactMatch = false;

            int pathLevel = m_path.ObjectLevel();

            // not recursive
            if (!m_recursive)
            {
                if (m_path.Compare(name) == 0) 
                {
                    exactMatch = true;
                    return pathLevel + 1;
                }
            }
            else
            {
                // recursive
                int result = CompareNames(m_path, name);
                ESS_ASSERT(result >= 1);  // 'couse first root object in all names 

                if (result == pathLevel + 1)
                {
                    exactMatch = (pathLevel == name.ObjectLevel());
                    return result;
                }
            }

            return 0;
        }

        bool VerifyAction(const Domain::NamedObject *pObj, const QString &attrName, 
            ISecurity::ActionKind kind, 
            /* for info */ int recIndex, /* out */ QString &info)
        {
            ESS_ASSERT(pObj != 0);
            info.clear();
            QString typeName = 
                DRI::DriUtils::RemoveNamespaces( pObj->metaObject()->className() ).c_str();

            QString actionInfo = ActionInfo(pObj, attrName, kind, recIndex);

            // attr in deny
            {
                bool plused;
                if ( m_denyList.Inside(attrName, typeName, plused) )
                {
                    // property read blocked by deny list only if attr is plused
                    bool not_blocked = (kind == ISecurity::akPropRead) && !plused;

                    if (!not_blocked)
                    {
                        info = FullActionInfo(actionInfo, false, "by deny list");
                        return false;
                    }
                }
            }

            // attr in allow
            {
                bool plused;
                if ( m_allowList.Inside(attrName, typeName, plused) )
                {
                    // property write allowed only if attr is plused 
                    bool blocked = (kind == ISecurity::akPropWrite) && !plused;

                    if (!blocked)
                    {
                        info = FullActionInfo(actionInfo, true, "by allow list");
                        return true;
                    }
                }
            }

            // default 
            if ( ActionByModeAllowed(m_mode, kind) )
            {
                info = FullActionInfo(actionInfo, true, "by mode " + m_modeAsStr);
                return true;                
            }

            info = FullActionInfo(actionInfo, false, "by mode " + m_modeAsStr);
            return false;
        }

    };

    // -----------------------------------------------------

    // TODO: allow/deny IP list 
    class NObjSecProfile : 
        public Domain::NamedObject, 
        public DRI::INonCreatable,
        public ISecurity
    {
        Q_OBJECT;

        Utils::SafeRefServer m_secRefServer;

        ISecContext &m_context;
        const QString m_name;

        std::vector<ProfileRecord> m_records;

        void ThrowIfRoot(QString msg)
        {
            if ( m_name != Const::RootProfile() ) return;
            ThrowRuntimeException(msg);
        }

        void TryToModify()
        {
            ThrowIfRoot("Can't modify ROOT profile!");

            if (m_secRefServer.getCountRef() != 0)
            {
                ThrowRuntimeException("Can't modify in-use profile!");
            }
        }

        QStringList RecList(bool withSysItems) const
        {
            QStringList sl;

            for(int i = 0; i < m_records.size(); ++i)
            {
                sl.push_back( QString("#%1 - %2").arg(i).arg(m_records.at(i).ToString(withSysItems)) );
            }

            return sl;
        }

        void Add(const ProfileRecord &rec, bool checkAllowModify)
        {
            if (checkAllowModify) TryToModify();

            for(int i = 0; i < m_records.size(); ++i)
            {
                if ( m_records.at(i).Simular(rec) )
                {
                    ESS_THROW_MSG(SecException, "Dublicate record!");
                }
            }

            m_records.push_back(rec);
        }


        int FindRecord(const Domain::ObjectName &name)
        {
            int recIndex = -1;
            int bestCmpValue = 0;

            for(int i = 0; i < m_records.size(); ++i)
            {
                bool exactMatch;
                int cmp = m_records.at(i).NameCompare(name, exactMatch);
                if (i == 0) ESS_ASSERT(cmp == 1);

                if (cmp > bestCmpValue)
                {
                    recIndex = i;
                    bestCmpValue = cmp;
                    if (exactMatch) break;
                }
            }

            ESS_ASSERT(recIndex >= 0);
            ESS_ASSERT(bestCmpValue > 0);

            return recIndex;
        }

        // QString ExtAllowList(const QString &allowList);

    // ISecurity impl
    private:

        bool VerifyAction(const Domain::NamedObject *pObj, const QString &attrName, 
            ActionKind kind, /* out */ QString &info)
        {
            ESS_ASSERT(pObj != 0);
            info.clear();

            int recIndx = FindRecord( pObj->Name() );
            return m_records.at(recIndx).VerifyAction(pObj, attrName, kind, recIndx, info);
        }


    public:

        NObjSecProfile(Domain::NamedObject *pParent, 
                       ISecContext &context,
                       QString name, 
                       NObjProfilesBase::Mode mode, QString allowList, QString denyList, bool blockSecurity) :
            Domain::NamedObject(&pParent->getDomain(), name, pParent),
            m_context(context),
            m_name(name)
        {   
            ProfileRecord rootRec("", true, mode, allowList, denyList, m_context.GetDefaultAllowList());

            if (!rootRec.IsRoot()) ESS_THROW_MSG(SecException, "Bad root record!");

            Add(rootRec, false); // m_records.push_back(rootRec);

            if (blockSecurity)
            {
                ProfileRecord rec("Security", true, NObjProfilesBase::acDeny, "", "", m_context.GetDefaultAllowList());
                Add(rec, false);
            }
        }

        ~NObjSecProfile();

        Q_INVOKABLE void List(DRI::ICmdOutput *pContext, bool withSysItems = false) const
        {
            pContext->Add( RecList(withSysItems) );
        }

        Q_INVOKABLE void AddRec(QString path, bool recursive, NObjProfilesBase::Mode mode,
            QString allowList = "", QString denyList = "")
        {
            ProfileRecord rec( path, recursive, mode, allowList, denyList, m_context.GetDefaultAllowList() );
            Add(rec, true);
        }

        Q_INVOKABLE void DeleteRec(int index)
        {
            TryToModify();

            if ((index <= 0) || (index >= m_records.size()))
            {
                ESS_THROW_MSG(SecException, "Bad index!");
            }

            m_records.erase(m_records.begin() + index);
        }

        Q_INVOKABLE void Delete()
        {
            ThrowIfRoot("Can't delete ROOT profile!");
            m_context.CheckCanDeleteProfile(m_name);

            AsyncDeleteSelf();
        }

        bool VerifyIP(const Utils::HostInf &remoteAddr)
        {
            if (remoteAddr.Empty()) return true;

            // TODO: unimplemented
            // ... 

            return true;
        }

        Utils::SafeRef<ISecurity> GetSecurity()
        {
            return Utils::SafeRef<ISecurity>(&m_secRefServer, this);
        }

    };
    
    
}  // namespace DriSec


#endif
