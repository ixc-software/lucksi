
#ifndef __METAOBJECTSINFO__
#define __METAOBJECTSINFO__


#include "Domain/NamedObject.h"
#include "Utils/ManagedList.h"

namespace DriSec
{
    class ISecurity;
}

// -----------------------------------------------

namespace DRI
{
    class ExecutiveDRI;
    class INamedObjectType;

    namespace MetaObjectsInfoDetail
    {
        // All methods excluded inherited
        class ObjMethods
        {
            ExecutiveDRI &m_exeDRI;
            QString m_objType;
            ObjMethods *m_pInherited;
            QStringList m_list;
            bool m_debug;
    
            static QString ExtractMethodName(const QString &signature);
            static QString ResolveMethodSignature(bool trace, const QString &storedSig, 
                                                      const QString &newSig);
            static bool ParamsTypesMatch(bool trace, const QString &ourMethod, 
                                             const QMetaMethod &checkedMethod,
                                             /* out */ QString &info);

            bool IsSameMethod(int index, const QMetaMethod &method) const;
            QString MethodToString(const QMetaMethod &method);
            void Parse(const QMetaObject &methodList);

            // Look through current object methods only
            int FindCurrMethodIndex(const QMetaMethod &method) const;
            
        public:
            ObjMethods(ExecutiveDRI &exeDRI, 
                       const QMetaObject &methodList, 
                       ObjMethods *pInherited = 0);
    
            // Returns signature. Look through inherited methods also
            QString GetMethod(const QMetaMethod &method) const;
            QStringList Get(int indent, bool withInherited) const;
            bool IsSame(const QMetaObject &other) const;

            Utils::StringList ListMethodsNames() const;
        };
        
    } // MetaObjectsInfoDetail

    // ---------------------------------------------------------------
    
    // All registered classes must be inherited from QObject!
    class MetaObjectsInfo : boost::noncopyable
    {
        typedef MetaObjectsInfoDetail::ObjMethods ObjMethods;
        
        ExecutiveDRI &m_exeDRI;
        Utils::ManagedList<ObjMethods> m_list;
        bool m_debug;

        static void ListEnumsExtern(Utils::StringList &sl, 
                             const QMetaObject *pMeta, 
                             int ident, 
                             bool noInherited);

        ObjMethods* FindObject(const QMetaObject &meta, bool autoRegister = true);

        // Except QObject
        /*static*/ void AppendBaseMetaObjList(const QMetaObject *pMeta, 
                                              Utils::ManagedList<const QMetaObject> &res) const;

        ObjMethods* RegisterMetaMethods(const QMetaObject *pMeta);
        
    public:
        MetaObjectsInfo(ExecutiveDRI &exeDRI);

        // Register this  type and all base types upto QObject 
        // if they are not in list already 
        void RegisterMeta(const QMetaObject *pMeta);

        // Register pOwner at first if method was not found in list
        QString MethodToString(const QMetaObject *pOwner,
                               const QMetaMethod &method);

        // Register pMeta at first if its methods was not found in list
        Utils::StringList ListMethods(const QMetaObject *pMeta, 
                                      int indent, 
                                      bool noInherited);

        Utils::StringList ListMethodsNames(const QMetaObject *pMeta);

        
        static QString EnumToString(const QMetaEnum &e);

        static Utils::StringList ListEnums(const QMetaObject *pMeta, 
                                               int indent, 
                                               bool noInherited,
                                               bool addExternThruPropertySearch = true);

        QString PropertyToString(const Domain::NamedObject *pObj,
                                 const QMetaProperty &prop, 
                                 Domain::NamedObject::ListPropertiesMode mode,
                                 bool withMarkRO,
                                 DriSec::ISecurity *pSec) const;

        Utils::StringList ListProperties(const Domain::NamedObject *pObj, 
                                         const QMetaObject *pMeta,
                                         Domain::NamedObject::ListPropertiesMode mode, 
                                         int indent, 
                                         bool noInherited,
                                         DriSec::ISecurity *pSec) const;

        Utils::StringList ListMetaTypeInfo(const QMetaObject *pMeta, 
                                           bool fullInfo, 
                                           bool noInherited);

        void AddMetaIdentificators(Utils::StringList &sl, 
                                   INamedObjectType *pType, 
                                   bool getInherited) const;
    };
    
} // namespace DRI

#endif
