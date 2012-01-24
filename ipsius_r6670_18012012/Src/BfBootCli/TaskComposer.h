#ifndef TASKCOMPOSER_H
#define TASKCOMPOSER_H

#include "TaskMngProfile.h"
#include "Utils/HostInf.h"
#include "BfBootCore/IHwFirmware.h"
#include "BfBootCore/ConfigLine.h"
#include "ITaskManager.h"
#include "TransportFactory.h"

#include "BfBootCore/ServerTuneParametrs.h"

namespace BfBootCli
{   
    using boost::shared_ptr;  
    using BfBootCore::UserParams;
    using BfBootCore::DefaultParam;
    
    
    struct TaskComposer
    {   
        //TaskComposer(const TaskMngProfile& prof, bool traceServer = false) -- ?

        static shared_ptr<ITaskManager> UpdateFirmware(
            const TaskMngProfile& prof,
            ITransportFactory& transport,         
            const std::string& pwd,
            /*const*/ BfBootCore::IHwFirmware &fw,
            bool traceServer = false
            );

        // ------------------------------------------------------------------------------------

        // установки дефолтной конфигурации (сервисный режим сервера)
        static shared_ptr<ITaskManager> SetParams(
            const TaskMngProfile& prof, 
            ITransportFactory& transport,
            const std::string& pwd, 
            const DefaultParam& param,
            bool autoReset,
            bool traceServer = false
            );

        // установка пользовательских настроек (пользовательский режим)
        static shared_ptr<ITaskManager> SetParams(
            const TaskMngProfile& prof, 
            ITransportFactory& transport,
            const std::string& pwd,
            const UserParams& param,
            bool traceServer = false
            );

        // ------------------------------------------------------------------------------------

        static shared_ptr<ITaskManager> UpdateBootImg(
            const TaskMngProfile& prof,
            const std::string& pwd,
            const BfBootCore::IHwFirmwareImage &bootImg, 
            ITransportFactory& transport,
            bool traceServer = false
            );

        // ------------------------------------------------------------------------------------

        static shared_ptr<ITaskManager> EraseDefault(
            const TaskMngProfile& prof,
            const std::string& pwd,                        
            ITransportFactory& transport,            
            bool traceServer = false
            );

        // ------------------------------------------------------------------------------------

        static shared_ptr<ITaskManager> StartNamedScript(
            const TaskMngProfile& prof,
            const std::string& pwd,            
            const std::string& scriptName,
            ITransportFactory& transport,
            bool traceServer = false
            );

        // ------------------------------------------------------------------------------------

        static shared_ptr<ITaskManager> ViewBoardParams(
            const TaskMngProfile& prof,
            const std::string& pwd,                        
            ITransportFactory& transport,
            shared_ptr<std::vector<BfBootCore::ConfigLine> > pResult, // if (!pResult) no result stored
            bool traceServer = false
            );

    };
} // namespace BfBootCli

#endif
