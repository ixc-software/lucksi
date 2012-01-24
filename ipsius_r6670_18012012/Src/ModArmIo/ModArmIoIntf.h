#ifndef MODARMIOINTF_H
#define MODARMIOINTF_H

#include "Platform/PlatformTypes.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/SafeRef.h"
#include "DevControl.h"

#include "ModArmIoDetail.h" // c-header move under namespace

// ModArmIoIntf provide interface of ModArmIo.ko driver


namespace ModArmIo
{
    using Platform::dword;

    class ModArmIoIntf
    {    
        DevControl m_dev;

        dword RunSingleCmd(Detail::Cmd cmd, dword addr, dword val, dword mask);
        

        struct Cmd
        {
            Detail::Cmd Command;
            dword Addr;
            dword Val;
            dword Mask;
            dword USleep;

            Cmd(Detail::Cmd cmd, dword addr, dword val, dword mask, dword uSleep);
        };             


    // public types definition
    public:        
        typedef DevControl::Err Err;

        //---------------

        class CmdList : public virtual Utils::SafeRefServer, boost::noncopyable
        {
            std::vector<Cmd> m_cmds;            
        public:            

            int AddRead(dword addr, dword uSleep = 0); // return index in Result;
            void AddWrite(dword addr, dword value, dword mask, dword uSleep = 0);
            int Size() const { return m_cmds.size(); }
            const Cmd& operator[]( int i ) const
            {
                ESS_ASSERT(i < Size() && i >= 0);                
                return m_cmds.at(i);
            }
            QString ToString() const;
        };

        //---------------

        class ResultList // copyable
        {
            Utils::SafeRef<const CmdList> m_cmds;
            std::vector<dword> m_list;

        public:            
            ResultList(const CmdList& cmds): m_cmds(&cmds) {}
            void Push(dword val) { m_list.push_back(val); }
            int Size() const { return m_list.size(); }
            dword operator[] (int i);
            bool Exist(int i) const;
            
            QString ToString(bool brief = true);
        };

        //---------------

    public:
        ModArmIoIntf(); // can throw Err        

        ResultList RunCommands(const CmdList& commands); // can throw Err

        // run single command
        dword Read(dword addr); // can throw Err
        void Write(dword addr, dword val, dword mask = 0); // can throw Err
    };


    void UseExample();   

} // namespace ModArmIo




#endif
