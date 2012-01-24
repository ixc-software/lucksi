#ifndef ITASKMANAGERREPORT_H
#define ITASKMANAGERREPORT_H

#include "Utils/IBasicInterface.h"

namespace BfBootCli
{
    struct TaskResult
    {
        bool Ok;
        const std::string& Info;

        TaskResult(bool ok, const std::string &info) : Ok(ok), Info(info)
        {
        }

        QString ToString() const
        {
            QString res = Ok ? "Ok" : "Failed";

            if (!Info.empty())
            {
                res += ", Info: ";
                res += Info.c_str();
            }

            return res;
        }
    };      

    class ITaskManagerReport : public Utils::IBasicInterface
    {
    public:        
        virtual void AllTaskFinished(const TaskResult& result) = 0;
        virtual void Info(/*QString taskName, ? */QString eventInfo, int val = -1) = 0;       
    };

} // namespace BfBootCli

#endif
