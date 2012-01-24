#ifndef STARTSCRIPT_H
#define STARTSCRIPT_H



//#include "VirtualInvoke.h"
//#include "CfgField.h"
#include "ImgDescriptor.h"
#include "Utils/ErrorsSubsystem.h"

namespace BfBootCore
{   

    class ScriptComposer
    {
        std::ostringstream m_script;

    public:
        //void AddCmdRun(ImgId img);
        void AddCmdLoad(const std::string& intfName, ImgId img);
        std::string toString() const;
    };

    ESS_TYPEDEF(InvalidScript);

    void RunScript(const std::string& raw); // can throw InvalidScript

    
    /*
    Набросок гибкой системмы формирования строкового скрипта и его выполнения.
    Для добавления в системму новой команды надо добавить Cmd-тип и зарегистрировать его в ScriptRunner.

    На пользовательской стороне c помощью ScriptComposer формируется строка скрипта.
    Нет проверок и отработок ошибок.
    */

    //class ICmd
    //{
    //    virtual void Run() = 0;
    //    virtual std::string getName() const = 0;
    //    virtual void FromString(const std::string& raw) = 0;
    //};

    //class CmdLoad : public ICmd
    //{
    //private:        
    //    FieldList m_fields;

    //    
    //    std::string getName() const {return Name();}
    //    
    //public:        

    //    std::string AsString()
    //    {
    //        ConfigLineList list;
    //        m_fields.WriteTo(list);
    //        return Name() + "(" + list.AsString(",") + ")" + ";";
    //    }

    //    // общая часть
    //    //------------------
    //    // специфичная часть

    //    static std::string Name() {return "Load";}
    //    
    //    // параметры
    //    CfgField<int> BoardIndex;
    //    CfgField<int> ImgIndex;                    

    //    CmdLoad(int boardIndex, int imgIndex)
    //        : BoardIndex("BoardIndex", boardIndex, m_fields),
    //        ImgIndex("ImgIndex", imgIndex, m_fields)
    //    {}    

    //    CmdLoad()
    //        : BoardIndex("BoardIndex", m_fields),
    //        ImgIndex("ImgIndex", m_fields)
    //    {            
    //    }

    //private:
    //    void FromString(const std::string& raw)
    //    {
    //        ConfigLineList params(raw);
    //        m_fields.InitFieldsFrom(params);
    //    }

    //    void Run() {/*Load(BoardIndex,  ImgIndex)*/}
    //};

    //// user side
    //class ScriptComposer
    //{
    //    std::string m_scriptString;

    //public:
    //    template<class T>
    //    ScriptComposer& AddCmd(const T& cmd)
    //    {
    //        m_scriptString += cmd.AsString();
    //        return *this;
    //    }

    //    const std::string& getScript()
    //    {
    //        return m_scriptString;
    //    }
    //};

    //// booter side
    //class ScriptRunner
    //{
    //    std::vector<scoped_ptr<ICmd> > m_cmd;
    //public:

    //    ScriptRunner()
    //    {
    //        // RegAllCmd:
    //        m_cmd.push_back(new CmdLoad);
    //        // m_cmd.push_back(new CmdRun);
    //        // ...
    //    }

    //    void RunScript(const std::string& raw)
    //    {
    //        //делим строку на команды
    //        // извлекаем из команды имя
    //        // ищем в m_cmd[i].getName() == имя
    //        // m_cmd[i]->Run()
    //    }
    //};
} // namespace BfBootCore

#endif
