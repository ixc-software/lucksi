#include "stdafx.h"

#include "TaskComposer.h"
#include "RealTasks.h"
#include "TaskManager.h"
#include "Domain/IDomain.h"
#include "Utils/HostInf.h"
#include "BfBootCli/CFactorySettingsSupport.h"

namespace 
{
    const Platform::dword CConnectionTeoutMsec = 5000; // -- ?
    using namespace BfBootCli;

    shared_ptr<TaskManager> CreateTaskMng(const TaskMngProfile& prof, ITransportFactory& transport, bool traceServer)
    {
        shared_ptr<TaskManager> result;
        result.reset(new TaskManager(prof, transport.Create(prof.Domain), CConnectionTeoutMsec) );
        if (traceServer) new TaskSetTrace(*result, traceServer);
        return result;
    }

} // namespace 

namespace BfBootCli
{  

    shared_ptr<ITaskManager> TaskComposer::UpdateFirmware( const TaskMngProfile& prof, ITransportFactory& transport, const std::string& pwd, /*const*/ BfBootCore::IHwFirmware &fw, bool traceServer /*= false */ )
    {
        shared_ptr<TaskManager> task = CreateTaskMng(prof, transport, traceServer);

        new TaskLogin(*task, pwd);            
        new TaskDeleteAllImg(*task);
        for (int i = 0; i < fw.ImageCount(); ++i)
        {            
            new TaskWrite(*task, fw.Image(i), TaskWrite::AppImg);
        }

        // todo прошивка должна отдавать ссылку на свой ScriptList
        BfBootCore::ScriptList list;
        for (int i = 0; i < fw.ScriptCount(); ++i)
        {
            list.Add( BfBootCore::NamedScript(fw.Script(i).Name(), fw.Script(i).Value()) );
        }                        

        new TaskSetUserSettings(*task, BfBootCore::StringToTypeConverter::toString(list), "StartScriptList");       

        new TaskCloseTransaction(*task, fw.Release());
        new TaskLogout(*task, true); 
        task->RunTasks();

        return task;
    }

    // ------------------------------------------------------------------------------------

    // установки дефолтной конфигурации (сервисный режим сервера)
    shared_ptr<ITaskManager> TaskComposer::SetParams( const TaskMngProfile& prof, ITransportFactory& transport, const std::string& pwd, const DefaultParam& param, bool autoReset, bool traceServer /*= false */ )
    {
        if (autoReset) ESS_ASSERT(BfBootCli::CFactorySettingsSupport);
        
        shared_ptr<TaskManager> task = CreateTaskMng(prof, transport, traceServer);

        new TaskLogin(*task, pwd, (autoReset) ? TaskLogin::mod_ServiceAuto : TaskLogin::mod_Service);
        param.WriteTaskToMng(*task);          
        new TaskLogout(*task, true);
        task->RunTasks();

        return task;
    }

    // ------------------------------------------------------------------------------------

    // установка пользовательских настроек (пользовательский режим)
    shared_ptr<ITaskManager> TaskComposer::SetParams( const TaskMngProfile& prof, ITransportFactory& transport, const std::string& pwd, const UserParams& param, bool traceServer /*= false */ )
    {
        shared_ptr<TaskManager> task = CreateTaskMng(prof, transport, traceServer);

        new TaskLogin(*task, pwd);
        param.WriteTaskToMng(*task);          
        new TaskLogout(*task, true);
        task->RunTasks();        

        return task;
    }

    // ------------------------------------------------------------------------------------

    shared_ptr<ITaskManager> TaskComposer::UpdateBootImg( const TaskMngProfile& prof, const std::string& pwd, const BfBootCore::IHwFirmwareImage &bootImg, ITransportFactory& transport, bool traceServer /*= false */ )
    {
        shared_ptr<TaskManager> task = CreateTaskMng(prof, transport, traceServer);

        new TaskLogin(*task, pwd);                   
        new TaskWrite(*task, bootImg, TaskWrite::BootImg); 
        new TaskLogout(*task, false);
        task->RunTasks();

        return task;
    }

    // ------------------------------------------------------------------------------------

    shared_ptr<ITaskManager> TaskComposer::EraseDefault( const TaskMngProfile& prof, const std::string& pwd, ITransportFactory& transport, bool traceServer /*= false */ )
    {
        ESS_ASSERT(BfBootCli::CFactorySettingsSupport);

        shared_ptr<TaskManager> task = CreateTaskMng(prof, transport, traceServer);

        new TaskLogin(*task, pwd, TaskLogin::mod_ServiceAuto);                
        new TaskLogout(*task, false);
        task->RunTasks();

        return task;
    }

    // ------------------------------------------------------------------------------------

    shared_ptr<ITaskManager> TaskComposer::StartNamedScript( 
        const TaskMngProfile& prof, 
        const std::string& pwd,
        const std::string& scriptName,
        ITransportFactory& transport,
        bool traceServer /*= false */ 
        )
    {
        ESS_ASSERT(scriptName != BfBootCore::NamedScript::NameOfBootScript());
        shared_ptr<TaskManager> task = CreateTaskMng(prof, transport, traceServer);

        new TaskLogin(*task, pwd);
        new TaskRunAddScript(*task, scriptName);        
        task->RunTasks();

        return task;
    }

    // ------------------------------------------------------------------------------------

    shared_ptr<ITaskManager> TaskComposer::ViewBoardParams(
        const TaskMngProfile& prof,
        const std::string& pwd,
        ITransportFactory& transport,
        shared_ptr<std::vector<BfBootCore::ConfigLine> > pResult,
        bool traceServer /*= false */ 
        )
    {
        shared_ptr<TaskManager> task = CreateTaskMng(prof, transport, traceServer);

        new TaskLogin(*task, pwd);                   
        new TaskViewParams(*task, pResult); 
        new TaskLogout(*task, false);
        task->RunTasks();

        return task;
    }
} // namespace BfBootCli
