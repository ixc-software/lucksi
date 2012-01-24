#include "stdafx.h"
#include "Script.h"
#include "BfStorage.h"
#include "Utils/StringUtils.h"
#include "ApplicationLoader.h"
#include "Config.h"



namespace BfBootSrv
{   
    using BfBootCore::ImgDescriptor;

    Script::Script( const std::string& name, const Config& cfg, /*const */BfStorage& storage )
        : m_storage(storage)
    {
        const BfBootCore::ScriptList& scripts = cfg.StartScriptList.Value();

        int index = scripts.FindByName(name);
        if (index == BfBootCore::ScriptList::NotFound) 
        {
            ESS_THROW(ScriptNotExist);
        }

        m_list.reset(new BfBootCore::ScriptCmdList(scripts[index].Value()));

        for(int i = 0; i < m_list->Size(); ++i)
        {
            ImgDescriptor img;
            if (!storage.Find((*m_list)[i].getImgId(), img)) ESS_THROW(MisfitToStorage);
            m_imgs.push_back(img);
        }
        // todo MAIN only in last
    }

    void Script::Run(IReload& loader)
    {
        for (int i = 0 ; i < m_list->Size(); ++i)
        {
            if ( (*m_list)[i].getIntf() == BfBootCore::ScriptCmdList::MainIntf())
            {
                loader.LoadToMain(m_storage, m_imgs.at(i));
                continue;
            }
            if ( (*m_list)[i].getIntf() == BfBootCore::ScriptCmdList::ExtIntf())
            {
                loader.LoadToSpi(m_storage, m_imgs.at(i));
                continue;
            }
            ESS_HALT("InvalidInterfaceName");
        }
    }
} // namespace BfBootSrv

