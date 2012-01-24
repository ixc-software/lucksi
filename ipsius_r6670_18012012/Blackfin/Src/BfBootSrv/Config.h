#ifndef CFGNEW_H
#define CFGNEW_H

#include "FieldList.h"
#include "BfBootCore/ImgDescriptor.h"
#include "RangeIOBase.h"
#include "ConfigLineList.h"

namespace BfBootSrv
{       

    using Platform::word;

    // Конфигурация загрузчика и приложения со средствами сериализации    
    class Config
    {   
    private:               
        FieldList m_metaFields; 

    private:
        void Restore(RangeIOBase& flash, RangeIOBase::NamedRangeIndex range); // can throw CfgRestoreError;
        Config();

    // public fields
    public: 
        typedef Platform::dword TCounter; // обязательно беззнаковый
        // При правке полей необходимо изменить CVer в .cpp !        
        // General:
        const CfgField<int> CfgVresion;
        CfgField<TCounter> UpdateCount;
        CfgField<dword> BooterVersion;
        CfgField<std::string> BooterVersionInfo;

        // Network settings:        
        CfgField<bool> UseDHCP;
        CfgField<std::string> NetMask;
        CfgField<std::string> IP;
        CfgField<std::string> Gateway;        
        

        // Server settings:
        CfgField<std::string> UserPwd;
        CfgField<dword, Range> WaitLoginMsec;        
        CfgField<BfBootCore::ScriptList> StartScriptList;

        // BfStorage data:
        CfgField< BfBootCore::ImgDescrList > AppImgTable;
        CfgField< dword > SoftwareRelNum;        

        CfgField<word> CmpPort;       // порт приложения

        // Настройки вывода трассировки.
        CfgField<std::string> UdpLogHost;
        CfgField<word> UdpLogPort;
        CfgField<bool> UseTimestamp;
        CfgField<word> CountToStore;
        
        enum {NoneRange = RangeIOBase::min};
        int/*RangeIOBase::NamedRangeIndex*/ SourceRangeIndex;     

        //enum {CVer = 6};
        enum {CVer = 7}; // add booter version
   
    // methods:
    public:
        static shared_ptr<Config> CreateCfg();        
        // can throw CfgRestoreError vs nullptr -- ? 
        static shared_ptr<Config> CreateCfg(RangeIOBase& flash, RangeIOBase::NamedRangeIndex range);        
        void Save(RangeIOBase& flash, RangeIOBase::NamedRangeIndex range);              
        // return false if invalid pair name-val
        bool SetUserSetting(const std::string& name, const std::string& val);
        void ListUserSettings(std::vector<BfBootCore::ConfigLine>& out) const;        
        // for tests
        bool Compare(const Config& other);                       
        
    };
} // namespace BfBootSrv

#endif
