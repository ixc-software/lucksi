#include "stdafx.h"
#include "FactorySettings.h"

namespace
{
         
    
} // namespace

namespace BfBootSrv
{

    FactorySettings::FactorySettings() 
        : m_vresion("FactoryCfgVer", CfgFieldProp::InternalSettings,  CVer, m_metaFields),
        m_isEmpty(true),
        HwNumber   ("HwNumber",CfgFieldProp::InternalSettings, m_metaFields),
        HwType     ("HwType",  CfgFieldProp::InternalSettings, m_metaFields),
        MAC        ("MAC",     CfgFieldProp::InternalSettings, m_metaFields)
    {        
    }

    void FactorySettings::Restore( RangeIOBase& flash )
    {
        ConfigLineList list(flash.InStreamRange(CStoreRange)->getReader());
        m_metaFields.InitFieldsFrom(list);

        if (CVer != m_vresion.Value()) ESS_THROW(ErrCfgVersion);
        m_isEmpty = false;
    }

    shared_ptr<FactorySettings> FactorySettings::ReadFromDefaultRange( RangeIOBase& flash )
    {        
        try
        {            
        	shared_ptr<FactorySettings> result;
            result = CreateEmpty();
            result->Restore(flash);                                
            return result;
        }
        catch(CfgRestoreError& err)
        {                            
            return CreateEmpty();
        }   
        return shared_ptr<FactorySettings>(); // suppress warning                 
    }

    void FactorySettings::SaveToDefaultRange( RangeIOBase& flash )
    {
        ConfigLineList ll;            
        m_metaFields.WriteTo(ll);
        ll.Save(flash.OutStreamRange(CStoreRange)->getWriter());
    }

    bool FactorySettings::IsEmpty() const
    {
        return m_isEmpty;
    }

    void FactorySettings::Set( dword hwType, dword hwNum, const std::string& mac )
    {
        ESS_ASSERT(m_isEmpty);
        // todo if (!Validate(mac)) errInvalidMac;
        HwType = hwType;
        HwNumber = hwNum;
        MAC = mac;

        m_isEmpty = false;
    }

    shared_ptr<FactorySettings> FactorySettings::CreateEmpty()
    {
        return shared_ptr<FactorySettings>(new FactorySettings);
    }

    void FactorySettings::ListSettings( std::vector<BfBootCore::ConfigLine>& out ) const
    {
        m_metaFields.ListSettings(out, CfgFieldProp::InternalSettings);
    }
    
} // namespace BfBootSrv

