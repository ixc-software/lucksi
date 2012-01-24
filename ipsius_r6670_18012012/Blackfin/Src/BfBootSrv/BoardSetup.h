#ifndef BOARDPARAMDRIVER_H
#define BOARDPARAMDRIVER_H

#include "E1App/MngLwip.h"
#include "iLog/LogManagerOutput.h"
#include "iLog/LogSettings.h"

#include "RangeIOBase.h"
#include "Config.h"
#include "BfStorage.h"
#include "Script.h"
#include "FactorySettings.h"
#include "BfBootCore/BroadcastMsg.h"

namespace BfBootSrv
{
    using boost::shared_ptr;  
    using Platform::word;  

    // ����� �������������� ������ � ��������� ���������� � ������� ���������� Blackfin �������� �� ����� RangeIOBase,
    // �� ����������� � ����������. ����� �������� �������������� ��������.

   
    // �������������� ������� ����, ��������� ����������������� ����������� � ����� ����������� � ����. (��� �������?)
    class BoardSetup// : public IParamDriver 
        : boost::noncopyable
    {           
        /* ������������� � BoardSetup*/
        RangeIOBase& m_flash;

        shared_ptr<FactorySettings> m_factorySettings;
        shared_ptr<Config> m_cfg; 
        bool m_haveUnsavedChanges;
        bool m_haveSavedChanges;
        
        scoped_ptr<BfStorage> m_appStorage; //todo rename AppStorage     

        mutable scoped_ptr<Script> m_scriptCash; // ������ ��������� ��� ��������� ���������� �������    
        bool m_booterUpdated;

        
    // use by BootServer (���������� IBootCtrl)
    public:
        void SetUserParam(const std::string& name, const std::string& value);

        //void AddScript(const BfBootCore::NamedScript& newScript);       
        //void SetScriptList(const ScriptList& scripts);
        const BfBootCore::ScriptList& GetScriptList() const; // can throw;
        
        void SetFactoryParams( dword hwNumber, dword hwType, const std::string& mac);
        void GetParamList(std::vector<BfBootCore::ConfigLine>& out) const; // -- return type.  ?
        const BfBootCore::ImgDescrList& GetAppList() const;  // can throw;
        
        void OpenWriteImg(dword size); // can throw
        void WriteImgChunk(const std::vector<byte>& data); // crc �� ��������������  // can throw;
        void WriteImgChunk(const void* pData, Platform::dword length);
        void AbortWriteApp();

        const BfBootCore::ImgDescriptor& CloseNewImg(const std::string& name, dword CRC);
        void DeleteLastImg();// ��������� ���������� ����������.
        void DeleteAllImg(); // ��������� ���������� ����������.
        void CloseTransaction(dword relNumber); // ���������� � ������� ������� ����� ������ � ��������� ����������.;
        
        void SaveChanges();
        bool WriteBootImg(const std::vector<byte>& data, Utils::SafeRef<IWriteProgress> progInd, bool withVerify = true); // ������������ ������ std::vector<byte> ��� ����������� CRC -- ?
        bool IsDefault() const;
        bool IsHaveUnsavedChanges() const;
        bool IsHaveSavedCanges() const;
        bool IsBooterUpdated() const;
        dword getSpaceForNewBootImg() const;
        void EraseDefaultCfg(); 
        dword getSrvConnectionTimeout() const;

        bool ValidatePwd(const std::string& pwd) const;
        
        // ���������� ��������� ����������� �� ��������� ������ � Config.        
        void TuneTrace(iLogW::LogSettings& settings);
        
        // �������� � ������, �������, ������ � ������� ������� � ����������� ������������ �������. 
        // ���������� false ���� ������ �� ������� ���������.        
        bool PrepareScript(const std::string& scriptName) const;

        bool HavePreparedScript() const;

        // call PrepareScript before
        Script& getPreparedScript(); // ��� ExitAction;

        // return false if can`t (not exist settings if IsServiceMode)
        bool ReadNetworkSettings(E1App::BoardAddresSettings& net) const;

        // return 0 if not exist
        word getCmpPort() const;


        bool CanCreateBroadcast() const
        {
            if (m_haveSavedChanges && m_haveUnsavedChanges) return false;                        
            if (IsServiceMode()) return false;
            return true;
        }
        BfBootCore::BroadcastMsg CreateBroadcastForBooter(int CBPport)  const
        {
            return CreateBroadcast(CBPport);
        }

        BfBootCore::BroadcastMsg CreateBroadcastForApp()  const
        {
            ESS_ASSERT(!IsDefault() && !IsServiceMode()); 
            return CreateBroadcast(0);     
        }        

    public:
        BoardSetup (RangeIOBase& flash);
        bool IsServiceMode()const;// No correct FactorySettings in range CfgDefault.;
        
        //std::string getCfgRangeAsString();

        // ����� ������� �������� � ��������� (�������� �� RangeIOBase::CfgDefault).
        void ResetSettings();                                      

    // Own methods:
    private:             

        void InitConfig();
        void InitStorage();                 

        void OpenUpdateTrasaction(); // ������� ����� ������ � ��������� ������.         
        BfBootCore::BroadcastMsg CreateBroadcast(int CBPport) const;
    };   
   
} // namespace BfBootSrv

#endif
