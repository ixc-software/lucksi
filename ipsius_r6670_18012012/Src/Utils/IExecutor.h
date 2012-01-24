#ifndef IEXECUTOR_H
#define IEXECUTOR_H

namespace Utils
{
    class IExecutor : public IBasicInterface
    {
    public:
        virtual void Execute() = 0;
    };
} // namespace Utils

#endif

