#pragma once 

namespace Utils
{
    
    class QtMsgHandler : boost::noncopyable
    {
    public:
        
        typedef boost::function<void (QtMsgType, QString typeAsStr, QString msg)> CallbackFn;
        
        QtMsgHandler(CallbackFn fn);
        ~QtMsgHandler();

        static void DefaultBehaviour(bool haltOnMsg);
    };
    
    
}  // namespace Utils