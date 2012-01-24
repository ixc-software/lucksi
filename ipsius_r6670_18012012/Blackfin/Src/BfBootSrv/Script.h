#ifndef SCRIPT_H
#define SCRIPT_H

#include "Utils/SafeRef.h"
#include "BfBootCore/ScriptCmdList.h"

namespace BfBootSrv
{    
    class BfStorage;
    class Config;   
    class IReload;

    using BfBootCore::ScriptError;

    class Script 
        : virtual public Utils::SafeRefServer,
        public boost::noncopyable
    {
        boost::scoped_ptr<BfBootCore::ScriptCmdList> m_list;
        std::vector<BfBootCore::ImgDescriptor> m_imgs;

        /*const */BfStorage& m_storage;       

    public:        
        
        ESS_TYPEDEF_FULL(MisfitToStorage, ScriptError);
        ESS_TYPEDEF_FULL(ScriptNotExist, ScriptError);

        Script(const std::string& name, const Config& cfg, /*const */BfStorage& storage);        
        void Run(IReload& loader);
    };
} // namespace BfBootSrv

#endif
