#ifndef BROADCASTMSG_H
#define BROADCASTMSG_H

#include "stdafx.h"

#include "Platform/PlatformTypes.h"
#include "Utils/HostInf.h"
#include "SafeBiProto/SbpSerialize.h"
#include "ImgDescrList.h"


namespace BfBootCore
{
    using Platform::dword;
    using Platform::byte;
    using Platform::word;       
    

    struct BroadcastMsg
    {           
        enum { CItemCount = 11, };

        // Serialized fields.                
        dword BroadcastVersion;
        std::string BooterVersionInfo;     
        dword BooterVersionNum;
               
        dword HwNumber;
        dword HwType;
        
        enum {CDirectMark = -1};
        // if (port <= 0) порт не указан, if port == CDirectMark отправленно по прямому запросу
        int BooterPort; 
        int CmpPort;

        std::string OwnerAddress;
        std::string Mac;    
        enum {NoRelNum = 0};
        
        dword SoftRelNumber; // number of transaction  == NoRelNum if not exist
        ImgDescrList SoftReleaseInfo;  // todo string name list        
        
        // не сериализируемые поля
        bool ExtraFieldExist;       
        Utils::HostInf CbpAddress;  
        Utils::HostInf SrcAddress;  

    public:

        BroadcastMsg();                     

        void ToBin(std::vector<Platform::byte>& out) const;  // Удобно ли vector<byte> -- ?
        bool Assign(const Utils::HostInf& src, const Platform::byte* data, int size);

        bool IsBooter() const { return BooterPort > 0; }
        bool IsExistSoftRelNum() const { return SoftRelNumber != NoRelNum; }

		Utils::HostInf CmpHost() const 
		{
			return Utils::HostInf(SrcAddress.Address(), CmpPort);
		}
        // for log
        std::string Info()const; 

        bool IsDirect() const // vs IsBroadcast
        {
            if (IsBooter()) return CmpPort == CDirectMark;
            else            return BooterPort == CDirectMark;
        }

        void MarkAsDirect(bool direct)
        {
            int val = direct ? CDirectMark : 0;
            if (IsBooter()) CmpPort = val;
            else            BooterPort = val;
        }

    private:
        bool Consistency() const;       

    };

    
    
} // namespace BfBootCore

#endif
