#ifndef CMDRECEIVEENGINE_H
#define CMDRECEIVEENGINE_H

#include "stdafx.h"
#include "Utils/IVirtualDestroy.h"
#include "Utils/ManagedList.h"


namespace SBProto
{
    class SbpRecvPack;

    // engine level class
    template<class TCallback>
    class ICmdInterface : public Utils::IVirtualDestroy
    {
    public:
        virtual std::string Name() const = 0;
        virtual void ProcessPacket(const SbpRecvPack &pack, TCallback &callback) const = 0;
    };

    //-------------------------------------------------------------------------------------    

    // engine level class
    template<class TCallback>
    class RecvProcess : boost::noncopyable
    {
        TCallback* m_pCallback;
        Utils::ManagedList< ICmdInterface<TCallback> > m_cmdList;    
        
    public:

        template<class TCmd>
        void Add(const TCmd &cmd)
        {
            std::string name = cmd.Name();

            m_cmdList.Add( new TCmd() );
        }

        RecvProcess(TCallback &callback) : m_pCallback(&callback)
        {
        }

        void ResetCallback(TCallback &callback)
        {
            m_pCallback = &callback;
        }

        bool Process(const SbpRecvPack &pack)
        {
            std::string cmdName(pack[0].AsString());

            for(size_t i = 0; i < m_cmdList.Size(); ++i)
            {
                if (m_cmdList[i]->Name() == cmdName)
                {
                    m_cmdList[i]->ProcessPacket(pack, *m_pCallback);
                    return true;
                }
            }
            return false;
        }
    };

} // namespace SBProto

#endif
