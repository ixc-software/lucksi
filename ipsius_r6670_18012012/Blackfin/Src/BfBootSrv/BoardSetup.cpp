#include "stdafx.h"
#include "BoardSetup.h"
#include "BfBootCore/BroadcastMsg.h"
#include "BfBootCore/BooterVersion.h"
#include "BfBootCore/UserResponseCode.h"

namespace
{
	const bool CTraceCout = false; // debug only
}

namespace BfBootSrv
{

    using namespace BfBootCore;

    void BoardSetup::SetUserParam( const std::string& name, const std::string& value )
    {                
        // if (IsServiceMode()) ESS_THROW_T(BoardException, errUnavailableInThisMode);
        if (!m_cfg->SetUserSetting(name, value)) ESS_THROW_T(BoardException, errInvalidPairNameVal);
        m_haveUnsavedChanges = true;
    }

    // ------------------------------------------------------------------------------------

    void BoardSetup::SetFactoryParams( dword hwNumber, dword hwType, const std::string& mac)
    {
        if (!IsServiceMode()) ESS_THROW_T(BoardException, errUnavailableInThisMode);        
        
        m_factorySettings->Set(hwType, hwNumber, mac);
        
        m_factorySettings->SaveToDefaultRange(m_flash); // here ?
        //m_haveUnsavedChanges = true; 
    }

    // ------------------------------------------------------------------------------------

    void BoardSetup::GetParamList( std::vector<BfBootCore::ConfigLine>& out ) const /* -- return type. ? */
    {
        out.clear();
        m_factorySettings->ListSettings(out);
        m_cfg->ListUserSettings(out);
        
    }

    // ------------------------------------------------------------------------------------

    const BfBootCore::ImgDescrList& BoardSetup::GetAppList() const
    {
        if (!m_cfg->SoftwareRelNum.IsInited()) ESS_THROW_T(BoardException, errWrongReleaseNumber);
        return m_cfg->AppImgTable.Value();
        // или BfStorage.getAppList() const
    }

    // ------------------------------------------------------------------------------------

    void BoardSetup::OpenWriteImg( dword size )
    {        
        //if ( IsServiceMode() ) ESS_THROW_T(BoardException, errUnavailableInThisMode);        
        m_appStorage->OpenForWrite(size); // can throw
    }

    // ------------------------------------------------------------------------------------

    void BoardSetup::WriteImgChunk( const std::vector<byte>& data ) /* crc не подсчитываетс€ // can throw */
    {
        if (data.empty()) return;
        m_appStorage->Write(&data[0], data.size());
    }

    // ------------------------------------------------------------------------------------

    void BoardSetup::WriteImgChunk(const void* pData, Platform::dword length) /* crc не подсчитываетс€ // can throw */
    {        
        m_appStorage->Write(pData, length);        
    }

    // ------------------------------------------------------------------------------------

    void BoardSetup::AbortWriteApp()
    {
        m_appStorage->AbortWrite();
    }

    // ------------------------------------------------------------------------------------

    const BfBootCore::ImgDescriptor& BoardSetup::CloseNewImg( const std::string& name, dword CRC )
    {
        const BfBootCore::ImgDescriptor& descr = m_appStorage->CloseWrite(name, CRC); // can throw        
        m_haveUnsavedChanges = true;
        OpenUpdateTrasaction();
        return descr;
    }

    // ------------------------------------------------------------------------------------

    void BoardSetup::DeleteLastImg()
    {
        //if (IsServiceMode())ESS_THROW_T(BoardException, errUnavailableInThisMode);
        OpenUpdateTrasaction();
        m_appStorage->DeleteLastImg(); // can throw if nothing
        m_haveUnsavedChanges = true;
        m_scriptCash.reset();
    }

    // ------------------------------------------------------------------------------------

    void BoardSetup::DeleteAllImg()
    {
        //if (IsServiceMode()) ESS_THROW_T(BoardException, errUnavailableInThisMode);
        OpenUpdateTrasaction();
        m_appStorage->DeleteAll(); // can throw if nothing
        m_haveUnsavedChanges = true;
        m_scriptCash.reset();
    }

    // ------------------------------------------------------------------------------------

    void BoardSetup::CloseTransaction( dword relNumber ) /* «аписывает в текущем конфиге номер релиза и выполн€ет сохранение. */
    {
        if (m_cfg->SoftwareRelNum.IsInited()) 
            ESS_THROW_T(BoardException, errNoOpenedUpdateTrasaction);
        m_cfg->SoftwareRelNum = relNumber;
        SaveChanges();
    }

    // ------------------------------------------------------------------------------------

    void BoardSetup::OpenUpdateTrasaction() /* —тирает номер релиза и сохран€ет конфиг. */
    {
        //if ( IsServiceMode() ) ESS_THROW_T(BoardException, errUnavailableInThisMode); 
        if (m_cfg->SoftwareRelNum.IsInited())
        {
            m_cfg->SoftwareRelNum.Clear();
            m_haveUnsavedChanges = true;
            SaveChanges();
        }
    }   

    // ------------------------------------------------------------------------------------

    void BoardSetup::SaveChanges()
    {
        if (!m_haveUnsavedChanges) ESS_THROW_T(BoardException, errSaveNothing);
        if (m_booterUpdated) ESS_THROW_T(BoardException, errSaveForbiddenBooterUpdated);

        //if (!IsServiceMode())ESS_THROW(UnavailableInThisMode); // проверить -- ?

        RangeIOBase::NamedRangeIndex range;
        switch (m_cfg->SourceRangeIndex)
        {
        case(Config::NoneRange):
            //range = RangeIOBase::CfgDefault;
            range = RangeIOBase::CfgPrim;
            break;
        case(RangeIOBase::CfgPrim):
            range = RangeIOBase::CfgSecond;
            break;
        case(RangeIOBase::CfgSecond):
            range = RangeIOBase::CfgPrim;
            break;
//         case(RangeIOBase::CfgDefault):
//             range = RangeIOBase::CfgPrim;
//             break;
        default:
            ESS_HALT("Invalid BootCtrlParams");
        }

        ++m_cfg->UpdateCount.Value();
        m_cfg->Save(m_flash, range);            
        m_haveUnsavedChanges = false;
        m_haveSavedChanges = true;
    }

    // ------------------------------------------------------------------------------------

    bool BoardSetup::WriteBootImg( const std::vector<byte>& data, Utils::SafeRef<IWriteProgress> progInd, bool withVerify) /* использовать вместо std::vector<byte> тип провер€ющий CRC -- ? */
    {
        ESS_ASSERT(m_flash.getFullSize() >= data.size());        
        ESS_ASSERT(!m_haveUnsavedChanges);
                
        shared_ptr<FlashOStream> stream = m_flash.OutStreamAll();
        stream->SetWriteProgressInd(progInd);
        stream->getWriter().WriteVectorByte(data);
        
        if (withVerify)
        {                        
            shared_ptr<FlashIStream> inStream = m_flash.InStreamAll();
            std::vector<byte> buff;

            const dword CBuffSize = 10 * 1024;
            dword offs = 0;
            while(offs < data.size())
            {
                dword size = ((data.size() - offs) >= CBuffSize) ? CBuffSize : data.size() - offs;
                inStream->getReader().ReadVectorByte(buff, size);
                if (!std::equal(data.begin() + offs, data.begin() + offs + size, buff.begin())) return false;//ESS_THROW_T(BoardException, errFlashWriting);                
                offs += size;
            }            
        }

        m_booterUpdated = true;
        return true;
    }

    // ------------------------------------------------------------------------------------

    bool BoardSetup::IsHaveUnsavedChanges() const
    {
        return m_haveUnsavedChanges;
    }

    // ------------------------------------------------------------------------------------

    bool BoardSetup::IsHaveSavedCanges() const
    {
        return m_haveSavedChanges;
    }

    // ------------------------------------------------------------------------------------

    bool BoardSetup::IsBooterUpdated() const
    {
        return m_booterUpdated;
    }

    // ------------------------------------------------------------------------------------

    BfBootSrv::dword BoardSetup::getSpaceForNewBootImg() const
    {
        return m_flash.getFullSize();
    }

    // ------------------------------------------------------------------------------------

    void BoardSetup::EraseDefaultCfg()
    {
        // —тирать не текущие ?
        //m_flash.OutStreamRange(RangeIOBase::CfgPrim)->getWriter().WriteByte(0);
        //m_flash.OutStreamRange(RangeIOBase::CfgSecond)->getWriter().WriteByte(0);
        m_flash.EraseRange(RangeIOBase::CfgPrim);
        m_flash.EraseRange(RangeIOBase::CfgSecond);

        m_flash.EraseRange(RangeIOBase::CfgDefault);
        m_factorySettings = FactorySettings::CreateEmpty();

        m_haveSavedChanges = true;
        
        // todo заблокировать любые изменени€ конфига тк в этом нет смысла
    }    
    
    // -------------------------------------------------------------------------------------

    const BfBootCore::ScriptList& BoardSetup::GetScriptList() const /* can throw */
    {
        if (!m_cfg->SoftwareRelNum.IsInited()) ESS_THROW_T(BoardException, errWrongReleaseNumber);               
        return m_cfg->StartScriptList.Value();
    }

    // -------------------------------------------------------------------------------------

    bool BoardSetup::PrepareScript(const std::string& scriptName) const
    {
    	if (!m_cfg->SoftwareRelNum.IsInited()) return false;
        if (IsServiceMode()) return false;

        if (CTraceCout) std::cout << "PrepareScript ... ";
        
        // find
        try
        {
            m_scriptCash.reset( new Script(scriptName, *m_cfg, *m_appStorage) );
        }
        catch(/*const*/ BfBootCore::ScriptError& e)
        {
            if (CTraceCout) std::cout << "failed. Err:" << e.what() << std::endl;
            return false;
        }
        if (CTraceCout) std::cout << "complete." << std::endl;
        return true;
    }

    // ------------------------------------------------------------------------------------

    bool BoardSetup::HavePreparedScript() const
    {
        return m_scriptCash.get() != 0;
    }

    // ------------------------------------------------------------------------------------

    Script& BoardSetup::getPreparedScript() /* дл€ ExitAction */
    {
        ESS_ASSERT(m_scriptCash); // script validate complete
        return *m_scriptCash;
    }

    // ------------------------------------------------------------------------------------
   

    BfBootCore::BroadcastMsg BoardSetup::CreateBroadcast( int CBPport ) const
    {
        ESS_ASSERT(CanCreateBroadcast());

        BroadcastMsg msg;

        msg.BooterVersionInfo = m_cfg->BooterVersionInfo.Value();
        msg.BooterVersionNum = m_cfg->BooterVersion.Value();

        msg.HwNumber = m_factorySettings->HwNumber.Value();
        msg.HwType = m_factorySettings->HwType.Value();
        msg.Mac = m_factorySettings->MAC.Value();

        msg.BooterPort = CBPport;        
        //msg.CmpPort = CmpPort;


        if (m_cfg->SoftwareRelNum.IsInited()) 
        {
            msg.SoftReleaseInfo = m_cfg->AppImgTable.Value();
            msg.SoftRelNumber = m_cfg->SoftwareRelNum.Value();
        }
        else
            msg.SoftRelNumber = BroadcastMsg::NoRelNum;

        return msg;
    }
    
    // -------------------------------------------------------------------------------------

    BoardSetup::BoardSetup( RangeIOBase& flash ) : m_flash(flash),
        m_factorySettings(FactorySettings::ReadFromDefaultRange(flash)),
        m_haveUnsavedChanges(false),
        m_haveSavedChanges(false),
        m_booterUpdated(false)
        //m_newImgWasAdded(false),
    {
        InitConfig();
        InitStorage();
        //InitNetwork();        
    }

    // ------------------------------------------------------------------------------------
    
    bool BoardSetup::IsServiceMode() const/* No correct Config in range CfgDefault. */
    {        
        return m_factorySettings->IsEmpty();
        //return m_cfg->SourceRangeIndex == Config::NoneRange;
    }

    // ------------------------------------------------------------------------------------

    void BoardSetup::ResetSettings()
    {
        ESS_ASSERT(m_cfg->SourceRangeIndex != Config::NoneRange);
        m_cfg = Config::CreateCfg();//(m_flash, RangeIOBase::CfgDefault); 
        m_cfg->Save(m_flash, RangeIOBase::CfgPrim);
        // затираем SecondCfg 
        m_flash.OutStreamRange(RangeIOBase::CfgSecond)->getWriter().WriteByte(0);
        InitStorage();
    }

    // ------------------------------------------------------------------------------------

    void BoardSetup::InitConfig()
    {
        ESS_ASSERT(!m_cfg);

        shared_ptr<Config> prim = Config::CreateCfg(m_flash, RangeIOBase::CfgPrim);
        shared_ptr<Config> second = Config::CreateCfg(m_flash, RangeIOBase::CfgSecond);       

        if (!prim && !second)
        {
            m_cfg = Config::CreateCfg();
            return;
        }

        // Only prim
        if (prim && !second)
        {
            m_cfg = prim;
            return;
        }

        // Only second
        if (!prim && second)
        {
            m_cfg = second;
            return;
        }

        // Both valid
        if ( second->UpdateCount.Value() > prim->UpdateCount.Value() )
        {
            m_cfg = second;
            return;
        }
        // else
        m_cfg = prim;
    }

    // ------------------------------------------------------------------------------------

    void BoardSetup::InitStorage()
    {
        ESS_ASSERT(m_cfg);                        
        m_appStorage.reset( new BfStorage(*m_cfg, m_flash) );
    }    

    // ------------------------------------------------------------------------------------

    BfBootSrv::dword BoardSetup::getSrvConnectionTimeout() const
    {
        return m_cfg->WaitLoginMsec.Value();
    }

    // ------------------------------------------------------------------------------------

    bool BoardSetup::ValidatePwd(const std::string& pwd) const
    {
        if (m_cfg->UserPwd.IsInited() && m_cfg->UserPwd.Value() == "") return true;
        return m_cfg->UserPwd.IsInited() && m_cfg->UserPwd.Value() == pwd;
    }

    // ------------------------------------------------------------------------------------   

    void BoardSetup::TuneTrace(iLogW::LogSettings& settings)
    {
        ESS_ASSERT(!m_haveUnsavedChanges && !m_haveSavedChanges); // запоминать адресс в конструкторе ?
        //if (IsServiceMode()) return;

        if (m_cfg->UdpLogHost.IsInited() && m_cfg->UdpLogPort.IsInited())
        {
            settings.out().Udp().DstHost = Utils::HostInf(m_cfg->UdpLogHost.Value(), m_cfg->UdpLogPort.Value());
            settings.out().Udp().TraceInd = true;            
            settings.TimestampInd(m_cfg->UseTimestamp.Value());
            settings.CountSyncroToStore(m_cfg->CountToStore.Value());
        }        
    }    

    // ------------------------------------------------------------------------------------

    word BoardSetup::getCmpPort() const
    {
        if (!m_cfg->CmpPort.IsInited()) return 0;
        return m_cfg->CmpPort.Value();
    }

    // ------------------------------------------------------------------------------------

    bool BoardSetup::ReadNetworkSettings( E1App::BoardAddresSettings& net ) const
    {       
        if (IsServiceMode()) return false;

        std::string mac;
        //if (m_factorySettings->MAC.IsInited()) mac = m_cfg->MAC.Value();
        mac = m_factorySettings->MAC.Value();

        if ( m_cfg->UseDHCP.Value() ||
            !m_cfg->IP.IsInited() ||
            !m_cfg->Gateway.IsInited() ||
            !m_cfg->NetMask.IsInited())
        {
            net.UseDHCP(mac);  
            return true;            
        }

        net.Set(m_cfg->IP.Value(), m_cfg->Gateway.Value(), m_cfg->NetMask.Value(), mac);
        return net.IsValid();
    }

    bool BoardSetup::IsDefault() const
    {
        return m_cfg->SourceRangeIndex == Config::NoneRange;
    }

//     std::string BoardSetup::getCfgRangeAsString()
//     {
//         if (m_cfg->SourceRangeIndex == Config::NoneRange) return "NoRange";
//         if (m_cfg->SourceRangeIndex == RangeIOBase::CfgPrim) return "PrimRange";
//         if (m_cfg->SourceRangeIndex == RangeIOBase::CfgSecond) return "SecondRange";
//         return "Unknown";
//     }

    
} // namespace BfBootSrv

