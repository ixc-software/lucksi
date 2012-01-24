#include "stdafx.h"
#include "StartScript.h"
#include "Utils/ErrorsSubsystem.h"

namespace
{
    using namespace BfBootCore;
    //const std::string CCmdRun = "Run";
    //const std::string CCmdLoad = "Load";
    
    // syntax consts:
    // param sep
    // open/close brace
    // comand terminator

    // ------------------------------------------------------------------------------------
    struct ParamsRange
    {
        size_t BeginPos;
        size_t Size;

        ParamsRange(const std::string& name, const std::string& cmdSeq, int offs)
        {
            ESS_ASSERT(cmdSeq.find(name, offs) == offs);
            BeginPos = cmdSeq.find('(', offs);
            size_t endParams = cmdSeq.find(')', BeginPos);
            ESS_ASSERT(endParams != std::string::npos);
            ESS_ASSERT(BeginPos < endParams);
            Size = BeginPos - endParams;
        }
    };    

    //struct CmdRun
    //{
    //    static const std::string& Name()
    //    {
    //        static const std::string name("Run");
    //        return name;
    //    }

    //    static void WriteCmd(std::ostringstream& out, ImgId id)
    //    {
    //        out << Name() << "(" << id << ")";
    //    }

    //    static int Run(const std::string& cmdSeq, int offs)
    //    {
    //        ParamsRange range(Name(), cmdSeq, offs);            

    //        ImgId id;
    //        std::istringstream is(cmdSeq.substr(range.BeginPos, range.Size));
    //        is >> id;

    //        ESS_ASSERT(!is.fail());
    //        // run cmd ...
    //        return range.Size + Name().size() + 2; // "()"
    //    }
    //};

    // ------------------------------------------------------------------------------------

    struct CmdLoad 
    {
        static const std::string& Name()
        {
            static const std::string name("Run");
            return name;
        }

        static void WriteCmd(std::ostringstream& out, const std::string& intfName, ImgId id )
        {
            out << Name() << "(" << intfName << "," << id << ")";
        }

        static int Run(const std::string& cmdSeq, int offs)
        {
            ParamsRange range(Name(), cmdSeq, offs);            
            int comma = cmdSeq.find(',', range.BeginPos);
            //ESS_ASSERT(comma != std::string::npos);                        
            if (comma == std::string::npos) ESS_THROW(InvalidScript);

            std::istringstream is(cmdSeq.substr(range.BeginPos, range.Size));
            std::string intfName;
            intfName.resize(comma - offs);
            is.get(&intfName[0], comma - offs);
            ImgId id;
            is >> id;
            //ESS_ASSERT(!is.fail());
            if (is.fail()) ESS_THROW(InvalidScript);

            ESS_HALT("todo");// run cmd ...            
            return range.Size + Name().size() + 3; // "(,)"
        }
    };

    // ------------------------------------------------------------------------------------

    void AssertNotExist(const std::string& intfName, char symbol)
    {
        ESS_ASSERT(intfName.find(symbol) == std::string::npos);
    }
}

namespace BfBootCore
{    
    /*void ScriptComposer::AddCmdRun( ImgId img )
    {
        CmdRun::WriteCmd(m_script, img);
    }*/

    void ScriptComposer::AddCmdLoad( const std::string& intfName, ImgId img )
    {
        AssertNotExist(intfName, ';');
        AssertNotExist(intfName, ',');
        AssertNotExist(intfName, '(');
        AssertNotExist(intfName, ')');        

        CmdLoad::WriteCmd(m_script, intfName, img);
    }

    // ------------------------------------------------------------------------------------

    std::string ScriptComposer::toString() const
    {
        return m_script.str();
    }

    // ------------------------------------------------------------------------------------    

    int CmdSwitch(const std::string& raw, int offs)
    {   
        //if (raw.find(CmdRun::Name(), offs) != std::string::npos) return CmdRun::Run(raw, offs);
        if (raw.find(CmdLoad::Name(), offs) != std::string::npos) return CmdLoad::Run(raw, offs);

        //ESS_HALT("Unknown command");    
        ESS_THROW(InvalidScript);
        return 0;
    }

    // ------------------------------------------------------------------------------------

    void RunScript(const std::string& raw)
    {
        int offs = 0;
        while (offs < raw.size())
        {
            offs += CmdSwitch(raw, offs);
        }
    }

} // namespace BfBootCore

