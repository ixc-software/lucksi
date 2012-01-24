#include "stdafx.h"
#include "ModArmIoIntf.h"

namespace 
{    
    const bool CTraceCmd = false;

    const std::string CDevName = std::string("/dev/") + ModArmIo::Detail::CName;

    using Platform::dword;
    using namespace ModArmIo;

    void TraceCmd(const Detail::UserData& cmd)
    {
        if (!CTraceCmd) return;

        std::cout << "ModArmIoIntf trace cmd:" << std::endl;

        for (int iter = 0; iter < cmd.m_itemCount; ++iter)
        {
            Detail::UserDataItem* pItem = cmd.m_pItem + iter;
            std::cout
                    << pItem->m_cmd
                    << " a " << std::hex << pItem->m_addr
                    << " val " << pItem->m_value
                    << " mask " << pItem->m_mask << std::endl;
        }
    }

    struct UserDataItem : public Detail::UserDataItem
    {        
    public:
        UserDataItem(dword addr, Detail::Cmd cmd, dword value, dword mask, dword usleep)
        {
            // init fields
            m_value = value;
            m_addr = addr;
            m_cmd = cmd;
            m_mask = mask;
            m_usleep = usleep;
            m_magic = Detail::CMagic;
        }      

        //UserDataItem()
    };

    // ------------------------------------------------------------------------------------

    // Detail::UserData wrapper
    class UserDataList
    {       
        Detail::UserData m_data;
        std::vector<UserDataItem> m_items;        

    public:             

        void Add(dword addr, Detail::Cmd cmd, dword value, dword mask, dword usleep)
        {
            m_items.push_back( UserDataItem(addr, cmd, value, mask, usleep) );
        }        

        int Size() const
        {
            return m_items.size();
        }

        const UserDataItem& operator[](int i) const
        {
            return m_items.at(i);
        }
        
        void Run(DevControl& dev)
        {
            m_data.m_magic = Detail::CMagic;
            m_data.m_itemCount = m_items.size();
            m_data.m_pItem = &m_items.at(0);

            TraceCmd(m_data);            

            dev.RunCmd(m_data);
        }

    };


} // namespace 

// ------------------------------------------------------------------------------------

// ModArmIoIntf internal types
namespace ModArmIo
{

    ModArmIoIntf::Cmd::Cmd(Detail::Cmd cmd, dword addr, dword val, dword mask, dword uSleep)
    {
        Command = cmd;
        Addr = addr;
        Val = val;
        Mask = mask;   
        USleep = uSleep;
    }    
   
    // ------------------------------------------------------------------------------------

    int ModArmIoIntf::CmdList::AddRead( dword addr, dword uSleep ) /* return index in Result */
    {
        m_cmds.push_back( Cmd(Detail::cmdRead, addr, 0, 0, uSleep) );
        return m_cmds.size() - 1;
    }

    // ------------------------------------------------------------------------------------

    void ModArmIoIntf::CmdList::AddWrite( dword addr, dword value, dword mask, dword uSleep )
    {
        m_cmds.push_back( Cmd(Detail::cmdWrite, addr, value, mask, uSleep) );
    }    

    // ------------------------------------------------------------------------------------

    QString ModArmIoIntf::CmdList::ToString() const
    {
        QString res;
        for (int i = 0; i < Size(); ++i)
        {
            const Cmd& cmd = m_cmds.at(i);
            res += QString("%1 addr: %2 val: %3 mask %4; ")
                .arg(cmd.Command == Detail::cmdRead ? "Read" : "Write")
                .arg(cmd.Addr)
                .arg(cmd.Val)
                .arg(cmd.Mask);
        }
        return res;
    }

    // ------------------------------------------------------------------------------------

    ModArmIo::dword ModArmIoIntf::ResultList::operator[]( int i )
    {        
        ESS_ASSERT(Exist(i));
        return m_list.at(i);
    }

    // ------------------------------------------------------------------------------------

    bool ModArmIoIntf::ResultList::Exist( int i ) const
    {
        ESS_ASSERT(i < Size() && i >= 0);
        ESS_ASSERT( i < m_cmds->Size() );
        return (*m_cmds)[i].Command == Detail::cmdRead;
    }

    // ------------------------------------------------------------------------------------

    QString ModArmIoIntf::ResultList::ToString( bool brief /*= true*/ )
    {
        QString res;
        ESS_ASSERT( m_list.size() == m_cmds->Size() );
        for (int i = 0; i < m_list.size(); ++i)
        {
            if (!brief) res += QString("Cmd: %1 ").arg(m_cmds->ToString());
            if (Exist(i)) res += QString("Res: %1 ").arg(m_list.at(i));
        }
        return res;
    }

} // namespace ModArmIo

// ------------------------------------------------------------------------------------

namespace ModArmIo
{        
    ModArmIoIntf::ModArmIoIntf()
        : m_dev(CDevName, true)
    {        
    }

    ModArmIoIntf::ResultList ModArmIoIntf::RunCommands( const CmdList& cmds )
    {                   
        UserDataList raw;
        
        for (int i = 0; i < cmds.Size(); ++i)
        {
            raw.Add(cmds[i].Addr, cmds[i].Command, cmds[i].Val, cmds[i].Mask, cmds[i].USleep);            
        }        

        raw.Run(m_dev);

        ResultList result(cmds);

        for (int i = 0; i < raw.Size(); ++i)
        {
            result.Push(raw[i].m_value);
        }

        return result;
    }

    // ------------------------------------------------------------------------------------

    ModArmIo::dword ModArmIoIntf::Read( dword addr ) 
    {
        return RunSingleCmd(Detail::cmdRead, addr, 0, 0);
    }

    // ------------------------------------------------------------------------------------

    void ModArmIoIntf::Write( dword addr, dword val, dword mask )
    {
        RunSingleCmd(Detail::cmdWrite, addr, val, mask);
    }

    // ------------------------------------------------------------------------------------

    dword ModArmIoIntf::RunSingleCmd( Detail::Cmd cmd, dword addr, dword val, dword mask )
    {
        UserDataList raw;
        raw.Add(addr, cmd, val, mask, 0);
        raw.Run(m_dev);
        return raw[0].m_value;
    }

    // ------------------------------------------------------------------------------------

    void UseExample()
    {
        ModArmIoIntf drv;
        ModArmIoIntf::CmdList commands;

        int index0 = commands.AddRead(0xffffffff); //  return ResultList index
        commands.AddWrite(0xffffffff, 100, 0);
        int index1 = commands.AddRead(0xffffffff);

        ModArmIoIntf::ResultList result = drv.RunCommands(commands);

        //trace        
        result.ToString(false); 

        // process result 1
        result[index0]; 
        result[index1];

        // process result 2
        for (int i = 0; i < result.Size(); ++i)
        {
            if (!result.Exist(i)) continue;
            result[i];
        }
    }

} // namespace ModArmIo
