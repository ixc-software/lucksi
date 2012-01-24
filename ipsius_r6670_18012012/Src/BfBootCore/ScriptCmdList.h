#ifndef SCRIPTCMDLIST_H
#define SCRIPTCMDLIST_H

#include "Utils/IBasicInterface.h"
#include "Utils/ErrorsSubsystem.h" 
#include "Utils/ManagedList.h"
#include "ImgDescriptor.h"


namespace BfBootCore
{    
    // интерфейс дает возможность спрятать определения команд в cpp
    class ICmd : public Utils::IBasicInterface 
    {
    public:
        virtual std::string getIntf() const = 0;
        virtual ImgId getImgId() const = 0;                
    };

    // ------------------------------------------------------------------------------------

    ESS_TYPEDEF(ScriptError); // base class of all script error (Core&Srv)

    // ------------------------------------------------------------------------------------

    // Format: "Load( [SPI/MAIN], [app number]);"
    class ScriptCmdList : boost::noncopyable
    {        
        class CmdLoad;        
        Utils::ManagedList<ICmd> m_list;

    public:                             

        ScriptCmdList(const std::string& raw); // can throw ScriptError
        const ICmd& operator[](int index);
        int Size() const;        

        // If script invalid & pErr !=0  writing to error info to pErr
        static bool Validate(const std::string& scriptString, int imgCount, std::string* pErr = 0);

        // Имя интерфейса загрузки:
        static const std::string& MainIntf();
        static const std::string& ExtIntf();
    };
} // namespace BfBootCore

#endif
