
#include "stdafx.h"

#include "Utils/QtHelpers.h"
#include "DriSec/SecUtils.h"

#include "MetaObjectsInfo.h"
#include "ExecutiveDRI.h"
#include "driutils.h"
#include "NamedObjectFactory.h"
#include "MetaObjectsInfo_helpers.h"

// -------------------------------------------------------------

namespace
{

    // detect thru type name like Ixxx
    bool IsSpecialParam(const QByteArray &paramType) 
    {
        QString typeName = DRI::DriUtils::FormatedTypename( paramType.data(), true ).c_str();

        if (typeName.size() > 2)
        {
            if ( (typeName.at(0) == 'I') && (typeName.at(1).isUpper()) )
            {
                return true;
            }
        }

        // prev version, don't work for enum 
        // (dri.ArgFactory().Find(typeName) == 0)

        return false;  
    }

    bool IsSpecialParamStr(const QString &paramType) 
    {
        return IsSpecialParam( paramType.toStdString().c_str() );
    }

    QStringList TypelistFromMethod(QString method)
    {
        QStringList lx;

        // remove '[' + ']'
        {
            int pos = method.indexOf('[');
            if (pos >= 0)
            {
                method.remove(pos, 1);
                pos = method.indexOf(']');
                ESS_ASSERT(pos >= 0);
                method.remove(pos, 1);
            }
        }

        // split params between '(' + ')'
        {
            int begin = method.indexOf("(");
            int end = method.indexOf(")");
            ESS_ASSERT( (begin < end) && (begin > 0) );

            lx = method.mid(begin + 1, (end - begin - 1)).split(",", QString::SkipEmptyParts);
        }

        // extract types 
        for(int i = 0; i < lx.size(); ++i)
        {
            QString type( lx.at(i).trimmed() );
            int pos = type.indexOf(" ");
            ESS_ASSERT(pos >= 0);
            type = type.left(pos);

            lx[i] = type;
        }

        return lx;
    }

    QStringList TypelistFromMethod(const QMetaMethod &method)
    {
        QStringList lx;

        {
            QList<QByteArray> typeList( method.parameterTypes() ); 
            for(int i = 0; i < typeList.size(); ++i) lx.append( typeList.at(i).constData() );
        }

        if ( lx.size() && IsSpecialParamStr(lx.at(0)) )
        {
            lx.removeAt(0);  // remove first special param
        }

        return lx;
    }

}  // namespace



// -------------------------------------------------------------

// MetaObjectsInfoDetail::ObjMethods impl
namespace DRI
{
    namespace MetaObjectsInfoDetail
    {
        ObjMethods::ObjMethods(ExecutiveDRI &exeDRI, 
                               const QMetaObject &methodList, 
                               ObjMethods *pInherited) : 
            m_exeDRI(exeDRI), m_objType(methodList.className()), m_pInherited(pInherited) 
        {
            m_debug = false;
            
            Parse(methodList);
             
            // check that !m_list.isEmpty() ?
        }

        // -----------------------------------------------------------------
        
        QString ObjMethods::ResolveMethodSignature(bool trace, const QString &storedSig, 
                                                   const QString &newSig) 
        {
            if (trace) std::cout << "Resolve method signature: " << storedSig 
                            << " and " << newSig << std::endl;

            ESS_ASSERT(storedSig != newSig);
            
            const QString openBreak("[");
            const QString closeBreak("]");

            QString res;
            if (storedSig.size() > newSig.size())
            {
                res = storedSig;
                int openIndex = res.indexOf(openBreak);
                if ((openIndex < 0) || (openIndex > newSig.indexOf(")")))
                {
                    res.remove(openBreak);
                    res.insert(newSig.indexOf(")"), openBreak);
                    if (!res.contains(closeBreak)) res.insert(res.indexOf(")"), closeBreak);
                }

                return res;
            }

            // storedSig < newSig
            res = newSig;
            int openIndex = storedSig.indexOf(openBreak);
            int openInsertPos = (openIndex < 0) ? storedSig.indexOf(")") : openIndex;
            res.insert(openInsertPos, openBreak);
            res.insert(res.indexOf(")"), closeBreak);
            
            return res;
        }

        // --------------------------------------------------------------------
        
        QString ObjMethods::ExtractMethodName(const QString &signature)
        {
            ESS_ASSERT(signature.count("(") == 1);
            
            return signature.left(signature.indexOf("(")).trimmed();
        }

        // -----------------------------------------------------------------

        bool ObjMethods::ParamsTypesMatch(bool trace, const QString &ourMethod, 
            const QMetaMethod &checkedMethod,
            /* out */ QString &info)
        {
            if (trace) std::cout << "Compare params types: " << ourMethod 
                            << " and " << checkedMethod.signature() << std::endl;

            QStringList ourTypelist = TypelistFromMethod(ourMethod);
            QStringList checkedTypelist = TypelistFromMethod(checkedMethod);

            if (ourTypelist != checkedTypelist)
            {
                info = QString("Fail for %1; parsed {%2} vs {%3}").arg(ourMethod).
                    arg( ourTypelist.join(", ") ).arg( checkedTypelist.join(", ") );

                return false;
            }

            return true;
        }

        // -----------------------------------------------------------------

        bool ObjMethods::IsSameMethod(int index, const QMetaMethod &method) const
        {
            QString name = ExtractMethodName(m_list.at(index));

            // check name and params
            if (name.toStdString() != DRI::DriUtils::MethodName(method)) return false;

            QString matchInfo;
            if ( !ParamsTypesMatch(m_debug, m_list.at(index), method, matchInfo) )
            {               
                int dummi = 0;

                // actually IsSameMethod() just search thru only method name match - !
                // ESS_THROW_MSG(ESS::Exception, matchInfo.toStdString());
            }
            
            return true;
        }

        // -----------------------------------------------------------------

        QString ObjMethods::MethodToString(const QMetaMethod &method)
        {
            if (m_debug) std::cout << "Save method: " << method.signature() << std::endl;
            
            QString res;

            // return type
            QString returnType(method.typeName());
            if (returnType.isEmpty()) returnType = "void";
            returnType += " ";

            // function name
            res += DRI::DriUtils::MethodName(method).c_str();

            // params
            QList<QByteArray> paramTypes = method.parameterTypes();
            QList<QByteArray> paramNames = method.parameterNames();

            QString params = "(";

            for(int i = 0; i < paramTypes.size(); ++i)
            {
                // skip first special params
                if ((i == 0) && (IsSpecialParam(paramTypes.at(i)))) continue;

                params += paramTypes.at(i);
                if (i < paramNames.size()) params += QString(" ") + paramNames.at(i);
                if (i != (paramTypes.size() - 1)) params += ", ";
            }

            params += ")";
            res += params;

            return res;
        }

        // -----------------------------------------------------------------
        
        void ObjMethods::Parse(const QMetaObject &methodList)
        {
            if (m_debug) std::cout << "Parse object: " 
                                << methodList.className() << std::endl;
            
            // save all methods EXCLUDED inherited
            const QMetaObject *pBaseIgnore = methodList.superClass();

            for(int i = 0; i < methodList.methodCount(); ++i)
            {
                QMetaMethod method = methodList.method(i);
    
                // skip base stuff
                if (pBaseIgnore->indexOfMethod(method.signature()) >= 0) continue;

                QString newMetSig = MethodToString(method);
                int found = FindCurrMethodIndex(method);
                if (found < 0) 
                {
                    m_list.push_back(newMetSig); // no indents
                    continue;
                }

                // overwrite found method, if new one has longer signature 
                QString foundMetSig = m_list[found];
                ESS_ASSERT(newMetSig != foundMetSig); // can be just shorter or longer

                m_list.replace( found, ResolveMethodSignature(m_debug, foundMetSig, newMetSig) );
            }

        }
        
        // -----------------------------------------------------------------

        int ObjMethods::FindCurrMethodIndex(const QMetaMethod &method) const
        {
            if (m_debug) std::cout << "Find method (non-inherited): " 
                                << method.signature() << " -- ";
            
            // find here
            for (int i = 0; i < m_list.size(); ++i)
            {
                if (IsSameMethod(i, method))
                {
                    if (m_debug) std::cout << "OK" << std::endl;
                    return i;
                }
            }

            if (m_debug) std::cout << "NO" << std::endl;

            return -1;
        }

        // -----------------------------------------------------------------
        
        // returns signature
        QString ObjMethods::GetMethod(const QMetaMethod &method) const
        {
            if (m_debug) std::cout << "Get method(" << m_objType << "):" 
                                <<  method.signature() << std::endl;
            
            // find in here
            int index = FindCurrMethodIndex(method);
            if (index >= 0) return m_list[index];

            // find in base
            QString res = m_pInherited->GetMethod(method);
            ESS_ASSERT(!res.isEmpty());
            
            return res;
        }

        // -----------------------------------------------------------------

        QStringList ObjMethods::Get(int indent, bool withInherited) const 
        {
            if (m_debug) std::cout << "Get("<< m_objType << "):" << std::endl;
            
            QStringList res; // (m_list);

            // inherited
            if ((withInherited) && (m_pInherited != 0))
            {
                if (m_debug) std::cout << "Inherited:" << std::endl;
                res += m_pInherited->Get(indent, withInherited);
            }

            // current(append indent)
            for (int i = 0; i < m_list.size(); ++i)
            {
                res.push_back(::MakeIndent(indent) + m_list.at(i));
            }

            if (m_debug) std::cout << "All:" << std::endl << res.join("\n") 
                                << std::endl << std::endl;
            
            return res; 
        }

        // -----------------------------------------------------------------

        bool ObjMethods::IsSame(const QMetaObject &other) const  
        { 
            return (other.className() == m_objType);
        }

        // -----------------------------------------------------------------

        Utils::StringList ObjMethods::ListMethodsNames() const
        {
            QStringList res; 

            // inherited
            if (m_pInherited != 0)
            {
                res += m_pInherited->ListMethodsNames();
            }

            // current
            for (int i = 0; i < m_list.size(); ++i)
            {
                QString m = m_list.at(i);
                int pos = m.indexOf("(");
                ESS_ASSERT(pos > 0);
                res.push_back( m.mid(0, pos) );
            }

            return res; 
        }

    } // namespace MetaObjectsInfoDetail 
    
} // namespace DRI


// --------------------------------------------------------------------
// MetaObjectsInfo impl

namespace DRI
{
    MetaObjectsInfo::MetaObjectsInfo(ExecutiveDRI &exeDRI) : 
        m_exeDRI(exeDRI)
    {
        m_debug = false;
    }

    // --------------------------------------------------------------------
    // Methods handlers
    
    QString MetaObjectsInfo::MethodToString(const QMetaObject *pOwner, 
                                            const QMetaMethod &method)
    {
        if (m_debug) std::cout << "Method to string (" << pOwner->className() 
                            << "): " << method.signature() << std::endl;
        
        ESS_ASSERT(pOwner != 0);
        ObjMethods *pInfo = FindObject(*pOwner);
        ESS_ASSERT(pInfo != 0);

        return pInfo->GetMethod(method);
    }

    // --------------------------------------------------------------------

    Utils::StringList MetaObjectsInfo::ListMethods(const QMetaObject *pMeta, 
                                                   int indent, 
                                                   bool noInherited)
    {
        if (m_debug) std::cout << "List methods (" << pMeta->className() 
                        << "):" << std::endl; 
            
        ESS_ASSERT(pMeta != 0);

        ObjMethods *pInfo = FindObject(*pMeta);
        ESS_ASSERT(pInfo != 0);
        
        return pInfo->Get(indent, !noInherited);
    }

    Utils::StringList MetaObjectsInfo::ListMethodsNames( const QMetaObject *pMeta )
    {
        ESS_ASSERT(pMeta != 0);

        ObjMethods *pInfo = FindObject(*pMeta);
        ESS_ASSERT(pInfo != 0);

        return pInfo->ListMethodsNames();
    }

    // --------------------------------------------------------------------
    // Enum handlers

    QString MetaObjectsInfo::EnumToString(const QMetaEnum &e)
    {
        QString res;

        for(int i = 0; i < e.keyCount(); ++i)
        {
            res += e.key(i);
            if (i != (e.keyCount() - 1)) res += ", ";
        }

        if (e.isFlag()) res = QString("%1 [ %2 ]").arg(e.name()).arg(res);
                   else res = QString("%1 { %2 }").arg(e.name()).arg(res);

        return res;
    }

    // --------------------------------------------------------------------

    // добавляет информацию об типах enum, если этот тип описан в другом классе, но
    // используется как тип одного из свойства
    void MetaObjectsInfo::ListEnumsExtern(Utils::StringList &sl, 
                                          const QMetaObject *pMeta, 
                                          int indent, 
                                          bool noInherited)
    {
        const QMetaObject *pBaseIgnore = (noInherited) ? pMeta->superClass() :
            &QObject::staticMetaObject;

        for(int i = 0; i < pMeta->propertyCount(); ++i)
        {
            QMetaProperty prop = pMeta->property(i);

            if (pBaseIgnore->indexOfProperty(prop.name()) >= 0) continue;

            if (!EnumExternType(pMeta->className(), prop.typeName())) continue;

            if (prop.isFlagType() || prop.isEnumType())
            {
                QString s = EnumToString( prop.enumerator() );
                sl.push_back(::MakeIndent(indent) + s);
            }
        }
    }

    // --------------------------------------------------------------------
    
    // static
    Utils::StringList MetaObjectsInfo::ListEnums(const QMetaObject *pMeta, 
                                                 int indent, 
                                                 bool noInherited,
                                                 bool addExternThruPropertySearch) 
    {
        // static const QMetaObject qObjMeta = QObject::staticMetaObject;
        const QMetaObject *pBaseIgnore = (noInherited) ? pMeta->superClass() :
                                                         &QObject::staticMetaObject;

        Utils::StringList sl;

        for(int i = 0; i < pMeta->enumeratorCount(); ++i)
        {
            QMetaEnum e = pMeta->enumerator(i);

            // skip basic object enums
            if (pBaseIgnore->indexOfEnumerator(e.name()) >= 0) continue;

            QString s = EnumToString(e);
            sl.push_back(::MakeIndent(indent) + s);
        }

        if (addExternThruPropertySearch)
        {
            ListEnumsExtern(sl, pMeta, indent, noInherited);
        }

        return sl;        
    }

    // --------------------------------------------------------------------
    // Property handlers
    
    QString MetaObjectsInfo::PropertyToString(const NamedObject *pObj, 
                                              const QMetaProperty &prop, 
                                              Domain::NamedObject::ListPropertiesMode mode,
                                              bool withMarkRO,
                                              DriSec::ISecurity *pSec) const
    {
        if (!prop.isReadable()) return "";

        bool addName = (mode != NamedObject::lpmValue);

        bool addTypeInfo = (mode == NamedObject::lpmNameTypeValue) || 
            (mode == NamedObject::lpmNameType);

        bool addValue = (mode == NamedObject::lpmNameTypeValue) || 
            (mode == NamedObject::lpmNameValue) ||
            (mode == NamedObject::lpmValue);

        if (addValue) 
        {
            ESS_ASSERT(pObj != 0);

            if (pSec != 0)
            {
                QString info;
                bool ok = pSec->VerifyAction(pObj, prop.name(), DriSec::ISecurity::akPropRead, info);
                if (!ok) return "";
            }
        }

        // name
        QString s;
        if (addName) s += prop.name();

        // type
        if (addTypeInfo)
        {
            s += ::CPropertyTypeSep;
            s += ::SimplifyPropertyTypename( prop.typeName() );  // simplify for enums only - ?
        }

        // value
        if (addValue)
        {
            if (!s.isEmpty()) s += " = ";

            s += m_exeDRI.PropertyFactory().Get(prop.name(), pObj);
        }

        // read-only
        if (withMarkRO && !prop.isWritable()) s += QString(" ") + ::CReadOnlyMarker;

        return s;
    }

    // --------------------------------------------------------------------

    Utils::StringList MetaObjectsInfo::ListProperties(const NamedObject *pObj, 
                                                      const QMetaObject *pMeta,
                                                      Domain::NamedObject::ListPropertiesMode mode, 
                                                      int indent, 
                                                      bool noInherited,
                                                      DriSec::ISecurity *pSec) const
    {
        const QMetaObject *pBaseIgnore = (noInherited) ? pMeta->superClass() :
            &QObject::staticMetaObject;

        Utils::StringList sl;

        for(int i = 0; i < pMeta->propertyCount(); ++i)
        {
            QMetaProperty prop = pMeta->property(i);

            if (pBaseIgnore->indexOfProperty(prop.name()) >= 0) continue;

            QString s = PropertyToString(pObj, prop, mode, true, pSec);
            if (s.isEmpty()) continue;

            sl.push_back(::MakeIndent(indent) + s);
        }

        return sl;
    }

    // --------------------------------------------------------------------
    // Other handlers
    
    Utils::StringList MetaObjectsInfo::ListMetaTypeInfo(const QMetaObject *pMeta, 
                                                        bool fullInfo, 
                                                        bool noInherited)
    {
        Utils::StringList sl;

        // class type
        {
            const QMetaObject *pCurrMeta = pMeta;
            QString classInfo = "class ";
            int count = 0;

            while(true)
            {
                QString curr = DRI::DriUtils::FormatedTypename(pCurrMeta->className(), true).c_str(); 
                if (curr == "QObject") break;

                if (::TypeIsNonCreatable(curr))
                {
                    curr += ' ';
                    curr += ::CReadOnlyMarker;
                }

                if (count > 0) classInfo += " : ";
                classInfo += curr;
                pCurrMeta = pCurrMeta->superClass();
                count++;
            }

            sl.push_back(classInfo);
        }

        if (!fullInfo) return sl;

        // list all stuff
        const int indent = 1;

        sl += ListEnums(pMeta, indent, noInherited);
        sl += ListProperties(0, pMeta, NamedObject::lpmNameType, indent, noInherited, 0);
        sl += ListMethods(pMeta, indent, noInherited);

        return sl;
    }

    // --------------------------------------------------------------------

    void MetaObjectsInfo::AddMetaIdentificators(Utils::StringList &sl, 
                                                INamedObjectType *pType, 
                                                bool getInherited) const
    {
        // type name
        ::AddUniqStringToList(sl, pType->TypeName());

        // props
        {
            Utils::StringList props = ListProperties(0, &pType->MetaObject(), 
                                                     Domain::NamedObject::lpmNameType, 0, 
                                                     !getInherited, 0);

            for(int i = 0; i < props.size(); ++i)
            {
                QString s = props.at(i);
                int pos = s.indexOf(::CPropertyTypeSep);
                ESS_ASSERT(pos >= 0);

                ::AddUniqStringToList(sl, s.left(pos));
            }
        }

        // methods 
        {
            Utils::StringList methods = 
                m_exeDRI.MetaObjInfo().ListMethods(&pType->MetaObject(), 0, !getInherited);

            for(int i = 0; i < methods.size(); ++i)
            {
                QString s = methods.at(i);
                int pos = s.indexOf('(');
                ESS_ASSERT(pos >= 0);

                ::AddUniqStringToList(sl, s.left(pos));
            }            
        }

        // enum values
        {
            ::AddEnumValues(sl, pType->MetaObject());
        }
    }

    // --------------------------------------------------------------------

    MetaObjectsInfoDetail::ObjMethods* MetaObjectsInfo::FindObject(const QMetaObject &meta, bool autoRegister) 
    {
        for (size_t i = 0; i < m_list.Size(); ++i)
        {
            if (m_list[i]->IsSame(meta)) return m_list[i];
        }

        if (autoRegister)
        {
            return RegisterMetaMethods(&meta);
        }

        return 0;
    }

    // --------------------------------------------------------------------

    // Except QObject
    // [0]          [1]         [2]
    // [Inherited11][Inherited1][Base]
    // static
    void MetaObjectsInfo::AppendBaseMetaObjList(const QMetaObject *pMeta, 
                                                Utils::ManagedList<const QMetaObject> &res) const
    {
        if (m_debug) std::cout << "Bases: ";
        // do not clear res

        const QMetaObject *pBase = pMeta->superClass();

        while (true)
        {
            if (pBase == &QObject::staticMetaObject) break; // skip Object
            if (pBase == 0) ESS_HALT("Object is not inherited from QObject");

            if (m_debug) std::cout << pBase->className()  << ", ";
            res.Add(pBase);
            
            pBase = pBase->superClass();
        }

        if (m_debug) std::cout << std::endl;
    }

    // --------------------------------------------------------------------

    MetaObjectsInfo::ObjMethods* MetaObjectsInfo::RegisterMetaMethods(const QMetaObject *pMeta)
    {
        ESS_ASSERT(pMeta != 0);

        if (m_debug) std::cout << "Register meta methods: " << pMeta->className() 
                        << std::endl;
        
        ESS_ASSERT(FindObject(*pMeta, false) == 0);

        Utils::ManagedList<const QMetaObject> addList(false); // not managed
        addList.Add(pMeta);
        AppendBaseMetaObjList(pMeta, addList);

        // Register pMeta bases (if need) and pMeta 
        // (look through in back order: from base type to current type)
        size_t last = addList.Size() - 1; 
        for (int i = last; i >= 0; --i) // int!
        {
            ObjMethods *pInherited = 0;

            if (i != last) pInherited = FindObject(*addList[i + 1], false);
           
            if (FindObject(*addList[i], false) != 0) continue;
            
            m_list.Add(new ObjMethods(m_exeDRI, *addList[i], pInherited));

            if (m_debug) std::cout << "Add meta: " << addList[i]->className() << std::endl;
        }

        // last one is current
        return m_list[m_list.Size() - 1];
    }
    
    // --------------------------------------------------------------------

    void MetaObjectsInfo::RegisterMeta(const QMetaObject *pMeta)
    {
        RegisterMetaMethods(pMeta);
    }

} // namespace DRI
