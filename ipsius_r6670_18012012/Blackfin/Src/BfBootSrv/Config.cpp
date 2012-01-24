#include "stdafx.h"
#include "Config.h"
#include "CfgRestoreError.h"
#include "BoardHardcodedConfig.h"
#include "BfBootCore/BooterVersion.h"



namespace BfBootSrv
{

    void Config::Restore( RangeIOBase& flash, RangeIOBase::NamedRangeIndex range ) /* can throw CfgRestoreError */
    {
        ConfigLineList list(flash.InStreamRange(range)->getReader());
        m_metaFields.InitFieldsFrom(list);        

        if (CVer != CfgVresion.Value()) ESS_THROW(ErrCfgVersion);
        /*
        {            
            ESS_THROW_MSG(ErrCfgVersion,  BfBootCore::StringToTypeConverter::toString( CfgVresion.Value() ) + " in loaded cfg" );                
        }
        */

        SourceRangeIndex = range;
    }

    Config::Config() 
        : CfgVresion    ("CfgVersion",      CfgFieldProp::InternalSettings, CVer,m_metaFields),
        UpdateCount     ("UpdateCount",     CfgFieldProp::InternalSettings, 0,       m_metaFields),
        BooterVersion   ("BooterVersion",   CfgFieldProp::InternalSettings, BfBootCore::BooterVersionNum(), m_metaFields),
        BooterVersionInfo("BooterVersionInfo",CfgFieldProp::InternalSettings, BfBootCore::BooterVersionInfo(), m_metaFields),
        UseDHCP         ("UseDHCP",         CfgFieldProp::UserSettings, false,       m_metaFields),
        NetMask         ("NetMask",         CfgFieldProp::UserSettings, "255.255.255.0", m_metaFields),          
        IP              ("IP",              CfgFieldProp::UserSettings, "192.168.0.101", m_metaFields),          
        Gateway         ("Gateway",         CfgFieldProp::UserSettings, "192.168.0.1",   m_metaFields),          
        
        UserPwd         ("UserPwd",         CfgFieldProp::UserSettings, "",          m_metaFields),
        WaitLoginMsec   ("WaitLoginMsec",   CfgFieldProp::UserSettings, 10*1000,    m_metaFields, Range<dword>(1000, 600*1000) ),
        StartScriptList ("StartScriptList", CfgFieldProp::UserSettings,             m_metaFields), // todo InternalSettings
        AppImgTable     ("AppImgTable",     CfgFieldProp::InternalSettings,         m_metaFields),
        SoftwareRelNum  ("SoftwareRelNum",  CfgFieldProp::InternalSettings,         m_metaFields),
        
        CmpPort         ("CmpPort",         CfgFieldProp::UserSettings,             m_metaFields),
        UdpLogHost      ("UdpLogHost",      CfgFieldProp::UserSettings, /*CUdpLogHost,*/    m_metaFields),
        UdpLogPort      ("UdpLogPort",      CfgFieldProp::UserSettings, /*CUdpLogPort, */   m_metaFields),
        UseTimestamp    ("UseTimestamp",    CfgFieldProp::UserSettings, CUseTimestamp,  m_metaFields),
        CountToStore    ("CountToStore",    CfgFieldProp::UserSettings, CUdpContToStore,m_metaFields),
        SourceRangeIndex(NoneRange)
    {
        BfBootCore::ImgDescrList emptyDescr;
        AppImgTable.Init(emptyDescr);

        BfBootCore::ScriptList emptyScript;
        StartScriptList.Init(emptyScript);
    }

    shared_ptr<Config> Config::CreateCfg() // static
    {
        return shared_ptr<Config>( new Config() );
    }

    shared_ptr<Config> Config::CreateCfg( RangeIOBase& flash, RangeIOBase::NamedRangeIndex range ) // static
    {
        shared_ptr<Config> result = CreateCfg(); // can throw
        try
        {
            result->Restore(flash, range);
        }
        catch (CfgRestoreError& e)
        {   
            return shared_ptr<Config>();
        }
        
        return result;
    }

    void Config::Save( RangeIOBase& flash, RangeIOBase::NamedRangeIndex range )
    {
        ESS_ASSERT(range != RangeIOBase::CfgDefault);

        ConfigLineList ll;            
        m_metaFields.WriteTo(ll);
        ll.Save(flash.OutStreamRange(range)->getWriter());
    }

    bool Config::SetUserSetting( const std::string& name, const std::string& val )
    {
        return m_metaFields.SetParam(name, CfgFieldProp::UserSettings, val);
    }

    void Config::ListUserSettings( std::vector<BfBootCore::ConfigLine>& out ) const
    {
        //stub! only UserSettings - ?
        const ICfgField& ver = CfgVresion;
        out.push_back(ver.ToCfgLine()); 

        m_metaFields.ListSettings(out, CfgFieldProp::UserSettings);
    }

    bool Config::Compare( const Config& other )
    {
        return m_metaFields.Compare(other.m_metaFields);
    }

    
} // namespace BfBootSrv

