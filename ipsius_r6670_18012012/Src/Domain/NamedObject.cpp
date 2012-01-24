#include "stdafx.h"

#include "Utils/QtEnumResolver.h"

#include "DriSec/SecUtils.h"

#include "DRI/NamedObjectFactory.h"
#include "DRI/commanddriresult.h"
#include "DRI/driutils.h"
#include "DRI/DriArgFactory.h"
#include "DRI/ExecutiveDRI.h"
#include "DRI/DriPropertyFactory.h"

#include "iLog/LogManager.h"
#include "NamedObject.h"
#include "DomainClass.h"
#include "NamedObject_helpers.h"

namespace
{
    const char* CRootName = "ROOT";
}  // namespace

// ---------------------------------------------------

namespace Domain
{

    NamedObject::NamedObject(IDomain *pDomain, const ObjectName &name, NamedObject *pParent, bool isRoot) : 
        iCore::MsgObject(pDomain->getDomain().getMsgThread()), 
        m_domain(pDomain->getDomain()), 
        m_name(name), 
        m_nameShort(name), 
        m_pParent(pParent), 
        /*m_pAsync(0),*/
        m_root(isRoot),
        m_allowUserChildrenAdd(false),
        m_exceptionHookEnabled(false)
    {
        m_pWaitChildForDelete = 0;

        // 1-й вариант, root
        if (m_root)
        {
            ESS_ASSERT(m_name.IsRoot());
            ESS_ASSERT(m_pParent == 0);
            return;
        }

        // 2-й вариант
        if (m_pParent == 0)
        {
            m_pParent = (name.ObjectLevel() == 1) ? 
                m_domain.ObjectRoot() : m_domain.FindFromRoot(m_name.Parent());
            
			ESS_ASSERT(m_pParent != 0);
        }
        else  // 3-й вариант
        {
            ESS_ASSERT(m_name.IsShort());

            m_name = m_name.AddParent(m_pParent->Name());
        }

        // set short name
        m_nameShort = m_name.GetShortName();

        // add self to parent
        m_pParent->AddChild(this);

        // notify -- tree changes
        m_domain.ExeDRI().ObjectTreeChanged(false);

		if(m_pParent->LogLevel() != 0) InitLog();
    }

    // -----------------------------------------------

    NamedObject::~NamedObject()
    {
        if (m_exceptionHookEnabled) 
        {
            m_domain.UnregisterExceptionHook(this);
            m_exceptionHookEnabled = false;
        }

        if (m_pParent) m_pParent->RemoveChild(this);

        FreeChildren();

        // notify -- tree changes
        if (!m_root)
        {
            m_domain.ExeDRI().ObjectTreeChanged(true);
            m_domain.OnNamedObjectDestroyed(*this);
        }        
    }

    // -----------------------------------------------

    void NamedObject::AddChild(NamedObject *pChild)
    {
        ESS_ASSERT(pChild);

        // verify child name
        if (IsRoot())
        {
            ESS_ASSERT(pChild->Name().ObjectLevel() == 1);
        }
        else
        {
            ObjectName childParent = pChild->Name().Parent();
            ESS_ASSERT(m_name == childParent);
        }

        // check for dublicate object        
        ESS_ASSERT(std::find(m_children.begin(), m_children.end(), pChild) == m_children.end());

        // check for dublicate name
        if (FindChildByName(pChild->Name()) != 0) 
        {            
            QString childName = pChild->Name().Name();
            ThrowRuntimeException( QString("Dublicate child %1").arg(childName) );
        }

        // add
        m_children.push_back(pChild);
    }

    // -----------------------------------------------

    void NamedObject::RemoveChild(NamedObject *pChild)
    {
        if (m_pWaitChildForDelete != 0)
        {
            ESS_ASSERT(m_pWaitChildForDelete == pChild);
            m_pWaitChildForDelete = 0;
            return;
        }

        ChildrenList::iterator i = std::find(m_children.begin(), m_children.end(), pChild);
        ESS_ASSERT(i != m_children.end());
        m_children.erase(i);
    }


    // -----------------------------------------------

    NamedObject* NamedObject::FindObject(const ObjectName &name) const
    {    
        if (name.IsEmpty())
        {
            return const_cast<NamedObject*>(this);
        }

        ObjectName head = name.GetHeadName();  

        NamedObject *pChild = FindChildByName(head);
        if (pChild == 0) return 0;  // not found

        if (name.ObjectLevel() == 1) return pChild;

        // recursive
        return pChild->FindObject(name.GetBeyondHead());
    }

    NamedObject* NamedObject::FindObject( const QString &name ) const
    {
        if (!Identificator::CheckObjectName(name)) return 0;
        return FindObject( ObjectName(name) );
    }

    // -----------------------------------------------

    NamedObject* NamedObject::FindChildByName(const ObjectName &name) const
    {
        ObjectName shortName = name.GetShortName();
        ChildrenList::const_iterator i = m_children.begin();

        while(i != m_children.end())
        {
            if ((*i)->NameShort() == shortName) return (*i);

            // next
            ++i;
        }

        return 0;
    }

    // -----------------------------------------------

    void NamedObject::FreeChildren(int remainsCount)
    {
        ESS_ASSERT(remainsCount >= 0);

        while(m_children.size() > remainsCount)
        {
            // delete last child
            m_pWaitChildForDelete = m_children.back();
            QString name = m_pWaitChildForDelete->NameShort().Name();
            m_pWaitChildForDelete->Delete();
            ESS_ASSERT(m_pWaitChildForDelete == 0);  // verify: callback is OK

            // remove it from list
            m_children.pop_back();
        }
    }
	
    // -----------------------------------------------

    /*
    NamedObject* NamedObject::Root()
    {
        return m_domain.ObjectRoot();
    } */


    // -----------------------------------------------

    FullObjectName NamedObject::GetFullObjectName() const
    {
        // return m_name.getFullName();
        return FullObjectName(m_domain.getName(), m_name);
    }

    // -----------------------------------------------

    // tryTreatAsProperty means what path must be exists
    QString NamedObject::PathToValue(const QString &pathIn, bool tryTreatAsProperty) const
    {
        QString path = pathIn;
        path = m_name.AddChild( path );

        /*
        if (path.isEmpty()) return path;

        // path is relative
        if (path.startsWith(DRI::CPathSep))
        {
            path = m_name.AddChild( path.mid(1) );
        } */

        if (!tryTreatAsProperty) return path;  // don't check result

        // verify path must be exists
        {
            NamedObject *pObj = m_domain.FindFromRoot(path);
            if (pObj != 0) return path;
        }

        // last chance -- try treat path as property
        {
            int pos = path.lastIndexOf(DRI::CPathSep);
            if (pos < 0) ThrowRuntimeException("Bad path!");

            QString propName = path.mid(pos + 1);
            path = path.left(pos);

            NamedObject *pObj = m_domain.FindFromRoot(path);
            if (pObj == 0) ThrowRuntimeException("Bad path / try as property!");

            QString propValue;
            if ( !ReadRawStringProperty(pObj, propName, propValue) )
            {
                ThrowRuntimeException("Bad path / no property!");
            }

            return propValue;
        }

    }

    // -----------------------------------------------

    void NamedObject::CS_Set( DRI::ISessionCmdContext *pContext, QString path ) const
    {
        path = PathToValue(path, true); // path.isEmpty() ? m_name.Name() : PathToValue(path, true);
        pContext->SetCurrentObject(path);  // m_name.AddChild(name)
    }

    void NamedObject::CS_SetBack( DRI::ISessionCmdContext *pContext ) const
    {
        pContext->SetCurrentObjectBack();
    }

    void NamedObject::CS_Begin( DRI::ISessionCmdContext *pContext, bool continueOnError ) const
    {
        pContext->TransactionBegin(continueOnError);
    }

    void NamedObject::CS_Print( DRI::ISessionCmdContext *pContext, QString text ) const
    {
        pContext->OutputPrint(text);
    }

    void NamedObject::CS_Exec( DRI::ISessionCmdContext *pContext, QString fileName, QString macroReplaceList ) const
    {
        pContext->ExecuteFile(fileName, macroReplaceList);
    }

    void NamedObject::CS_Timeout( DRI::ISessionCmdContext *pContext, int intervalMs ) const
    {
        pContext->SetCommandTimeout(intervalMs);
    }

    void NamedObject::CS_Discard( DRI::ISessionCmdContext *pContext ) const
    {
        pContext->TransactionDiscard();
    }

    void NamedObject::CS_End( DRI::ISessionCmdContext *pContext ) const
    {
        pContext->TransactionEnd();
    }

    void NamedObject::CS_Exit( DRI::ISessionCmdContext *pContext ) const
    {
        pContext->SessionExitRequest();
    }

    // -----------------------------------------------

    void NamedObject::CS_Info(DRI::ISessionCmdContext *pContext) const
    {
        pContext->PrintSessionInfo();
    }

    // -----------------------------------------------

    /*
    void NamedObject::CS_SwitchSecProfile(DRI::ISessionCmdContext *pContext, QString profileName) const
    {
        pContext->SwitchSecProfile(profileName);
    } */

    // -----------------------------------------------

    DRI::INamedObjectType* NamedObject::FindNamedObjectType( QString typeName, bool throwException ) const
    {
        DRI::INamedObjectType *pType = 
            DRI::NamedObjectFactory::Instance().Find( typeName.toStdString() );

        // type not found in factory
        if ((pType == 0) && throwException)
        {
            ThrowRuntimeException( QString("Type %1 not found in factory").arg(typeName) );
        }

        return pType;
    }

    // -----------------------------------------------

    void NamedObject::ParentForChildCreationCheck(QString fullChildName, DRI::INamedObjectType *pType)
    {
        QString parentName = ObjectName(fullChildName).Parent(); // .getName().c_str();
        NamedObject *pParent = m_domain.FindFromRoot(parentName);

        /*
        NamedObject *pRoot = m_domain.ObjectRoot();
        NamedObject *pParent = (parentName.isEmpty()) ? 
            pRoot : 
            pRoot->FindObject(parentName); */

        if (pParent == 0) 
        {
            ThrowRuntimeException( QString("Parent '%1' not found").arg(parentName) );
        }

        if (!pParent->AllowCreateChild(pType))
        {
            ThrowRuntimeException( QString("Object %1 not allow children creation").arg(parentName) );
        }

    }

    // -----------------------------------------------

    void NamedObject::ObjCreate( DRI::ISessionCmdContext *pContext, QString objectType, QString objectName, bool setAsCurrent )
    {
        if (objectType.isEmpty()) ThrowRuntimeException("objectType is empty");

        // find type info
        DRI::INamedObjectType *pType = FindNamedObjectType(objectType, true);

        // type non creatable
        if (!pType->AllowCreate())
        {
            ThrowRuntimeException( QString("Type %1 is non-creatable").arg(objectType) );
        }

        // verify objectName -- it must be short 
        if ( !objectName.isEmpty() )
        {
            ObjectName name(objectName);
            if (!name.IsShort()) ThrowRuntimeException("You can use only short object name!");
        }

        // make global name
        QString name = objectName.isEmpty() ? 
            GenerateNewName(objectType) : 
            PathToValue(objectName, false);

        // parent check
        ParentForChildCreationCheck(name, pType);

        // create
        NamedObject *pInst = pType->CreateInstance(&m_domain, name);

        // set current
        if (setAsCurrent)
        {
            pContext->SetCurrentObject(name);
        }
    }

    // -----------------------------------------------

     void NamedObject::ObjDelete(DRI::IAsyncCmd *pAsyncCmd)
    {
        QString fullName = PathToValue("", true); 
        NamedObject *p = m_domain.FindFromRoot(fullName); //  FindObject(objName);

        if (p == 0)
        {
            ThrowRuntimeException( QString("Object %1 not found").arg(fullName) );
        }

        if (p->IsNonCreatable())
        {
            ThrowRuntimeException( QString("Object %1 is non-creatable").arg(fullName) );
        }

        if (p->IsRoot())
        {
            ThrowRuntimeException("Can't delete root!");
        }

        // user code
        OnObjectDelete(pAsyncCmd);

        // is user begin async operation?
        if (/*m_pAsync != 0*/ AsyncActive()) return;  // user call AsyncBegin() inside OnObjectDelete()

        // async delete self
        p->AsyncDeleteSelf();  // sync version: p->Delete();

        // formal: begin and close async command ObjDelete
        // AsyncBegin(pAsyncCmd);
        // AsyncComplete(true);
    }

    // -----------------------------------------------

    /*
        Этот метод имеет смысл, только если будут объекты "контейнеры", т.е.
        пользователь создает объект-папку, а потом уже внутри этого объекта
        создает вложенные объекты нужного типа.

        Сейчас "контейнеры" не работают, т.к. удаление вложенных объектов
        всегда синхронно. Это дает возможность обходить асинхронный
        механизм удаления, который может быть встроен в функцию ObjDelete().

        Для того, чтобы можно было ввести класс "контейнеров" нужно на уровне
        NamedObject встроить схему асинхронного удаления вложенных объектов, и
        ввести правило, что объекты ВСЕГДА удаляются только через нее. 

    */

    /*
    void NamedObject::ObjDeleteChildren()
    {
        // verify -- where is no non-creatable children
        ChildrenList::iterator i = m_children.begin();

        while(i != m_children.end())
        {
            if ((*i)->IsNonCreatable()) 
            {
                ThrowRuntimeException( QString("Object has non-creatable child(s)") );
            }

            ++i;
        }

        // delete children
        FreeChildren();
    } */

    // -----------------------------------------------

    void NamedObject::ThrowRuntimeException( const std::string &s ) const
    {
        std::string msg = s;

        // add current object name
        msg += " @ ";
        msg += Name().Name(true).toStdString();

        ESS_THROW_MSG(DRI::ExecuteException, msg);
    }

    // -----------------------------------------------

    QString NamedObject::GenerateNewName( QString objectType ) const
    {
        int currNumber = 1;  // auto-names begin from 1, yep this not geek feature

        while(true)
        {
            ESS_ASSERT(currNumber < 1024);  // limit

            QString currName = QString("%1%2").arg(objectType).arg(currNumber);
            ObjectName name(currName);

            NamedObject *pChild = FindChildByName(name);
            if (pChild == 0) 
            {
                name = name.AddParent(m_name);
                return name.Name();
            }

            ++currNumber;
        }
    }

    // -----------------------------------------------

    void NamedObject::ObjList(DRI::ICmdOutput *pOutput, bool listProperties, int recurseLevel) const
    {
        ObjListRecursive(pOutput, listProperties, 0, recurseLevel);
    }

    // -----------------------------------------------

    void NamedObject::ObjListRecursive(DRI::ICmdOutput *pOutput, 
        bool listProperties, int currLevel, int maxLevel) const
    {
        // name/type
        pOutput->Add( ObjectNameAndType(currLevel) );

        // check level
        if ((currLevel > maxLevel) && (maxLevel >= 0)) return;

        DriSec::ISecurity *pSec = pOutput->GetSecurity();

        // props
        if (listProperties)
        {
            const QMetaObject *pMeta = MetaObjectForType("", true);
            Utils::StringList sl = 
                ExeDRI().MetaObjInfo().ListProperties(this, pMeta, lpmNameValue, currLevel + 1, false, pSec);
            pOutput->Add(sl);
            // for(int i = 0; i < sl.size(); ++i)  pOutput->Add( sl.at(i) );
        }

        // list children
        ChildrenList::const_iterator i = m_children.begin();

        while(i != m_children.end())
        {
            (*i)->ObjListRecursive(pOutput, listProperties, currLevel + 1, maxLevel);

            ++i;
        }
    }

    // -----------------------------------------------

    QString NamedObject::ObjectNameAndType(int identLevel) const
    {
        std::string typeName;
        typeName = DRI::DriUtils::FormatedTypename(typeid(*this).name(), true);  // DRI::DriUtils::RemoveNamespaces( metaObject()->className() ).c_str();

        QString objName = (m_root) ? CRootName : m_nameShort.Name();        

        QString res;

        res += ::MakeIndent(identLevel);
        res += objName;
        res += " : ";
        res += typeName.c_str(); 

        if (ObjectIsNonCreatable(this))
        {
            res += ' ';
            res += CReadOnlyMarker;
        }

        return res;
    }

    // -----------------------------------------------

    const QMetaObject* NamedObject::MetaObjectForType(QString typeName, bool throwException) const
    {
        if (typeName.isEmpty()) return metaObject();

        DRI::INamedObjectType *pType = FindNamedObjectType(typeName, throwException);
        if (pType == 0) return 0;

        return &pType->MetaObject();
    }

    // -----------------------------------------------

    void NamedObject::PropertyList(DRI::ICmdOutput *pOutput, 
        QString typeName, QString propertyName, ListPropertiesMode mode) const
    {
        Utils::StringList sl;

        DriSec::ISecurity *pSec = pOutput->GetSecurity();
        const QMetaObject *pMeta = MetaObjectForType(typeName, true); 

        if (propertyName.isEmpty())
        {
            sl = ExeDRI().MetaObjInfo().ListProperties(this, pMeta, mode, 0, false, pSec);
        }
        else
        {
            int indx = pMeta->indexOfProperty( propertyName.toStdString().c_str() );
            if (indx >= 0) 
            {
                QString s = 
                    ExeDRI().MetaObjInfo().PropertyToString(this, pMeta->property(indx), mode, true, pSec);
                if (!s.isEmpty()) sl.push_back(s);
            }
            else
            {
                ThrowRuntimeException("Property not found!");
            }
        }

        if (!sl.isEmpty())
        {
            pOutput->Add(sl);
        }
    }

    // -----------------------------------------------

    void NamedObject::ObjView( DRI::ICmdOutput *pOutput, bool withTypes, QString propertyName ) const
    {
        ListPropertiesMode mode = withTypes ? lpmNameTypeValue : lpmNameValue;
        PropertyList(pOutput, "", propertyName, mode);
    }

    // -----------------------------------------------

    void NamedObject::MetaFields( DRI::ICmdOutput *pOutput, QString typeName, QString propertyName ) const
    {
        PropertyList(pOutput, typeName, propertyName, lpmNameType);
    }

    // -----------------------------------------------

    void NamedObject::MetaMethods( DRI::ICmdOutput *pOutput, QString typeName, QString methodName ) const
    {
        Utils::StringList sl;

        const QMetaObject *pMeta = MetaObjectForType(typeName, true);
        
        if (methodName.isEmpty())
        {
            sl = ExeDRI().MetaObjInfo().ListMethods(pMeta, 0, false);
        }
        else
        {
            int indx = FindMethodByName(pMeta, methodName); // pMeta->indexOfMethod(methodName.toStdString().c_str());
            if (indx >= 0)
            {
                QString s = ExeDRI().MetaObjInfo().MethodToString(pMeta, pMeta->method(indx));
                sl.push_back(s);
            }
            else
            {
                ThrowRuntimeException("Method not found!");
            }
        }

        pOutput->Add(sl);
    }

    // -----------------------------------------------

    void NamedObject::MetaEnums( DRI::ICmdOutput *pOutput, 
        QString typeName, QString enumName ) const
    {
        Utils::StringList sl;

        const QMetaObject *pMeta = MetaObjectForType(typeName, true);

        if (enumName.isEmpty())
        {
            sl = ExeDRI().MetaObjInfo().ListEnums(pMeta, 0, false);
        }
        else
        {
            
            int indx = pMeta->indexOfEnumerator(enumName.toStdString().c_str());  // FindMethodByName(pMeta, methodName); 
            if (indx >= 0)
            {
                QString s = ExeDRI().MetaObjInfo().EnumToString(pMeta->enumerator(indx));
                sl.push_back(s);
            } 
            else
            {
                ThrowRuntimeException("Enum not found!");
            }
        }

        pOutput->Add(sl);
    }

    // -----------------------------------------------

    void NamedObject::MetaTypeInfo(DRI::ICmdOutput *pOutput, 
        QString typeName, bool withInherited) const
    {
        const QMetaObject *pMeta = MetaObjectForType(typeName, true);

        bool alwaysWtihInherited = m_domain.getDomain().ExeDRI().SpecClassFactory()
                                        .IsAlwaysShowWithInherited(pMeta->className());
        withInherited = (alwaysWtihInherited)? true : withInherited;
        
        Utils::StringList sl = 
            ExeDRI().MetaObjInfo().ListMetaTypeInfo(pMeta, true, !withInherited);

        pOutput->Add(sl);
    }

    // -----------------------------------------------

    void NamedObject::MetaTypes(DRI::ICmdOutput *pOutput, 
        bool fullInfo, bool withInherited, bool onlyCreatable) const
    {
        Utils::StringList result;

        DRI::NamedObjectFactory& factory = DRI::NamedObjectFactory::Instance();

        for(int i = 0; i < factory.Count(); ++i)
        {
            DRI::INamedObjectType *pType = factory[i];

            if (onlyCreatable && !pType->AllowCreate()) continue;

            Utils::StringList sl = 
                ExeDRI().MetaObjInfo().ListMetaTypeInfo(&pType->MetaObject(), 
                                                        fullInfo, !withInherited);
            result += sl;
        }

        pOutput->Add(result);
    }

    // -----------------------------------------------

    bool NamedObject::ReadProperty( const QString &propertyName, QString &result, bool withMarkRO ) const
    {
        const QMetaObject *pMeta = MetaObjectForType("", true); 

        int indx = pMeta->indexOfProperty( propertyName.toStdString().c_str() );

        if (indx < 0) return false;

        DriSec::ISecurity *pSec = 0;  // don't use security control, it's already done

        result = 
            ExeDRI().MetaObjInfo().PropertyToString(this, pMeta->property(indx), lpmValue, withMarkRO, pSec);

        return true;
    }

    // -----------------------------------------------

    QString NamedObject::Property( QString propertyName, bool withMarkRO ) const
    {        
        BeforeInvoke();  // verify state

        QString result;
        if (!ReadProperty(propertyName, result, withMarkRO)) return "";

        return result;
    }

    // -----------------------------------------------

    /*
    bool NamedObject::Property( QString propertyName, QString val )
    {        
        BeforeInvoke();  // verify state

        // find property
        QMetaProperty prop;
        {
            const QMetaObject *pMeta = MetaObjectForType("", true); 

            int indx = pMeta->indexOfProperty( propertyName.toStdString().c_str() );

            if (indx < 0) return false;
            prop = pMeta->property(indx);
        }

        // writetable 
        if (!prop.isWritable()) return false;

        // enum type check
        if (prop.isEnumType())
        {
            QMetaEnum e = prop.enumerator();

            if (prop.isFlagType())
            {
                ESS_UNIMPLEMENTED;
            }
            else
            {
                int indx = e.keyToValue(val.toStdString().c_str());
                if (indx < 0) return false;
            }
        }

        // set
        bool res = setProperty(propertyName.toStdString().c_str(), QVariant(val) );

        return res;
    } */

    bool NamedObject::Property( QString propertyName, QString val )
    {
        if (!m_onPropertyWrite.IsEmpty())
        {
            if (!m_onPropertyWrite->OnPropertyWrite(this, propertyName, val))
            {
                return false;
            }
        }

        ExeDRI().PropertyFactory().Set(propertyName, this, val);

        return true;
    }

    // -----------------------------------------------

    bool NamedObject::IsNonCreatable()
    {
        DRI::INonCreatable *p = dynamic_cast<DRI::INonCreatable*>(this);
        return (p != 0);
    }

    // -----------------------------------------------

    DRI::DriArgFactory& NamedObject::ArgFactory() const
    {
        return m_domain.ExeDRI().ArgFactory();
    }

    // -----------------------------------------------

    void NamedObject::AsyncBegin( DRI::IAsyncCmd *pCmd, 
		boost::function<void ()> abortFn)
    {
        ESS_ASSERT(!AsyncActive());
        ESS_ASSERT(m_asyncAbort.empty());
        ESS_ASSERT(pCmd != 0);

        Utils::ISafeRefOwner *pOwner = dynamic_cast<Utils::ISafeRefOwner*>(pCmd);
        ESS_ASSERT(pOwner != 0);

        // m_pAsync = pCmd;
        m_pAsync = Utils::SafeRef<DRI::IAsyncCmd>(pOwner, pCmd);
        ESS_ASSERT(AsyncActive());

		m_asyncAbort = abortFn;

        pCmd->AsyncStarted(this);
    }

    // -----------------------------------------------
    
    void NamedObject::AsyncOutput( const QString &line, bool lineFeed) const
    {
        ESS_ASSERT(AsyncActive());

        m_pAsync->Add(line, lineFeed);
    }

    // -----------------------------------------------

    void NamedObject::AsyncComplete( bool ok, const QString &errorMsg /*= ""*/ )
    {
        // ESS_ASSERT(m_pAsync != 0);
        ESS_ASSERT(AsyncActive());

        m_pAsync->AsyncCompleted(ok, errorMsg);

        m_pAsync.Clear();
        m_asyncAbort.clear();
    }

    // -----------------------------------------------

    void NamedObject::AsyncFlush()
    {
        // ESS_ASSERT(m_pAsync != 0);
        ESS_ASSERT(AsyncActive());

        m_pAsync->Flush();
    }

    // -----------------------------------------------

    bool NamedObject::AsyncActive() const 
    { 
        // return (m_pAsync != 0); 
        return !m_pAsync.IsEmpty();
    }
    
    // -----------------------------------------------

    void NamedObject::BeforeInvoke() const
    {
        // ESS_ASSERT(m_pAsync == 0);
        ESS_ASSERT(!AsyncActive());
    }

    // -----------------------------------------------

    void NamedObject::GetAllMetaIdentificators( Utils::StringList &sl ) const
    {
        sl.clear();

        DRI::NamedObjectFactory& factory = DRI::NamedObjectFactory::Instance();

        for(int i = 0; i < factory.Count(); ++i)
        {
            DRI::INamedObjectType *pType = factory[i];

            // just once get inherited for NamedObject stuff
            bool getInherited = (i == 0);

            ExeDRI().MetaObjInfo().AddMetaIdentificators(sl, pType, getInherited);
        }
    }

    // -----------------------------------------------

    void NamedObject::GetAllShortNames( Utils::StringList &sl ) const
    {
        QString name = m_nameShort.Name();
        if (!name.isEmpty()) AddUniqStringToList(sl, name);

        // call recursive for all children
        ChildrenList::const_iterator i = m_children.begin();

        while(i != m_children.end())
        {
            (*i)->GetAllShortNames(sl);

            // next
            ++i;
        }

    }

    // -----------------------------------------------

    QStringList NamedObject::GetAllChildrenNames() const
    {
        QStringList res;

        ChildrenList::const_iterator i = m_children.begin();

        while(i != m_children.end())
        {
            res.append( (*i)->m_nameShort.Name() );
            
            // next
            ++i;
        }

        return res;
    }


    // -----------------------------------------------

    ExecutiveDRI& NamedObject::ExeDRI() const
    {
        return m_domain.ExeDRI();
    }

    // -----------------------------------------------

    void NamedObject::PropertyWriteEvent( Utils::SafeRef<IPropertyWriteEvent> handle )
    {
        m_onPropertyWrite = handle;
    }

    // -----------------------------------------------

    void NamedObject::PropertyWriteEventAbort( QString msg )
    {
        m_propertyWriteAbortMsg = msg;
        PropertyWriteEvent( Utils::SafeRef<IPropertyWriteEvent>(this, this) );
    }

    // -----------------------------------------------

    void NamedObject::PropertyWriteEventClear()
    {
        m_onPropertyWrite.Clear();
    }

    // -----------------------------------------------

    bool NamedObject::OnPropertyWrite( NamedObject *pObject, 
                                       QString propertyName, 
                                       QString val )
    {        
        QString msg = QString("Can't write property: %1").arg(m_propertyWriteAbortMsg);
        ThrowRuntimeException(msg);
        return false;
    }

	// -------------------------------------------------------------------------------
	// log management

	class NamedObject::LogImpl : boost::noncopyable
	{
	public:
		LogImpl(iLogW::LogSession *session) : 
			m_session(session),
			m_infoTag(m_session->RegisterRecordKind(L"Info", false)),
			m_warningTag(m_session->RegisterRecordKind(L"Warning", false)),
			m_errorTag(m_session->RegisterRecordKind(L"Error", false))
		{}
	
		iLogW::LogSession &Session() {	return *m_session;	}
		
		void LogLevel(int level)
		{
			ESS_ASSERT(level >= 0 && level < 4) ;

			m_infoTag.Activate(false);
			m_warningTag.Activate(false);
			m_errorTag.Activate(false);

			if(level == 0) 
			{
				m_session->LogActive(false);
				return;
			}

			m_session->LogActive(true);
			m_errorTag.Activate(true);			

			if(level == 1) return;

			m_warningTag.Activate(true);			
			if(level == 2) return;


			m_infoTag.Activate(true);			

			return;
		}

		int LogLevel() const
		{
			if(IsTagActive(InfoTag)) return 3;
			if(IsTagActive(WarningTag)) return 2;
			if(IsTagActive(ErrorTag))  return 1;
			
			return 0;
		}

		bool IsTagActive(LogTag tag) const
		{
			return m_session->LogActive(Tag(tag));
		}

		bool IsLogActive(LogTag tag)
		{
			if(!IsTagActive(tag)) return false;
			
			*m_session << Tag(tag);

			return true;
		}


		const iLogW::LogRecordTag& Tag(LogTag tag) const
		{
			if(tag == ErrorTag)  return m_errorTag;
			if(tag == WarningTag)return m_warningTag;
			ESS_ASSERT(tag == InfoTag);
			return m_infoTag;
		}

		iLogW::LogRecordTag& Tag(LogTag tag)
		{
			if(tag == ErrorTag)  return m_errorTag;
			if(tag == WarningTag)return m_warningTag;

			ESS_ASSERT(tag == InfoTag);
			return m_infoTag;
		}

	private:
		boost::scoped_ptr<iLogW::LogSession> m_session;
		iLogW::LogRecordTag m_infoTag;
		iLogW::LogRecordTag m_warningTag;
		iLogW::LogRecordTag m_errorTag;
	};

	// -------------------------------------------------------------------------------

	void NamedObject::InitLog()
	{
		ESS_ASSERT(m_log == 0);

		iLogW::ILogSessionCreator &logCreator(m_domain.Log());
		if(!m_root) logCreator = m_pParent->Log();

		m_log.reset(new LogImpl(
			logCreator.CreateSession(m_name.Name().toStdString(), true)));

		if(m_pParent != 0) m_log->LogLevel(m_pParent->LogLevel());
	}

	// -----------------------------------------------

	iLogW::LogSession &NamedObject::Log()
	{
		if(m_log == 0) InitLog();

		return m_log->Session();
	}

	// -------------------------------------------------------------------------------

	iLogW::LogSession &NamedObject::Log(LogTag tag)
	{
		if(m_log == 0) InitLog();

		return m_log->Session() << m_log->Tag(tag);
	}

	// -------------------------------------------------------------------------------

	bool NamedObject::IsLogActive(LogTag tag)
	{
		return (m_log == 0) ? false : m_log->IsLogActive(tag);
	}

	// -------------------------------------------------------------------------------

	bool NamedObject::IsTagActive(LogTag tag) const
	{
		return (m_log == 0) ? false : m_log->IsTagActive(tag);
	}

	// -------------------------------------------------------------------------------

	void NamedObject::SetupLogLevelToChild(int level)
	{
		if(m_log != 0) m_log->LogLevel(level);
	}

	// -----------------------------------------------

	bool NamedObject::LogLevel(int level)
	{
		if(level < 0 || level > 3) return false;

		if(m_log == 0) InitLog();

		m_log->LogLevel(level);

		for(ChildrenList::iterator i = m_children.begin();
			i != m_children.end(); ++i)
		{
			(*i)->SetupLogLevelToChild(level);
		}
		
		return true;
	}

	// -----------------------------------------------

	int  NamedObject::LogLevel() const
	{
		return (m_log == 0) ? 0 : m_log->LogLevel();
	}

	// -------------------------------------------------------------------------------
	// Dri wrapper

	void NamedObject::LogLevelDri(int level)
	{
		if(!LogLevel(level)) 
			ThrowRuntimeException("Wrong log level value. Allowable value: 0..3");
	}

	// -----------------------------------------------

	int  NamedObject::LogLevelDri() const
	{
		return LogLevel();
	}

	// -----------------------------------------------

    bool NamedObject::AbortAsyncCommand()
    {   
        ESS_ASSERT(AsyncActive());
        
        if (m_asyncAbort.empty()) return false;
        
        m_asyncAbort();

        m_asyncAbort.clear();

        return (!AsyncActive());
    }

	// -----------------------------------------------

    void NamedObject::EnableThreadHook()
    {
        ESS_ASSERT(!m_exceptionHookEnabled);

        m_exceptionHookEnabled = true;
        m_domain.RegisterExceptionHook(this);
    }

    // -----------------------------------------------

    void NamedObject::DisableThreadHook()
    {
        ESS_ASSERT(m_exceptionHookEnabled);

        m_exceptionHookEnabled = false;
        m_domain.UnregisterExceptionHook(this);
    }

    // -----------------------------------------------

    QStringList NamedObject::GetAllDriActiveMethodList()
    {
        QStringList sl;

        // sl.append("CS_SwitchSecProfile");
        sl.append("ObjCreate");
        sl.append("ObjDelete");

        return sl;
    }

}  // namespace Domain


