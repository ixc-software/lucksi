#ifndef IBOOTSERVERTOCLIENT_H
#define IBOOTSERVERTOCLIENT_H

#include "Utils/IBasicInterface.h"
#include "Utils/ErrorsSubsystem.h"
#include "Platform/PlatformTypes.h"


#include "UserResponseCode.h"
#include "StateInfoCode.h"

namespace BfBootCore
{    
    using Platform::dword;
    using Platform::int32;

    class ConfigLine;
    class ScriptList;
    class ImgDescrList;
    struct ImgDescriptor;  
    

    // Интерфейс удаленного клиента загрузчика(ответы на ILogin + IBootCtrl)
    class IBootServerToClient : public Utils::IBasicInterface
    {
    public:       
        virtual void RespLogin(const UserResponseInfo& code, const std::string&additionalInfo, const std::string& booterVersion, bool userMode) = 0;                
        virtual void RespCmd(const UserResponseInfo& code) = 0;
        virtual void RespGetParam(const std::vector<ConfigLine>& userParams) = 0;
        virtual void RespGetAppList(const ImgDescrList& appList) = 0;
        virtual void RespNewImgAdded(const ImgDescriptor&) = 0;
        virtual void RespGetScriptList(const ScriptList&) = 0;
        virtual void StateInfo(const SrvStateInfo& info, int32 progress = -1) = 0;        
    };

} // namespace BfBootCore

#endif
