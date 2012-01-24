#include "stdafx.h"
#include "Platform/Platform.h"
#include "ConfigSetup.h"
#include "BoardSetup.h"
#include "Config.h"

namespace BfBootSrv
{

    // приводит к стиранию первого сектора в регионе
    void DamageCfg(RangeIOBase& flash, RangeIOBase::NamedRangeIndex index)
    {
        // Damage second:
        flash.OutStreamRange(index)->getWriter().WriteByte(0);
    }  

    // ------------------------------------------------------------------------------------
    
    // return true if cfgField is changed
    template<class T1, class T2>
    bool SetMandatoryCfgField(T1& cfgField, const T2& presetField)
    {
        if (!cfgField.IsInited() || cfgField.Value() != presetField)
        {            
            cfgField = presetField;
            return true;
        }
        return false;
    }

    // ------------------------------------------------------------------------------------

    // return true if cfgField is changed    
    template<class T>
    bool SetMandatoryCfgField(BfBootSrv::CfgField<T>& cfgField, const Utils::InitProp<T>& presetField)
    {
        return SetMandatoryCfgField(cfgField, presetField.get());        
    }
    

    // ------------------------------------------------------------------------------------

    // return true if cfgField is changed
    template<class T1, class T2>
    bool SetOptionalCfgField(T1& cfgField, const T2& presetField)
    {
        if (presetField.IsInited()) return SetMandatoryCfgField(cfgField, presetField);
        return false;
    }

    // ------------------------------------------------------------------------------------    

    void ConfigSetup(RangeIOBase& flash, const BfBootCore::DefaultParam& preset, bool clearOther)
    {
        bool needSave = false;

        // Set FactorySettings
        {            
            boost::shared_ptr<BfBootSrv::FactorySettings> hw = BfBootSrv::FactorySettings::ReadFromDefaultRange(flash);
            if (hw->IsEmpty())
            {
                hw->HwNumber = preset.HwNumber;
                hw->HwType = preset.HwType;
                hw->MAC = preset.Mac;
                hw->SaveToDefaultRange(flash);
            }
        }

        RangeIOBase::NamedRangeIndex range = RangeIOBase::CfgPrim;
        boost::shared_ptr<BfBootSrv::Config> cfg = BfBootSrv::Config::CreateCfg(flash, range);
        if (!cfg) 
        {
            cfg = BfBootSrv::Config::CreateCfg();
            needSave = true;
        }               
        
        needSave = SetOptionalCfgField(cfg->CmpPort, preset.OptionalSettings.CmpPort) || needSave;
        needSave = SetOptionalCfgField(cfg->CountToStore, preset.OptionalSettings.CountToStore) || needSave;
        needSave = SetOptionalCfgField(cfg->UdpLogHost, preset.OptionalSettings.UdpLogHost) || needSave;
        needSave = SetOptionalCfgField(cfg->UdpLogPort, preset.OptionalSettings.UdpLogPort) || needSave;
        needSave = SetOptionalCfgField(cfg->UseTimestamp, preset.OptionalSettings.UseTimestamp) || needSave;
        needSave = SetOptionalCfgField(cfg->UserPwd, preset.OptionalSettings.UserPwd) || needSave;
        needSave = SetOptionalCfgField(cfg->WaitLoginMsec, preset.OptionalSettings.WaitLoginMsec) || needSave;

        if (preset.OptionalSettings.Network.IsInited())
        {
            const E1App::BoardAddresSettings& net = preset.OptionalSettings.Network;
            ESS_ASSERT(net.IsValid());
            
            needSave = SetMandatoryCfgField(cfg->IP, net.IP()) || needSave;
            needSave = SetMandatoryCfgField(cfg->Gateway, net.Gateway()) || needSave;
            needSave = SetMandatoryCfgField(cfg->NetMask, net.Mask()) || needSave;

            needSave = SetMandatoryCfgField(cfg->UseDHCP, net.DHCP()) || needSave;
        }

        if (needSave) cfg->Save(flash, RangeIOBase::CfgPrim);                

        if (clearOther)
        {
            DamageCfg(flash, RangeIOBase::CfgSecond);
            //DamageCfg(flash, RangeIOBase::CfgPrim);
        }     
    }
   
    // ------------------------------------------------------------------------------------

    void DeleteDefault(RangeIOBase& flash)
    {
        //DamageCfg(flash, RangeIOBase::CfgDefault);
        ESS_UNIMPLEMENTED;
    }


} // namespace BfBootSrv

