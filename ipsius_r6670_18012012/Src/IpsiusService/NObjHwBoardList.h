#ifndef __NOBJHWBOARDLIST__
#define __NOBJHWBOARDLIST__

#include "DRI/ISessionCmdContext.h"
#include "Utils/ManagedList.h"
#include "iCore/MsgTimer.h"

#include "NObjHwBoard.h"
#include "IBootUpdater.h"

#include "BfBootCore/BroadcastMsg.h"

namespace IpsiusService
{

    // список обнаруженных (через броадкаст) плат
    class NObjHwBoardList :
        public NamedObject, 
        public DRI::INonCreatable
    {
        Q_OBJECT;

        enum
        {
            CBoardProcessIntervalMs = 1 * 1000,
        };

        typedef std::vector<NObjHwBoard*> BoardList;

        IHwBoardOwner &m_owner;
        BoardList m_boards;  // childrens
        iCore::MsgTimer m_timer;

        void OnTimer(iCore::MsgTimer*)
        {
            for(int i = 0; i < m_boards.size(); ++i)
            {
                m_boards.at(i)->TimeProcess();
            }
        }

    public:

        NObjHwBoardList(IDomain *pDomain, const ObjectName &name, IHwBoardOwner &owner) :
          NamedObject(pDomain, name),
          m_owner(owner),
          m_timer(this, &NObjHwBoardList::OnTimer)
        {
            m_timer.Start(CBoardProcessIntervalMs, true);
        }

        void Scan(IBootUpdater& updater, Platform::dword newVersion)
        {
            for(int i = 0; i < m_boards.size(); ++i)    
            {
                if (!m_boards.at(i)->IsFiltered()) continue;                
                const BfBootCore::BroadcastMsg& msg = m_boards.at(i)->Msg();
                if (msg.BooterVersionNum >= newVersion) continue;
                updater.Update(msg/*.SrcAddress*/);                
            }
        }        

        Q_INVOKABLE void ListBoards(DRI::ICmdOutput *pCmd, bool briefly = true)
        {
            for(int i = 0; i < m_boards.size(); ++i)            
            {
                m_boards.at(i)->ListInfo(pCmd, briefly);
            }
        }

        void BroadcastMsg(const BfBootCore::BroadcastMsg &msg)
        {
            // try to put message into exists boards
            for(int i = 0; i < m_boards.size(); ++i)
            {
                if (m_boards.at(i)->ProcessMsg(msg)) return;
            }            
            
            m_boards.push_back( new NObjHwBoard(this, m_owner, msg) );
        }

        void Clean()
        {
            BoardList::iterator i = m_boards.begin();

            while(i != m_boards.end())
            {
                NObjHwBoard *p = *i;

                if (p->CleanRequired())
                {
                    delete p;
                    i = m_boards.erase(i);
                }
                else
                {
                    ++i;
                }
            }
        }

        void RemoveDirectBoards(const Utils::HostInf& addr)
        {
            BoardList::iterator i;
            
            for(i = m_boards.begin(); i != m_boards.end(); ++i)
            {
                NObjHwBoard *p = *i;
                if (p->IsDirect() && p->Msg().SrcAddress == addr) break;                
            }

            if (i == m_boards.end()) return;

            NObjHwBoard *p = *i;
            delete p;
            i = m_boards.erase(i);                                    
        }
                
    };    
    
}  // namespace IpsiusService

#endif
