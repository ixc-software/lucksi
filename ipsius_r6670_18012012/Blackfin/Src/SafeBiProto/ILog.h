#ifndef ILOG_H
#define ILOG_H

namespace SBProto
{
    class ILog : public Utils::IBasicInterface
    {
    public:
        virtual void LogSBP(const std::string &s) = 0;
    };
} // namespace SBProto

#endif
