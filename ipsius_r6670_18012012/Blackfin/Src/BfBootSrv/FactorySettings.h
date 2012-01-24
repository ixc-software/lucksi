#ifndef FACTORYSETTINGS_H
#define FACTORYSETTINGS_H

#include "FieldList.h"
#include "RangeIOBase.h"
#include "ConfigLineList.h"

namespace BfBootSrv
{
    const RangeIOBase::NamedRangeIndex CStoreRange = RangeIOBase::CfgDefault;

    using Platform::dword;

    class FactorySettings
    {         
    private:                
        FieldList m_metaFields; 
        const CfgField<int> m_vresion;    
        bool m_isEmpty;        
    private:
        FactorySettings();  
        void Restore(RangeIOBase& flash);

    public:        
        // Factory settings:
        CfgField<dword> HwNumber;     // заводской номер
        CfgField<dword> HwType;       // тип платы
        CfgField<std::string> MAC;        

    public:                
        
        static shared_ptr<FactorySettings> CreateEmpty();
        static shared_ptr<FactorySettings> ReadFromDefaultRange(RangeIOBase& flash);
        void SaveToDefaultRange(RangeIOBase& flash);
        bool IsEmpty() const;
        void Set(dword hwType, dword hwNum, const std::string& mac);       
        void ListSettings(std::vector<BfBootCore::ConfigLine>& out) const;        
      
        enum {CVer = 5};
    };
    
    

} // namespace BfBootSrv

#endif
