#ifndef IEFACTORY_H
#define IEFACTORY_H

#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "Utils/BaseSpecAllocated.h"
#include "Utils/ManagedList.h"

#include "ICommonIe.h"
#include "isdninfr.h"

namespace ISDN
{   
    class RawData;

    // Фабрика информационных элементов. 
    // Создает конкретный Ie с потока по указанному стандартному идентификатору
    class IeFactory : boost::noncopyable
    {
        class IMetaInterface;

        template<class T>
        class MetaType;

        //typedef std::map<IeId, IMetaInterface*> IeMap;
        // typedef boost::ptr_map<IeId, IMetaInterface> IeMap;

    public:  

        enum
        {
            CSpecCodSet = 255,
            CSpecId_IeUnknown = 0,
        };        
        
        IeFactory(IsdnInfra& infra);

        ~IeFactory();

        template<class T>
        void RegisterIe();

        ICommonIe* CreateIeObj(IeId id, const RawData& raw);

    private:

        static void RegIes(IeFactory& factory);

        IsdnInfra& m_infra;
        Utils::ManagedList<IMetaInterface> m_IeCreators;
    };

    /*
        Функция регистрации определяется в .cpp-файле.
        Это возможно тк инстанцирование этой фции происходит в h-файле заголовков типов для этой фабрики
        который включается cpp-файл фабрики.
    */

} // namespace ISDN


#endif

